// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <DotsFilter.dots.h>
#include <DotsPredicate.dots.h>

namespace dots::type
{
    struct Struct;
    struct StructDescriptor;
}

namespace dots::filter
{
    /*!
     * @brief Evaluate a predicate against a struct instance.
     *
     * The predicate must have been validated against the instance's
     * descriptor (see validate()); calling matches() with a malformed
     * or type-mismatched predicate is undefined behavior.
     *
     * An empty or default-constructed predicate matches everything.
     *
     * @param predicate The predicate to evaluate.
     * @param instance  The struct instance to test.
     * @return true  if the instance matches the predicate.
     */
    bool matches(const DotsPredicate& predicate, const type::Struct& instance);

    /*!
     * @brief Validate a predicate against a target type descriptor.
     *
     * Checks:
     *  - Pre-order well-formedness (every internal node's declared arity
     *    matches the number of children that actually follow it; no
     *    leftover nodes).
     *  - Every leaf's propertyTag exists in the descriptor.
     *  - Every leaf's value slot matches the property's type.
     *  - Every leaf's op is applicable to the property's type
     *    (e.g. ordered ops only on numeric / string / time properties).
     *
     * @param predicate  The predicate to validate.
     * @param descriptor The target type's descriptor.
     *
     * @exception std::invalid_argument Thrown on any validation failure.
     *            The message identifies the offending node where possible.
     */
    void validate(const DotsPredicate& predicate, const type::StructDescriptor& descriptor);
}
