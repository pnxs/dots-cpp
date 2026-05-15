// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/Filter.h>

#include <algorithm>
#include <stdexcept>
#include <string>

#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyDescriptor.h>
#include <dots/type/Struct.h>
#include <dots/type/StructDescriptor.h>
#include <dots/type/Typeless.h>

namespace dots::filter
{
    namespace
    {
        const type::PropertyDescriptor* findPropertyByTag(const type::StructDescriptor& sd, uint32_t tag)
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
            // Anything other than aggregate types (Vector/Struct).
            return t != type::Type::Vector && t != type::Type::Struct;
        }

        bool valueSlotMatches(const DotsPredicateValue& v, type::Type t, bool wantList)
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

        // ---- validation ----

        void validateLeaf(const DotsPredicateLeaf& leaf, const type::StructDescriptor& sd)
        {
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

            if (op == DotsCompareOp::isNull || op == DotsCompareOp::notNull)
            {
                return; // no value required, applicable to any property
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
        }

        void validateNode(const vector_t<DotsPredicateNode>& nodes, size_t& cursor,
                          const type::StructDescriptor& sd)
        {
            if (cursor >= nodes.size())
            {
                throw std::invalid_argument{ "predicate truncated: expected another node" };
            }

            const DotsPredicateNode& node = nodes[cursor++];
            if (!node.kind.isValid())
            {
                throw std::invalid_argument{ "predicate node is missing kind" };
            }

            switch (*node.kind)
            {
                case DotsPredicateKind::leaf:
                    if (!node.leaf.isValid())
                    {
                        throw std::invalid_argument{ "leaf node missing leaf payload" };
                    }
                    validateLeaf(*node.leaf, sd);
                    break;

                case DotsPredicateKind::andOp:
                case DotsPredicateKind::orOp:
                    if (!node.arity.isValid() || *node.arity < 1)
                    {
                        throw std::invalid_argument{ "and/or node arity must be >= 1" };
                    }
                    for (uint32_t i = 0; i < *node.arity; ++i)
                    {
                        validateNode(nodes, cursor, sd);
                    }
                    break;

                case DotsPredicateKind::notOp:
                    if (!node.arity.isValid() || *node.arity != 1)
                    {
                        throw std::invalid_argument{ "not node arity must be 1" };
                    }
                    validateNode(nodes, cursor, sd);
                    break;
            }
        }

        // ---- evaluation ----

        bool evalNode(const vector_t<DotsPredicateNode>& nodes, size_t& cursor,
                      const type::Struct& instance);

        // Compare an instance property (read as Typeless bytes) against a typed rhs value.
        template <typename T>
        bool cmpScalar(const type::Descriptor<>& vd, const type::Typeless& lhs,
                       const T& rhs, DotsCompareOp op)
        {
            const type::Typeless& rhsTL = type::Typeless::From(rhs);
            switch (op)
            {
                case DotsCompareOp::eq:  return  vd.equal(lhs, rhsTL);
                case DotsCompareOp::neq: return !vd.equal(lhs, rhsTL);
                case DotsCompareOp::lt:  return  vd.less(lhs, rhsTL);
                case DotsCompareOp::le:  return  vd.lessEqual(lhs, rhsTL);
                case DotsCompareOp::gt:  return  vd.greater(lhs, rhsTL);
                case DotsCompareOp::ge:  return  vd.greaterEqual(lhs, rhsTL);
                default: return false; // isIn/notIn handled in inList()
            }
        }

        template <typename WireT, typename NarrowT>
        bool inListNarrowed(const type::Descriptor<>& vd, const type::Typeless& lhs,
                            const vector_t<WireT>& list, bool negate)
        {
            for (const WireT& wireItem : list)
            {
                NarrowT narrowed = static_cast<NarrowT>(wireItem);
                const type::Typeless& rhsTL = type::Typeless::From(narrowed);
                if (vd.equal(lhs, rhsTL)) return !negate;
            }
            return negate;
        }

        template <typename T>
        bool inListDirect(const type::Descriptor<>& vd, const type::Typeless& lhs,
                          const vector_t<T>& list, bool negate)
        {
            for (const T& item : list)
            {
                const type::Typeless& rhsTL = type::Typeless::From(item);
                if (vd.equal(lhs, rhsTL)) return !negate;
            }
            return negate;
        }

        bool evalLeaf(const DotsPredicateLeaf& leaf, const type::Struct& instance)
        {
            const type::StructDescriptor& sd = instance._descriptor();
            const type::PropertyDescriptor* pd = findPropertyByTag(sd, *leaf.propertyTag);
            if (pd == nullptr) return false;

            const type::PropertyArea& area = sd.propertyArea(instance);
            const bool isSet = pd->set() <= area.validProperties();

            const DotsCompareOp op = *leaf.op;
            if (op == DotsCompareOp::isNull) return !isSet;
            if (op == DotsCompareOp::notNull) return isSet;
            if (!isSet) return false; // unset property never satisfies a value comparison

            const type::Descriptor<>& vd = pd->valueDescriptor();
            const type::Typeless& lhs = area.getProperty<type::Typeless>(pd->offset());
            const DotsPredicateValue& v = *leaf.value;
            const bool negate = (op == DotsCompareOp::notIn);

            switch (vd.type())
            {
                case type::Type::boolean:
                    return cmpScalar(vd, lhs, *v.boolVal, op);

                case type::Type::int8: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<int64_t, int8_t>(vd, lhs, *v.intList, negate);
                    int8_t narrowed = static_cast<int8_t>(*v.intVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::int16: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<int64_t, int16_t>(vd, lhs, *v.intList, negate);
                    int16_t narrowed = static_cast<int16_t>(*v.intVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::int32: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<int64_t, int32_t>(vd, lhs, *v.intList, negate);
                    int32_t narrowed = static_cast<int32_t>(*v.intVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::int64: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<int64_t>(vd, lhs, *v.intList, negate);
                    int64_t narrowed = *v.intVal;
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::uint8: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<uint64_t, uint8_t>(vd, lhs, *v.uintList, negate);
                    uint8_t narrowed = static_cast<uint8_t>(*v.uintVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::uint16: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<uint64_t, uint16_t>(vd, lhs, *v.uintList, negate);
                    uint16_t narrowed = static_cast<uint16_t>(*v.uintVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::uint32: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<uint64_t, uint32_t>(vd, lhs, *v.uintList, negate);
                    uint32_t narrowed = static_cast<uint32_t>(*v.uintVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::uint64: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<uint64_t>(vd, lhs, *v.uintList, negate);
                    uint64_t narrowed = *v.uintVal;
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::float32: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListNarrowed<float64_t, float32_t>(vd, lhs, *v.floatList, negate);
                    float32_t narrowed = static_cast<float32_t>(*v.floatVal);
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::float64: {
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<float64_t>(vd, lhs, *v.floatList, negate);
                    float64_t narrowed = *v.floatVal;
                    return cmpScalar(vd, lhs, narrowed, op);
                }
                case type::Type::string:
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<string_t>(vd, lhs, *v.stringList, negate);
                    return cmpScalar(vd, lhs, *v.stringVal, op);

                case type::Type::timepoint:
                case type::Type::steady_timepoint:
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<timepoint_t>(vd, lhs, *v.timepointList, negate);
                    return cmpScalar(vd, lhs, *v.timepointVal, op);

                case type::Type::duration:
                    return cmpScalar(vd, lhs, *v.durationVal, op);

                case type::Type::uuid:
                    if (op == DotsCompareOp::isIn || op == DotsCompareOp::notIn)
                        return inListDirect<uuid_t>(vd, lhs, *v.uuidList, negate);
                    return cmpScalar(vd, lhs, *v.uuidVal, op);

                default:
                    return false; // unsupported type — caught by validate()
            }
        }

        bool evalNode(const vector_t<DotsPredicateNode>& nodes, size_t& cursor,
                      const type::Struct& instance)
        {
            const DotsPredicateNode& node = nodes[cursor++];

            switch (*node.kind)
            {
                case DotsPredicateKind::leaf:
                    return evalLeaf(*node.leaf, instance);

                case DotsPredicateKind::andOp: {
                    bool result = true;
                    for (uint32_t i = 0; i < *node.arity; ++i)
                    {
                        if (!evalNode(nodes, cursor, instance)) result = false;
                        // continue walking children to keep cursor consistent
                    }
                    return result;
                }
                case DotsPredicateKind::orOp: {
                    bool result = false;
                    for (uint32_t i = 0; i < *node.arity; ++i)
                    {
                        if (evalNode(nodes, cursor, instance)) result = true;
                    }
                    return result;
                }
                case DotsPredicateKind::notOp:
                    return !evalNode(nodes, cursor, instance);
            }
            return false;
        }
    } // namespace

    bool matches(const DotsPredicate& predicate, const type::Struct& instance)
    {
        if (!predicate.nodes.isValid() || predicate.nodes->empty())
        {
            return true; // empty predicate matches all
        }
        size_t cursor = 0;
        return evalNode(*predicate.nodes, cursor, instance);
    }

    void validate(const DotsPredicate& predicate, const type::StructDescriptor& descriptor)
    {
        if (!predicate.nodes.isValid() || predicate.nodes->empty())
        {
            return; // empty predicate is always valid
        }
        size_t cursor = 0;
        validateNode(*predicate.nodes, cursor, descriptor);
        if (cursor != predicate.nodes->size())
        {
            throw std::invalid_argument{ "predicate has extra nodes not attached to the tree" };
        }
    }
}
