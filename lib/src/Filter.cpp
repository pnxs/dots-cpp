// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/Filter.h>

#include <algorithm>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>

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
        const type::PropertyDescriptor* findPropertyByTag(const type::StructDescriptor& sd, std::uint32_t tag)
        {
            const auto& pds = sd.propertyDescriptors();
            auto it = std::find_if(pds.begin(), pds.end(),
                [tag](const type::PropertyDescriptor& pd) { return pd.tag() == tag; });
            return it == pds.end() ? nullptr : &*it;
        }

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

        bool valueSlotMatches(const types::DotsPredicateValue& v, type::Type t, bool wantList)
        {
            if (wantList)
            {
                switch (t)
                {
                    case type::Type::int8:    case type::Type::int16:
                    case type::Type::int32:   case type::Type::int64:
                        return v.intList.isValid();
                    case type::Type::uint8:   case type::Type::uint16:
                    case type::Type::uint32:  case type::Type::uint64:
                        return v.uintList.isValid();
                    case type::Type::float32: case type::Type::float64:
                        return v.floatList.isValid();
                    case type::Type::string:    return v.stringList.isValid();
                    case type::Type::timepoint: case type::Type::steady_timepoint:
                        return v.timepointList.isValid();
                    case type::Type::uuid:      return v.uuidList.isValid();
                    default: return false;
                }
            }
            switch (t)
            {
                case type::Type::boolean:   return v.boolVal.isValid();
                case type::Type::int8:      case type::Type::int16:
                case type::Type::int32:     case type::Type::int64:
                    return v.intVal.isValid();
                case type::Type::uint8:     case type::Type::uint16:
                case type::Type::uint32:    case type::Type::uint64:
                    return v.uintVal.isValid();
                case type::Type::float32:   case type::Type::float64:
                    return v.floatVal.isValid();
                case type::Type::string:    return v.stringVal.isValid();
                case type::Type::timepoint: case type::Type::steady_timepoint:
                    return v.timepointVal.isValid();
                case type::Type::duration:  return v.durationVal.isValid();
                case type::Type::uuid:      return v.uuidVal.isValid();
                default: return false;
            }
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
            switch (vd.type())
            {
                case type::Type::boolean:   constructAt(vd, dst, *v.boolVal); break;
                case type::Type::int8:      constructAt(vd, dst, static_cast<types::int8_t>(*v.intVal)); break;
                case type::Type::int16:     constructAt(vd, dst, static_cast<types::int16_t>(*v.intVal)); break;
                case type::Type::int32:     constructAt(vd, dst, static_cast<types::int32_t>(*v.intVal)); break;
                case type::Type::int64:     constructAt(vd, dst, static_cast<types::int64_t>(*v.intVal)); break;
                case type::Type::uint8:     constructAt(vd, dst, static_cast<types::uint8_t>(*v.uintVal)); break;
                case type::Type::uint16:    constructAt(vd, dst, static_cast<types::uint16_t>(*v.uintVal)); break;
                case type::Type::uint32:    constructAt(vd, dst, static_cast<types::uint32_t>(*v.uintVal)); break;
                case type::Type::uint64:    constructAt(vd, dst, static_cast<types::uint64_t>(*v.uintVal)); break;
                case type::Type::float32:   constructAt(vd, dst, static_cast<types::float32_t>(*v.floatVal)); break;
                case type::Type::float64:   constructAt(vd, dst, static_cast<types::float64_t>(*v.floatVal)); break;
                case type::Type::string:    constructAt(vd, dst, *v.stringVal); break;
                case type::Type::timepoint: case type::Type::steady_timepoint:
                                            constructAt(vd, dst, *v.timepointVal); break;
                case type::Type::duration:  constructAt(vd, dst, *v.durationVal); break;
                case type::Type::uuid:      constructAt(vd, dst, *v.uuidVal); break;
                default: break; // unreachable: rejected by validation above
            }
        }

        // Narrow a list of wire values into N back-to-back typed slots starting at dst.
        // Returns the number of elements written.
        std::uint32_t narrowList(const type::Descriptor<type::Typeless>& vd, std::byte* dst,
                                 const types::DotsPredicateValue& v)
        {
            const std::size_t stride = vd.size();
            std::uint32_t n = 0;
            auto step = [&]() { dst += stride; ++n; };

            switch (vd.type())
            {
                case type::Type::int8:    for (auto w : *v.intList) { constructAt(vd, dst, static_cast<types::int8_t>(w)); step(); } break;
                case type::Type::int16:   for (auto w : *v.intList) { constructAt(vd, dst, static_cast<types::int16_t>(w)); step(); } break;
                case type::Type::int32:   for (auto w : *v.intList) { constructAt(vd, dst, static_cast<types::int32_t>(w)); step(); } break;
                case type::Type::int64:   for (auto w : *v.intList) { constructAt(vd, dst, static_cast<types::int64_t>(w)); step(); } break;
                case type::Type::uint8:   for (auto w : *v.uintList) { constructAt(vd, dst, static_cast<types::uint8_t>(w)); step(); } break;
                case type::Type::uint16:  for (auto w : *v.uintList) { constructAt(vd, dst, static_cast<types::uint16_t>(w)); step(); } break;
                case type::Type::uint32:  for (auto w : *v.uintList) { constructAt(vd, dst, static_cast<types::uint32_t>(w)); step(); } break;
                case type::Type::uint64:  for (auto w : *v.uintList) { constructAt(vd, dst, static_cast<types::uint64_t>(w)); step(); } break;
                case type::Type::float32: for (auto w : *v.floatList) { constructAt(vd, dst, static_cast<types::float32_t>(w)); step(); } break;
                case type::Type::float64: for (auto w : *v.floatList) { constructAt(vd, dst, static_cast<types::float64_t>(w)); step(); } break;
                case type::Type::string:  for (const auto& w : *v.stringList) { constructAt(vd, dst, w); step(); } break;
                case type::Type::timepoint:
                case type::Type::steady_timepoint:
                                          for (const auto& w : *v.timepointList) { constructAt(vd, dst, w); step(); } break;
                case type::Type::uuid:    for (const auto& w : *v.uuidList) { constructAt(vd, dst, w); step(); } break;
                default: break; // unreachable
            }
            return n;
        }
    } // anonymous namespace

    // ---- CompiledPredicate::Node lifetime ---------------------------------------

    CompiledPredicate::Node::Node(Node&& other) noexcept :
        kind(other.kind),
        op(other.op),
        arity(other.arity),
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
        // Walks the predicate tree in pre-order, validating and emitting one
        // CompiledPredicate::Node per source DotsPredicateNode. Validation
        // failures throw std::invalid_argument matching the original messages
        // produced by filter::validate().
        void compileNode(const vector_t<types::DotsPredicateNode>& src, std::size_t& cursor,
                         const type::StructDescriptor& sd,
                         std::vector<CompiledPredicate::Node>& out)
        {
            using namespace types;
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

                    const type::PropertyDescriptor* pd = findPropertyByTag(sd, *leaf.propertyTag);
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
                        std::size_t srcCount = 0;
                        switch (t)
                        {
                            case type::Type::int8:    case type::Type::int16:
                            case type::Type::int32:   case type::Type::int64:
                                srcCount = leaf.value->intList->size(); break;
                            case type::Type::uint8:   case type::Type::uint16:
                            case type::Type::uint32:  case type::Type::uint64:
                                srcCount = leaf.value->uintList->size(); break;
                            case type::Type::float32: case type::Type::float64:
                                srcCount = leaf.value->floatList->size(); break;
                            case type::Type::string:
                                srcCount = leaf.value->stringList->size(); break;
                            case type::Type::timepoint:
                            case type::Type::steady_timepoint:
                                srcCount = leaf.value->timepointList->size(); break;
                            case type::Type::uuid:
                                srcCount = leaf.value->uuidList->size(); break;
                            default: break;
                        }
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
                    // calls; do not touch it past this point.
                    for (std::uint32_t i = 0; i < arity; ++i)
                    {
                        compileNode(src, cursor, sd, out);
                    }
                    return;
                }

                case DotsPredicateKind::notOp:
                {
                    if (!srcNode.arity.isValid() || *srcNode.arity != 1)
                    {
                        throw std::invalid_argument{ "not node arity must be 1" };
                    }
                    node.arity = 1;
                    compileNode(src, cursor, sd, out);
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
                        if (!evalCompiledNode(nodes, cursor, area)) result = false;
                        // walk remaining children to keep cursor consistent
                    }
                    return result;
                }
                case DotsPredicateKind::orOp:
                {
                    bool result = false;
                    for (std::uint32_t i = 0; i < n.arity; ++i)
                    {
                        if (evalCompiledNode(nodes, cursor, area)) result = true;
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
        m_nodes.reserve(src.size());
        std::size_t cursor = 0;
        compileNode(src, cursor, descriptor, m_nodes);
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
