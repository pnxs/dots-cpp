// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/Filter.h>

#include <algorithm>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <dots/type/Descriptor.h>
#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/type/StructDescriptor.h>
#include <dots/type/Typeless.h>

namespace dots::filter
{
    namespace
    {
        bool isOrderedType(type::Type t)
        {
            switch (t)
            {
                case type::Type::int8:    case type::Type::int16:
                case type::Type::int32:   case type::Type::int64:
                case type::Type::uint8:   case type::Type::uint16:
                case type::Type::uint32:  case type::Type::uint64:
                case type::Type::float32: case type::Type::float64:
                case type::Type::string:
                case type::Type::timepoint: case type::Type::steady_timepoint:
                case type::Type::duration:
                    return true;
                default:
                    return false;
            }
        }

        bool isFundamentalEquatableType(type::Type t)
        {
            return t != type::Type::Vector && t != type::Type::Struct;
        }

        // ---- wire-slot traits ---------------------------------------------
        // The single place that maps a property's type::Type to the
        // DotsPredicateValue slot holding its scalar (and, where applicable,
        // list) wire representation, plus the narrowing cast target. Slot
        // matching, list sizing, and rhs narrowing all dispatch through it.

        struct NoSlot {};

        template <typename TNarrowed, typename TScalarPtr, typename TListPtr>
        struct Slot
        {
            using narrowed_t = TNarrowed;
            static constexpr bool HasList = !std::is_same_v<TListPtr, std::nullptr_t>;

            TScalarPtr scalar;
            TListPtr list;
        };

        template <typename TNarrowed, typename TScalarPtr, typename TListPtr = std::nullptr_t>
        constexpr auto makeSlot(TScalarPtr scalar, TListPtr list = nullptr)
        {
            return Slot<TNarrowed, TScalarPtr, TListPtr>{ scalar, list };
        }

        template <typename F>
        decltype(auto) withSlot(type::Type t, F&& f)
        {
            using V = types::DotsPredicateValue;

            switch (t)
            {
                case type::Type::boolean:   return f(makeSlot<types::bool_t>(&V::boolVal));
                case type::Type::int8:      return f(makeSlot<types::int8_t>(&V::intVal, &V::intList));
                case type::Type::int16:     return f(makeSlot<types::int16_t>(&V::intVal, &V::intList));
                case type::Type::int32:     return f(makeSlot<types::int32_t>(&V::intVal, &V::intList));
                case type::Type::int64:     return f(makeSlot<types::int64_t>(&V::intVal, &V::intList));
                case type::Type::uint8:     return f(makeSlot<types::uint8_t>(&V::uintVal, &V::uintList));
                case type::Type::uint16:    return f(makeSlot<types::uint16_t>(&V::uintVal, &V::uintList));
                case type::Type::uint32:    return f(makeSlot<types::uint32_t>(&V::uintVal, &V::uintList));
                case type::Type::uint64:    return f(makeSlot<types::uint64_t>(&V::uintVal, &V::uintList));
                case type::Type::float32:   return f(makeSlot<types::float32_t>(&V::floatVal, &V::floatList));
                case type::Type::float64:   return f(makeSlot<types::float64_t>(&V::floatVal, &V::floatList));
                case type::Type::string:    return f(makeSlot<types::string_t>(&V::stringVal, &V::stringList));
                case type::Type::timepoint:
                case type::Type::steady_timepoint:
                                            return f(makeSlot<types::timepoint_t>(&V::timepointVal, &V::timepointList));
                case type::Type::duration:  return f(makeSlot<types::duration_t>(&V::durationVal));
                case type::Type::uuid:      return f(makeSlot<types::uuid_t>(&V::uuidVal, &V::uuidList));
                default:                    return f(NoSlot{});
            }
        }

        bool valueSlotMatches(const types::DotsPredicateValue& v, type::Type t, bool wantList)
        {
            return withSlot(t, [&](auto slot) -> bool
            {
                if constexpr (std::is_same_v<decltype(slot), NoSlot>)
                {
                    return false;
                }
                else if constexpr (!decltype(slot)::HasList)
                {
                    return !wantList && (v.*slot.scalar).isValid();
                }
                else
                {
                    return wantList ? (v.*slot.list).isValid() : (v.*slot.scalar).isValid();
                }
            });
        }

        // ---- rhs buffer helpers -------------------------------------------------

        std::byte* allocRhsBuffer(std::size_t bytes, std::size_t alignment)
        {
            return static_cast<std::byte*>(
                ::operator new(bytes, std::align_val_t{ alignment }));
        }

        void freeRhsBuffer(std::byte* buffer, std::size_t alignment)
        {
            ::operator delete(buffer, std::align_val_t{ alignment });
        }

        template <typename T>
        void constructAt(const type::Descriptor<type::Typeless>& vd, std::byte* dst, const T& value)
        {
            vd.constructInPlace(*reinterpret_cast<type::Typeless*>(dst),
                                type::Typeless::From(value));
        }

        // Narrow a single scalar wire value into a typed slot at dst.
        void narrowScalar(const type::Descriptor<type::Typeless>& vd, std::byte* dst,
                          const types::DotsPredicateValue& v)
        {
            withSlot(vd.type(), [&](auto slot)
            {
                // NoSlot is unreachable: rejected by validation beforehand
                if constexpr (!std::is_same_v<decltype(slot), NoSlot>)
                {
                    using narrowed_t = typename decltype(slot)::narrowed_t;
                    constructAt(vd, dst, static_cast<narrowed_t>(*(v.*slot.scalar)));
                }
            });
        }

        // Number of elements in the matching list slot (0 when the type has
        // no list form).
        std::size_t listSlotSize(const types::DotsPredicateValue& v, type::Type t)
        {
            return withSlot(t, [&](auto slot) -> std::size_t
            {
                if constexpr (std::is_same_v<decltype(slot), NoSlot>)
                {
                    return 0;
                }
                else if constexpr (!decltype(slot)::HasList)
                {
                    return 0;
                }
                else
                {
                    return (v.*slot.list)->size();
                }
            });
        }

        // Narrow a list of wire values into N back-to-back typed slots starting at dst.
        // Returns the number of elements written.
        std::uint32_t narrowList(const type::Descriptor<type::Typeless>& vd, std::byte* dst,
                                 const types::DotsPredicateValue& v)
        {
            return withSlot(vd.type(), [&](auto slot) -> std::uint32_t
            {
                if constexpr (std::is_same_v<decltype(slot), NoSlot>)
                {
                    return 0; // unreachable: rejected by validation beforehand
                }
                else if constexpr (!decltype(slot)::HasList)
                {
                    return 0; // unreachable
                }
                else
                {
                    using narrowed_t = typename decltype(slot)::narrowed_t;
                    const std::size_t stride = vd.size();
                    std::uint32_t n = 0;

                    for (const auto& w : *(v.*slot.list))
                    {
                        constructAt(vd, dst + n * stride, static_cast<narrowed_t>(w));
                        ++n;
                    }

                    return n;
                }
            });
        }
    } // anonymous namespace

    // ---- CompiledPredicate::Node lifetime ---------------------------------------

    CompiledPredicate::Node::Node(Node&& other) noexcept :
        kind(other.kind),
        op(other.op),
        arity(other.arity),
        subtreeSize(other.subtreeSize),
        offset(other.offset),
        propertySet(other.propertySet),
        valueDescriptor(other.valueDescriptor),
        rhs(other.rhs),
        rhsCount(other.rhsCount)
    {
        other.rhs = nullptr;
        other.rhsCount = 0;
    }

    CompiledPredicate::Node& CompiledPredicate::Node::operator=(Node&& other) noexcept
    {
        if (this == &other) return *this;
        this->~Node();
        new (this) Node(std::move(other));
        return *this;
    }

    CompiledPredicate::Node::~Node()
    {
        if (rhs == nullptr) return;
        const type::Descriptor<type::Typeless>& vd = *valueDescriptor;
        const std::size_t stride = vd.size();
        for (std::uint32_t i = 0; i < rhsCount; ++i)
        {
            vd.destruct(*reinterpret_cast<type::Typeless*>(rhs + i * stride));
        }
        freeRhsBuffer(rhs, vd.alignment());
    }

    // ---- compile ----------------------------------------------------------------

    namespace
    {
        // The predicate arrives as a flat pre-order vector, so transport-level
        // nesting limits do not bound it. Both compilation and evaluation
        // recurse once per tree level, so an unbounded predicate from a guest
        // could overflow the host's stack. Cap tree depth and total node count
        // before recursing.
        constexpr std::size_t MaxPredicateDepth = 32;
        constexpr std::size_t MaxPredicateNodes = 1024;

        // Walks the predicate tree in pre-order, validating and emitting one
        // CompiledPredicate::Node per source DotsPredicateNode. Validation
        // failures throw std::invalid_argument matching the original messages
        // produced by filter::validate().
        void compileNode(const vector_t<types::DotsPredicateNode>& src, std::size_t& cursor,
                         const type::StructDescriptor& sd,
                         std::vector<CompiledPredicate::Node>& out,
                         std::size_t depth)
        {
            using namespace types;
            if (depth > MaxPredicateDepth)
            {
                throw std::invalid_argument{
                    "predicate exceeds maximum nesting depth of " + std::to_string(MaxPredicateDepth) };
            }
            if (cursor >= src.size())
            {
                throw std::invalid_argument{ "predicate truncated: expected another node" };
            }

            const DotsPredicateNode& srcNode = src[cursor++];
            if (!srcNode.kind.isValid())
            {
                throw std::invalid_argument{ "predicate node is missing kind" };
            }

            const DotsPredicateKind kind = *srcNode.kind;
            CompiledPredicate::Node& node = out.emplace_back();
            node.kind = kind;

            switch (kind)
            {
                case DotsPredicateKind::leaf:
                {
                    if (!srcNode.leaf.isValid())
                    {
                        throw std::invalid_argument{ "leaf node missing leaf payload" };
                    }
                    const DotsPredicateLeaf& leaf = *srcNode.leaf;

                    if (!leaf.propertyTag.isValid() || !leaf.op.isValid())
                    {
                        throw std::invalid_argument{ "predicate leaf is missing propertyTag or op" };
                    }

                    const type::PropertyDescriptor* pd = sd.findPropertyByTag(*leaf.propertyTag);
                    if (pd == nullptr)
                    {
                        throw std::invalid_argument{
                            "property tag " + std::to_string(*leaf.propertyTag) +
                            " not found in type " + sd.name()
                        };
                    }

                    const type::Type t = pd->valueDescriptor().type();
                    const DotsCompareOp op = *leaf.op;

                    node.op = op;
                    node.offset = pd->offset().offset();
                    node.propertySet = pd->set();
                    node.valueDescriptor = &pd->valueDescriptor();

                    if (op == DotsCompareOp::isNull || op == DotsCompareOp::notNull)
                    {
                        return; // no rhs required
                    }

                    if (!leaf.value.isValid())
                    {
                        throw std::invalid_argument{
                            "predicate leaf is missing value (op=" + std::to_string(static_cast<int>(op)) +
                            ") on property '" + pd->name() + "'"
                        };
                    }

                    const bool wantList = (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn);

                    if (!valueSlotMatches(*leaf.value, t, wantList))
                    {
                        throw std::invalid_argument{
                            "predicate leaf value slot does not match property '" + pd->name() +
                            "' of type " + pd->valueDescriptor().name()
                        };
                    }

                    if (op == DotsCompareOp::lt || op == DotsCompareOp::le ||
                        op == DotsCompareOp::gt || op == DotsCompareOp::ge)
                    {
                        if (!isOrderedType(t))
                        {
                            throw std::invalid_argument{
                                "ordered comparison op on non-ordered property '" + pd->name() + "'"
                            };
                        }
                    }
                    else if (op == DotsCompareOp::eq || op == DotsCompareOp::neq ||
                             op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                    {
                        if (!isFundamentalEquatableType(t))
                        {
                            throw std::invalid_argument{
                                "equality op on non-equatable property '" + pd->name() + "'"
                            };
                        }
                    }

                    // Materialize rhs. For scalar ops a single typed slot; for
                    // isIn/notIn N back-to-back slots. Bool/duration cannot occur
                    // here as list types (validation above rejects wantList).
                    const type::Descriptor<type::Typeless>& vd = pd->valueDescriptor();
                    const std::size_t stride = vd.size();

                    if (wantList)
                    {
                        std::size_t srcCount = listSlotSize(*leaf.value, t);
                        if (srcCount > 0)
                        {
                            node.rhs = allocRhsBuffer(srcCount * stride, vd.alignment());
                            node.rhsCount = narrowList(vd, node.rhs, *leaf.value);
                        }
                    }
                    else
                    {
                        node.rhs = allocRhsBuffer(stride, vd.alignment());
                        narrowScalar(vd, node.rhs, *leaf.value);
                        node.rhsCount = 1;
                    }
                    return;
                }

                case DotsPredicateKind::andOp:
                case DotsPredicateKind::orOp:
                {
                    if (!srcNode.arity.isValid() || *srcNode.arity < 1)
                    {
                        throw std::invalid_argument{ "and/or node arity must be >= 1" };
                    }
                    const std::uint32_t arity = *srcNode.arity;
                    node.arity = arity;
                    // 'node' reference becomes invalid after subsequent emplace_back
                    // calls; refer to it by index past this point.
                    const std::size_t selfIndex = out.size() - 1;
                    for (std::uint32_t i = 0; i < arity; ++i)
                    {
                        compileNode(src, cursor, sd, out, depth + 1);
                    }
                    out[selfIndex].subtreeSize = static_cast<std::uint32_t>(out.size() - selfIndex);
                    return;
                }

                case DotsPredicateKind::notOp:
                {
                    if (!srcNode.arity.isValid() || *srcNode.arity != 1)
                    {
                        throw std::invalid_argument{ "not node arity must be 1" };
                    }
                    node.arity = 1;
                    const std::size_t selfIndex = out.size() - 1;
                    compileNode(src, cursor, sd, out, depth + 1);
                    out[selfIndex].subtreeSize = static_cast<std::uint32_t>(out.size() - selfIndex);
                    return;
                }
            }
        }

        // ---- evaluation -----------------------------------------------------

        bool evalCompiledNode(const std::vector<CompiledPredicate::Node>& nodes,
                              std::size_t& cursor,
                              const type::PropertyArea& area);

        bool evalCompiledLeaf(const CompiledPredicate::Node& n, const type::PropertyArea& area)
        {
            using namespace types;
            const bool isSet = n.propertySet <= area.validProperties();

            const DotsCompareOp op = n.op;
            if (op == DotsCompareOp::isNull)  return !isSet;
            if (op == DotsCompareOp::notNull) return isSet;
            if (!isSet) return false;

            const type::Descriptor<>& vd = *n.valueDescriptor;
            const type::Typeless& lhs = area.getProperty<type::Typeless>(n.offset);

            if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
            {
                const bool negate = (op == DotsCompareOp::notIn);
                const std::size_t stride = vd.size();
                for (std::uint32_t i = 0; i < n.rhsCount; ++i)
                {
                    const auto& rhs = *reinterpret_cast<const type::Typeless*>(n.rhs + i * stride);
                    if (vd.equal(lhs, rhs)) return !negate;
                }
                return negate;
            }

            const auto& rhs = *reinterpret_cast<const type::Typeless*>(n.rhs);
            switch (op)
            {
                case DotsCompareOp::eq:  return  vd.equal(lhs, rhs);
                case DotsCompareOp::neq: return !vd.equal(lhs, rhs);
                case DotsCompareOp::lt:  return  vd.less(lhs, rhs);
                case DotsCompareOp::le:  return  vd.lessEqual(lhs, rhs);
                case DotsCompareOp::gt:  return  vd.greater(lhs, rhs);
                case DotsCompareOp::ge:  return  vd.greaterEqual(lhs, rhs);
                default: return false;
            }
        }

        bool evalCompiledNode(const std::vector<CompiledPredicate::Node>& nodes,
                              std::size_t& cursor,
                              const type::PropertyArea& area)
        {
            using namespace types;
            const CompiledPredicate::Node& n = nodes[cursor++];

            switch (n.kind)
            {
                case DotsPredicateKind::leaf:
                    return evalCompiledLeaf(n, area);

                case DotsPredicateKind::andOp:
                {
                    bool result = true;
                    for (std::uint32_t i = 0; i < n.arity; ++i)
                    {
                        if (!result)
                        {
                            // short-circuit: skip the decided branch wholesale
                            cursor += nodes[cursor].subtreeSize;
                        }
                        else if (!evalCompiledNode(nodes, cursor, area))
                        {
                            result = false;
                        }
                    }
                    return result;
                }
                case DotsPredicateKind::orOp:
                {
                    bool result = false;
                    for (std::uint32_t i = 0; i < n.arity; ++i)
                    {
                        if (result)
                        {
                            // short-circuit: skip the decided branch wholesale
                            cursor += nodes[cursor].subtreeSize;
                        }
                        else if (evalCompiledNode(nodes, cursor, area))
                        {
                            result = true;
                        }
                    }
                    return result;
                }
                case DotsPredicateKind::notOp:
                    return !evalCompiledNode(nodes, cursor, area);
            }
            return false;
        }
    } // anonymous namespace

    CompiledPredicate::CompiledPredicate(const DotsPredicate& predicate,
                                         const type::StructDescriptor& descriptor)
    {
        if (!predicate.nodes.isValid() || predicate.nodes->empty())
        {
            return;
        }
        const auto& src = *predicate.nodes;
        if (src.size() > MaxPredicateNodes)
        {
            throw std::invalid_argument{
                "predicate exceeds maximum node count of " + std::to_string(MaxPredicateNodes) };
        }
        m_nodes.reserve(src.size());
        std::size_t cursor = 0;
        compileNode(src, cursor, descriptor, m_nodes, 0);
        if (cursor != src.size())
        {
            throw std::invalid_argument{ "predicate has extra nodes not attached to the tree" };
        }
    }

    bool CompiledPredicate::matches(const type::Struct& instance) const
    {
        if (m_nodes.empty()) return true;
        const type::PropertyArea& area = instance._descriptor().propertyArea(instance);
        std::size_t cursor = 0;
        return evalCompiledNode(m_nodes, cursor, area);
    }

    // ---- legacy free-function entry points --------------------------------------

    bool matches(const DotsPredicate& predicate, const type::Struct& instance)
    {
        if (!predicate.nodes.isValid() || predicate.nodes->empty())
        {
            return true;
        }
        CompiledPredicate compiled{ predicate, instance._descriptor() };
        return compiled.matches(instance);
    }

    void validate(const DotsPredicate& predicate, const type::StructDescriptor& descriptor)
    {
        CompiledPredicate{ predicate, descriptor };
    }
}
