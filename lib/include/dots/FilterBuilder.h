// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <optional>
#include <utility>
#include <type_traits>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/PropertySet.h>
#include <DotsFilter.dots.h>
#include <DotsPredicate.dots.h>

namespace dots::filter
{
    // -------------------------------------------------------------------------
    // Predicate accumulator. Internally a pre-order vector of DotsPredicateNode
    // matching the wire form. Combinators (&, |, !) collapse n-ary ops in place.
    // -------------------------------------------------------------------------
    struct Predicate
    {
        vector_t<DotsPredicateNode> nodes;
    };

    // -------------------------------------------------------------------------
    // Internal: populate a DotsPredicateValue's slot based on a typed value.
    // The slot choice mirrors the width-unified wire format (intVal/uintVal/
    // floatVal/etc.); narrowing happens server-side per the property's actual
    // type.
    // -------------------------------------------------------------------------
    namespace detail
    {
        template <typename T>
        DotsPredicateValue makeValue(const T& x)
        {
            DotsPredicateValue v;
            if constexpr (std::is_same_v<T, bool_t>)
                v.boolVal = x;
            else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
                v.intVal = static_cast<int64_t>(x);
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
                v.uintVal = static_cast<uint64_t>(x);
            else if constexpr (std::is_same_v<T, float32_t> || std::is_same_v<T, float64_t>)
                v.floatVal = static_cast<float64_t>(x);
            else if constexpr (std::is_same_v<T, string_t>)
                v.stringVal = x;
            else if constexpr (std::is_same_v<T, timepoint_t>)
                v.timepointVal = x;
            else if constexpr (std::is_same_v<T, duration_t>)
                v.durationVal = x;
            else if constexpr (std::is_same_v<T, uuid_t>)
                v.uuidVal = x;
            else
                static_assert(sizeof(T) == 0, "unsupported value type for predicate leaf");
            return v;
        }

        template <typename T, typename Range>
        DotsPredicateValue makeListValue(const Range& xs)
        {
            DotsPredicateValue v;
            if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
            {
                vector_t<int64_t> out;
                for (const auto& x : xs) out.push_back(static_cast<int64_t>(x));
                v.intList = std::move(out);
            }
            else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
            {
                vector_t<uint64_t> out;
                for (const auto& x : xs) out.push_back(static_cast<uint64_t>(x));
                v.uintList = std::move(out);
            }
            else if constexpr (std::is_same_v<T, float32_t> || std::is_same_v<T, float64_t>)
            {
                vector_t<float64_t> out;
                for (const auto& x : xs) out.push_back(static_cast<float64_t>(x));
                v.floatList = std::move(out);
            }
            else if constexpr (std::is_same_v<T, string_t>)
            {
                vector_t<string_t> out;
                for (const auto& x : xs) out.push_back(x);
                v.stringList = std::move(out);
            }
            else if constexpr (std::is_same_v<T, timepoint_t>)
            {
                vector_t<timepoint_t> out;
                for (const auto& x : xs) out.push_back(x);
                v.timepointList = std::move(out);
            }
            else if constexpr (std::is_same_v<T, uuid_t>)
            {
                vector_t<uuid_t> out;
                for (const auto& x : xs) out.push_back(x);
                v.uuidList = std::move(out);
            }
            else
                static_assert(sizeof(T) == 0, "unsupported value type for predicate list");
            return v;
        }

        inline Predicate singleLeaf(uint32_t tag, DotsCompareOp op, std::optional<DotsPredicateValue> value)
        {
            DotsPredicateLeaf leaf;
            leaf.propertyTag = tag;
            leaf.op = op;
            if (value.has_value()) leaf.value = std::move(*value);

            DotsPredicateNode node;
            node.kind = DotsPredicateKind::leaf;
            node.leaf = std::move(leaf);

            Predicate p;
            p.nodes.push_back(std::move(node));
            return p;
        }

        // Returns the root node's kind, or `leaf` if the predicate is empty.
        inline DotsPredicateKind rootKind(const Predicate& p)
        {
            return p.nodes.empty() ? DotsPredicateKind::leaf : *p.nodes.front().kind;
        }
    }

    // -------------------------------------------------------------------------
    // Combinators with n-ary collapse: `(a & b) & c` becomes a single andOp
    // node with arity 3, not a binary chain.
    // -------------------------------------------------------------------------
    inline Predicate combine(DotsPredicateKind op, Predicate lhs, Predicate rhs)
    {
        // An empty (default-constructed) predicate acts as the identity of the
        // combinator, so the accumulator pattern `Predicate p; p = p | leaf;`
        // works. Without this, the emitted head would count the empty side in
        // its arity while appending zero nodes — a corrupt tree the host
        // rejects as truncated.
        if (lhs.nodes.empty()) return rhs;
        if (rhs.nodes.empty()) return lhs;

        const bool lhsIsSame = detail::rootKind(lhs) == op;
        const bool rhsIsSame = detail::rootKind(rhs) == op;

        const uint32_t lhsArity = lhsIsSame ? *lhs.nodes.front().arity : 1;
        const uint32_t rhsArity = rhsIsSame ? *rhs.nodes.front().arity : 1;

        DotsPredicateNode head;
        head.kind = op;
        head.arity = lhsArity + rhsArity;

        Predicate out;
        out.nodes.push_back(std::move(head));

        auto append = [&](Predicate& src, bool isSame)
        {
            auto first = src.nodes.begin();
            if (isSame) ++first; // skip the matching head; flatten its children
            for (auto it = first; it != src.nodes.end(); ++it)
                out.nodes.push_back(std::move(*it));
        };
        append(lhs, lhsIsSame);
        append(rhs, rhsIsSame);

        return out;
    }

    inline Predicate operator&(Predicate lhs, Predicate rhs)
    {
        return combine(DotsPredicateKind::andOp, std::move(lhs), std::move(rhs));
    }

    inline Predicate operator|(Predicate lhs, Predicate rhs)
    {
        return combine(DotsPredicateKind::orOp, std::move(lhs), std::move(rhs));
    }

    inline Predicate operator!(Predicate p)
    {
        // Negating an empty predicate stays empty (no constraint) instead of
        // emitting a notOp head with no child.
        if (p.nodes.empty()) return p;

        DotsPredicateNode head;
        head.kind = DotsPredicateKind::notOp;
        head.arity = 1u;

        Predicate out;
        out.nodes.push_back(std::move(head));
        for (auto& n : p.nodes) out.nodes.push_back(std::move(n));
        return out;
    }

    // -------------------------------------------------------------------------
    // Filter builder — fluent surface that produces a wire DotsFilter.
    //   predicate(p)            // rows only, all columns
    //   project(mask)           // all rows, masked columns
    //   predicate(p).project(m) // both
    // -------------------------------------------------------------------------
    struct Filter
    {
        std::optional<Predicate>         pred;
        std::optional<property_set_t>    mask;

        Filter& project(property_set_t m) & { mask = m; return *this; }
        Filter&& project(property_set_t m) && { mask = m; return std::move(*this); }

        // Build the wire-form DotsFilter. Consumes the builder.
        DotsFilter build() &&
        {
            DotsFilter f;
            if (pred.has_value() && !pred->nodes.empty())
            {
                DotsPredicate wp;
                wp.nodes = std::move(pred->nodes);
                f.predicate = std::move(wp);
            }
            if (mask.has_value()) f.propertyMask = *mask;
            return f;
        }
    };

    inline Filter predicate(Predicate p) { return Filter{ std::move(p), std::nullopt }; }
    inline Filter project(property_set_t m) { return Filter{ std::nullopt, m }; }

    // -------------------------------------------------------------------------
    // Typed property handle. Constexpr — knows propertyTag and value_t at
    // compile time, so wrong-type comparisons fail to compile.
    // -------------------------------------------------------------------------
    template <typename P>
    struct Attr
    {
        using value_t = typename P::value_t;
        // Not constexpr: with DOTS_PROPERTIES_NO_CONSTEXPR_OFFSETS (e.g. Clang)
        // P::Metadata is not a constant expression. The tag is only ever needed
        // at runtime (as a singleLeaf argument), so read it lazily.
        static uint32_t tag() { return P::Metadata.tag(); }

        Predicate eq      (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::eq,  detail::makeValue(v)); }
        Predicate neq     (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::neq, detail::makeValue(v)); }
        Predicate lt      (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::lt,  detail::makeValue(v)); }
        Predicate le      (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::le,  detail::makeValue(v)); }
        Predicate gt      (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::gt,  detail::makeValue(v)); }
        Predicate ge      (const value_t& v) const { return detail::singleLeaf(tag(), DotsCompareOp::ge,  detail::makeValue(v)); }
        Predicate isNull  () const { return detail::singleLeaf(tag(), DotsCompareOp::isNull,  std::nullopt); }
        Predicate notNull () const { return detail::singleLeaf(tag(), DotsCompareOp::notNull, std::nullopt); }

        template <typename Range>
        Predicate isIn(const Range& xs) const { return detail::singleLeaf(tag(), DotsCompareOp::isIn,  detail::makeListValue<value_t>(xs)); }

        template <typename Range>
        Predicate notIn(const Range& xs) const { return detail::singleLeaf(tag(), DotsCompareOp::notIn, detail::makeListValue<value_t>(xs)); }

        Predicate isIn(std::initializer_list<value_t> xs) const { return isIn<std::initializer_list<value_t>>(xs); }
        Predicate notIn(std::initializer_list<value_t> xs) const { return notIn<std::initializer_list<value_t>>(xs); }

        // Operator forms — same as eq/neq/lt/le/gt/ge.
        Predicate operator==(const value_t& v) const { return eq (v); }
        Predicate operator!=(const value_t& v) const { return neq(v); }
        Predicate operator< (const value_t& v) const { return lt (v); }
        Predicate operator<=(const value_t& v) const { return le (v); }
        Predicate operator> (const value_t& v) const { return gt (v); }
        Predicate operator>=(const value_t& v) const { return ge (v); }
    };

    template <typename P>
    inline constexpr Attr<P> attr{};
}
