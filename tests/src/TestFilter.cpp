// SPDX-License-Identifier: LGPL-3.0-only
#include <dots/testing/gtest/gtest.h>
#include <dots/Filter.h>
#include <DotsTestStruct.dots.h>

using namespace dots;
using namespace dots::types;

namespace
{
    // Tag references for DotsTestStruct from lib/src/model/dotstesttypes.dots:
    //   1: stringField (string)
    //   2: indKeyfField (int32, key)
    //   3: floatField (float32)
    //   4: enumField (DotsTestEnum)
    //   5: tp (timepoint)
    //   6: subStruct (DotsTestSubStruct)
    //   7: uuid (uuid)
    //   8: uint64Field (uint64)
    //   9: int64Field (int64)

    DotsPredicateNode leafNode(uint32_t tag, DotsCompareOp op, DotsPredicateValue value)
    {
        DotsPredicateLeaf leaf;
        leaf.propertyTag = tag;
        leaf.op = op;
        if (op != DotsCompareOp::isNull && op != DotsCompareOp::notNull)
            leaf.value = std::move(value);

        DotsPredicateNode node;
        node.kind = DotsPredicateKind::leaf;
        node.leaf = std::move(leaf);
        return node;
    }

    DotsPredicateNode opNode(DotsPredicateKind kind, uint32_t arity)
    {
        DotsPredicateNode node;
        node.kind = kind;
        node.arity = arity;
        return node;
    }

    DotsPredicate predicate(std::initializer_list<DotsPredicateNode> nodes)
    {
        DotsPredicate p;
        p.nodes = vector_t<DotsPredicateNode>{ nodes };
        return p;
    }

    DotsPredicateValue intV(int64_t v)
    {
        DotsPredicateValue pv;
        pv.intVal = v;
        return pv;
    }
    DotsPredicateValue uintV(uint64_t v)
    {
        DotsPredicateValue pv;
        pv.uintVal = v;
        return pv;
    }
    DotsPredicateValue strV(std::string v)
    {
        DotsPredicateValue pv;
        pv.stringVal = std::move(v);
        return pv;
    }
    DotsTestStruct sample(int32_t key, std::string s, int64_t i64, uint64_t u64)
    {
        DotsTestStruct dts;
        dts.indKeyfField = key;
        dts.stringField = std::move(s);
        dts.int64Field = i64;
        dts.uint64Field = u64;
        dts.floatField = 3.14f;
        return dts;
    }
}

TEST(TestFilter, empty_predicate_matches_everything)
{
    DotsPredicate empty;
    DotsTestStruct dts = sample(1, "x", 0, 0);
    EXPECT_TRUE(filter::matches(empty, dts));
}

TEST(TestFilter, eq_on_int32_key)
{
    auto p = predicate({ leafNode(2, DotsCompareOp::eq, intV(42)) });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE(filter::matches(p, sample(42, "x", 0, 0)));
    EXPECT_FALSE(filter::matches(p, sample(43, "x", 0, 0)));
}

TEST(TestFilter, neq_on_string)
{
    auto p = predicate({ leafNode(1, DotsCompareOp::neq, strV("hello")) });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE(filter::matches(p, sample(1, "world", 0, 0)));
    EXPECT_FALSE(filter::matches(p, sample(1, "hello", 0, 0)));
}

TEST(TestFilter, ordered_ops_on_int64)
{
    auto lt100 = predicate({ leafNode(9, DotsCompareOp::lt, intV(100)) });
    auto ge50  = predicate({ leafNode(9, DotsCompareOp::ge, intV(50))  });

    EXPECT_TRUE (filter::matches(lt100, sample(1, "", 99, 0)));
    EXPECT_FALSE(filter::matches(lt100, sample(1, "", 100, 0)));
    EXPECT_TRUE (filter::matches(ge50,  sample(1, "", 50, 0)));
    EXPECT_FALSE(filter::matches(ge50,  sample(1, "", 49, 0)));
}

TEST(TestFilter, ordered_ops_on_uint64)
{
    auto gt1000 = predicate({ leafNode(8, DotsCompareOp::gt, uintV(1000)) });
    filter::validate(gt1000, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(gt1000, sample(1, "", 0, 1001)));
    EXPECT_FALSE(filter::matches(gt1000, sample(1, "", 0, 1000)));
}

TEST(TestFilter, is_null_and_not_null)
{
    DotsTestStruct withString = sample(1, "set", 0, 0);
    DotsTestStruct withoutString;
    withoutString.indKeyfField = 1;
    // stringField left unset

    auto pIsNull  = predicate({ leafNode(1, DotsCompareOp::isNull,  {}) });
    auto pNotNull = predicate({ leafNode(1, DotsCompareOp::notNull, {}) });

    EXPECT_FALSE(filter::matches(pIsNull,  withString));
    EXPECT_TRUE (filter::matches(pIsNull,  withoutString));
    EXPECT_TRUE (filter::matches(pNotNull, withString));
    EXPECT_FALSE(filter::matches(pNotNull, withoutString));
}

TEST(TestFilter, is_in_on_int32)
{
    DotsPredicateValue v;
    v.intList = vector_t<int64_t>{ 10, 20, 30 };
    auto p = predicate({ leafNode(2, DotsCompareOp::isIn, std::move(v)) });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(p, sample(20, "", 0, 0)));
    EXPECT_FALSE(filter::matches(p, sample(25, "", 0, 0)));
}

TEST(TestFilter, not_in_on_string)
{
    DotsPredicateValue v;
    v.stringList = vector_t<string_t>{ "a", "b", "c" };
    auto p = predicate({ leafNode(1, DotsCompareOp::notIn, std::move(v)) });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(p, sample(1, "d", 0, 0)));
    EXPECT_FALSE(filter::matches(p, sample(1, "b", 0, 0)));
}

TEST(TestFilter, and_n_ary)
{
    // and(stringField == "ok", int64Field >= 100, uint64Field < 1000)
    auto p = predicate({
        opNode(DotsPredicateKind::andOp, 3),
        leafNode(1, DotsCompareOp::eq, strV("ok")),
        leafNode(9, DotsCompareOp::ge, intV(100)),
        leafNode(8, DotsCompareOp::lt, uintV(1000)),
    });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(p, sample(1, "ok",  100, 999)));
    EXPECT_FALSE(filter::matches(p, sample(1, "ok",   99, 999)));  // int64 fails
    EXPECT_FALSE(filter::matches(p, sample(1, "no",  100, 999)));  // string fails
    EXPECT_FALSE(filter::matches(p, sample(1, "ok",  100, 1000))); // uint64 fails
}

TEST(TestFilter, or_n_ary)
{
    // or(indKeyfField == 1, indKeyfField == 7, indKeyfField == 42)
    auto p = predicate({
        opNode(DotsPredicateKind::orOp, 3),
        leafNode(2, DotsCompareOp::eq, intV(1)),
        leafNode(2, DotsCompareOp::eq, intV(7)),
        leafNode(2, DotsCompareOp::eq, intV(42)),
    });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(p, sample(1,  "", 0, 0)));
    EXPECT_TRUE (filter::matches(p, sample(7,  "", 0, 0)));
    EXPECT_TRUE (filter::matches(p, sample(42, "", 0, 0)));
    EXPECT_FALSE(filter::matches(p, sample(3,  "", 0, 0)));
}

TEST(TestFilter, not_unary)
{
    // not(stringField == "skip")
    auto p = predicate({
        opNode(DotsPredicateKind::notOp, 1),
        leafNode(1, DotsCompareOp::eq, strV("skip")),
    });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_FALSE(filter::matches(p, sample(1, "skip",  0, 0)));
    EXPECT_TRUE (filter::matches(p, sample(1, "other", 0, 0)));
}

TEST(TestFilter, nested_and_or_not)
{
    // (key == 1 && string != "drop") || !(int64 < 0)
    auto p = predicate({
        opNode(DotsPredicateKind::orOp, 2),
            opNode(DotsPredicateKind::andOp, 2),
                leafNode(2, DotsCompareOp::eq,  intV(1)),
                leafNode(1, DotsCompareOp::neq, strV("drop")),
            opNode(DotsPredicateKind::notOp, 1),
                leafNode(9, DotsCompareOp::lt, intV(0)),
    });
    filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance());

    EXPECT_TRUE (filter::matches(p, sample(1, "ok",   0, 0)));  // left matches
    EXPECT_TRUE (filter::matches(p, sample(2, "drop", 5, 0)));  // !int64<0 matches
    EXPECT_FALSE(filter::matches(p, sample(2, "drop",-1, 0)));  // both fail
}

TEST(TestFilter, validate_rejects_unknown_property_tag)
{
    auto p = predicate({ leafNode(99, DotsCompareOp::eq, intV(1)) });
    EXPECT_THROW(filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance()), std::invalid_argument);
}

TEST(TestFilter, validate_rejects_wrong_value_slot)
{
    // indKeyfField is int32; uintVal slot is wrong.
    auto p = predicate({ leafNode(2, DotsCompareOp::eq, uintV(1)) });
    EXPECT_THROW(filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance()), std::invalid_argument);
}

TEST(TestFilter, validate_rejects_arity_mismatch_truncated)
{
    // andOp claims arity 2 but only one child follows.
    auto p = predicate({
        opNode(DotsPredicateKind::andOp, 2),
        leafNode(2, DotsCompareOp::eq, intV(1)),
    });
    EXPECT_THROW(filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance()), std::invalid_argument);
}

TEST(TestFilter, validate_rejects_extra_nodes)
{
    // Two top-level leaves with no joining and/or — extra node left unattached.
    auto p = predicate({
        leafNode(2, DotsCompareOp::eq, intV(1)),
        leafNode(2, DotsCompareOp::eq, intV(2)),
    });
    EXPECT_THROW(filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance()), std::invalid_argument);
}

TEST(TestFilter, validate_rejects_not_arity_not_one)
{
    auto p = predicate({
        opNode(DotsPredicateKind::notOp, 2),
        leafNode(2, DotsCompareOp::eq, intV(1)),
        leafNode(2, DotsCompareOp::eq, intV(2)),
    });
    EXPECT_THROW(filter::validate(p, dots::type::Descriptor<DotsTestStruct>::Instance()), std::invalid_argument);
}
