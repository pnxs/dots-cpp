// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <DotsFilter.dots.h>
#include <DotsPredicate.dots.h>
#include <dots/type/PropertySet.h>

namespace dots::type
{
    struct Struct;
    struct StructDescriptor;
    template <typename T> struct Descriptor;
}

namespace dots::filter
{
    /*!
     * @brief A pre-compiled, descriptor-bound form of a DotsPredicate.
     *
     * Construction performs the same validation as filter::validate() but
     * additionally resolves every leaf against the target descriptor and
     * narrows every right-hand-side value to the property's actual type.
     *
     * Subsequent matches() calls skip property-lookup, type-switching, and
     * per-element casting. Build once at subscribe time, evaluate many.
     *
     * An empty / default-constructed instance matches everything.
     */
    class CompiledPredicate
    {
    public:
        CompiledPredicate() noexcept = default;
        CompiledPredicate(const DotsPredicate& predicate, const type::StructDescriptor& descriptor);

        CompiledPredicate(CompiledPredicate&&) noexcept = default;
        CompiledPredicate& operator=(CompiledPredicate&&) noexcept = default;
        CompiledPredicate(const CompiledPredicate&) = delete;
        CompiledPredicate& operator=(const CompiledPredicate&) = delete;
        ~CompiledPredicate() = default;

        bool empty() const noexcept { return m_nodes.empty(); }
        bool matches(const type::Struct& instance) const;

        // Internal compiled-node layout. Exposed only so that the eval helpers
        // defined in Filter.cpp can name the type; users should not depend on
        // its shape.
        struct Node
        {
            DotsPredicateKind         kind{};
            DotsCompareOp             op{};
            std::uint32_t             arity = 0;
            // Number of nodes in this subtree including the node itself.
            // Lets evaluation skip decided and/or branches wholesale.
            std::uint32_t             subtreeSize = 1;
            std::size_t               offset = 0;
            type::PropertySet         propertySet{};
            const type::Descriptor<>* valueDescriptor = nullptr;
            // Owned aligned buffer holding count consecutive elements of
            // valueDescriptor's type (1 for scalar ops; >=1 for isIn/notIn;
            // 0 for inner / null-test nodes).
            std::byte*                rhs = nullptr;
            std::uint32_t             rhsCount = 0;

            Node() = default;
            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&& other) noexcept;
            Node& operator=(Node&& other) noexcept;
            ~Node();
        };

    private:
        std::vector<Node> m_nodes;
    };

    /*!
     * @brief Evaluate a predicate against a struct instance.
     *
     * Convenience overload that compiles the predicate on each call.
     * Use CompiledPredicate directly when the same predicate is
     * evaluated more than once.
     *
     * An empty or default-constructed predicate matches everything.
     */
    bool matches(const DotsPredicate& predicate, const type::Struct& instance);

    /*!
     * @brief Validate a predicate against a target type descriptor.
     *
     * Equivalent to constructing a CompiledPredicate and discarding it.
     * Throws std::invalid_argument on any structural or type mismatch.
     */
    void validate(const DotsPredicate& predicate, const type::StructDescriptor& descriptor);
}
