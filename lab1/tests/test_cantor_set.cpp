#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "set/cantor_set.hpp"

using lab1::CantorSet;

namespace {

CantorSet parse(const std::string& text) { return CantorSet::parse(text); }

CantorSet makeAtoms(std::size_t count) {
    CantorSet result;
    for (std::size_t index = 0; index < count; ++index) {
        result.add("x" + std::to_string(index));
    }
    return result;
}

}  // namespace

TEST(CantorSetTest, NewSetIsEmpty) {
    const CantorSet set;
    EXPECT_TRUE(set.isEmpty());
    EXPECT_EQ(set.cardinality(), 0U);
}

TEST(CantorSetTest, AddRejectsDuplicates) {
    CantorSet set;
    EXPECT_TRUE(set.add("a"));
    EXPECT_FALSE(set.add("a"));
    EXPECT_FALSE(set.isEmpty());
    EXPECT_EQ(set.cardinality(), 1U);
}

TEST(CantorSetTest, InitializerListDropsDuplicates) {
    const CantorSet set{"a", "b", "a"};
    EXPECT_EQ(set.cardinality(), 2U);
}

TEST(CantorSetTest, SetCanContainSets) {
    CantorSet set;
    EXPECT_TRUE(set.add(parse("{a, b}")));
    EXPECT_FALSE(set.add(parse("{b, a}")));
    EXPECT_TRUE(set.add(CantorSet()));
    EXPECT_EQ(set.cardinality(), 2U);
}

TEST(CantorSetTest, RemoveDeletesExistingElementOnly) {
    CantorSet set = parse("{a, {b}, c}");
    EXPECT_TRUE(set.remove("a"));
    EXPECT_TRUE(set.remove(parse("{b}")));
    EXPECT_FALSE(set.remove("missing"));
    EXPECT_EQ(set, parse("{c}"));
}

TEST(CantorSetTest, MembershipIsCheckedByIndexOperator) {
    const CantorSet set = parse("{a, {b, c}}");
    EXPECT_TRUE(set["a"]);
    EXPECT_TRUE(set[parse("{c, b}")]);
    EXPECT_FALSE(set["b"]);
    EXPECT_TRUE(set.contains("a"));
}

TEST(CantorSetTest, UnionContainsElementsOfBoth) {
    const CantorSet result = parse("{a, b}") + parse("{b, c}");
    EXPECT_EQ(result, parse("{a, b, c}"));
}

TEST(CantorSetTest, IntersectionKeepsCommonElements) {
    const CantorSet result = parse("{a, b, {c}}") * parse("{b, {c}, d}");
    EXPECT_EQ(result, parse("{b, {c}}"));
}

TEST(CantorSetTest, DifferenceRemovesElementsOfSecond) {
    const CantorSet result = parse("{a, b, c}") - parse("{b, d}");
    EXPECT_EQ(result, parse("{a, c}"));
}

TEST(CantorSetTest, CompoundOperatorsModifyLeftOperand) {
    CantorSet set = parse("{a, b}");
    set += parse("{c}");
    EXPECT_EQ(set, parse("{a, b, c}"));
    set *= parse("{b, c, d}");
    EXPECT_EQ(set, parse("{b, c}"));
    set -= parse("{c}");
    EXPECT_EQ(set, parse("{b}"));
}

TEST(CantorSetTest, OperationsWithItselfAreSafe) {
    CantorSet united = parse("{a, {b}}");
    united += united;
    EXPECT_EQ(united, parse("{a, {b}}"));
    CantorSet intersected = parse("{a, {b}}");
    intersected *= intersected;
    EXPECT_EQ(intersected, parse("{a, {b}}"));
    CantorSet subtracted = parse("{a, {b}}");
    subtracted -= subtracted;
    EXPECT_TRUE(subtracted.isEmpty());
}

TEST(CantorSetTest, EqualityIgnoresOrderAndComparesNestedSets) {
    EXPECT_EQ(parse("{{a, b}, c}"), parse("{c, {b, a}}"));
    EXPECT_NE(parse("{a, b}"), parse("{a, c}"));
    EXPECT_NE(parse("{a}"), parse("{a, b}"));
    EXPECT_NE(parse("{{a}}"), parse("{{b}}"));
}

TEST(CantorSetTest, CopyIsIndependentOfOriginal) {
    CantorSet original = parse("{a, {b}}");
    CantorSet copy(original);
    CantorSet assigned;
    assigned = original;
    original.add("c");
    EXPECT_EQ(copy, parse("{a, {b}}"));
    EXPECT_EQ(assigned, parse("{a, {b}}"));
}

TEST(CantorSetTest, PowerSetOfEmptySetHasOneElement) {
    const CantorSet empty;
    EXPECT_EQ(empty.powerSet(), parse("{{}}"));
}

TEST(CantorSetTest, PowerSetOfTwoElements) {
    const CantorSet result = parse("{a, b}").powerSet();
    EXPECT_EQ(result, parse("{{}, {a}, {b}, {a, b}}"));
}

TEST(CantorSetTest, PowerSetHasTwoToTheNElements) {
    const std::size_t count = 5;
    EXPECT_EQ(makeAtoms(count).powerSet().cardinality(), std::size_t{1} << count);
}

TEST(CantorSetTest, PowerSetOfNestedSetsWorks) {
    const CantorSet result = parse("{a, {b}}").powerSet();
    EXPECT_TRUE(result[parse("{{b}}")]);
    EXPECT_TRUE(result[parse("{a, {b}}")]);
    EXPECT_EQ(result.cardinality(), 4U);
}

TEST(CantorSetTest, PowerSetAtLimitIsBuilt) {
    const CantorSet result = makeAtoms(CantorSet::kMaxPowerSetCardinality).powerSet();
    EXPECT_EQ(result.cardinality(), std::size_t{1} << CantorSet::kMaxPowerSetCardinality);
}

TEST(CantorSetTest, PowerSetAboveLimitThrows) {
    const CantorSet large = makeAtoms(CantorSet::kMaxPowerSetCardinality + 1);
    EXPECT_THROW(large.powerSet(), std::length_error);
}

TEST(CantorSetParseTest, ParsesExampleFromTheTask) {
    const CantorSet set = parse("{a, b, c, {a, b}, {}, {a, {c}}}");
    EXPECT_EQ(set.cardinality(), 6U);
    EXPECT_TRUE(set[parse("{a, {c}}")]);
    EXPECT_TRUE(set[parse("{}")]);
}

TEST(CantorSetParseTest, ParsesEmptySetAndIgnoresSpaces) {
    EXPECT_TRUE(parse("  {  }  ").isEmpty());
    EXPECT_EQ(parse("{ a ,b,{ c } }"), parse("{a, b, {c}}"));
}

TEST(CantorSetParseTest, DuplicatesCollapse) {
    EXPECT_EQ(parse("{a, a, {b}, {b}}").cardinality(), 2U);
}

TEST(CantorSetParseTest, MalformedTextIsRejected) {
    const std::vector<std::string> bad = {"",     "a",    "{a",   "a}",   "{a,}",    "{,a}",
                                          "{a b}", "{a}}", "{{a}", "{a,,b}", "{a} {b}"};
    for (const std::string& text : bad) {
        SCOPED_TRACE(text);
        EXPECT_THROW(CantorSet::parse(text), std::invalid_argument);
    }
}

TEST(CantorSetParseTest, TooDeepNestingIsRejected) {
    const std::size_t depth = 300;
    const std::string text = std::string(depth, '{') + std::string(depth, '}');
    EXPECT_THROW(CantorSet::parse(text), std::invalid_argument);
}

TEST(CantorSetParseTest, ToStringRoundTrips) {
    const CantorSet set = parse("{a, {b, {}}, {c}}");
    EXPECT_EQ(parse(set.toString()), set);
    EXPECT_EQ(set.toString(), "{a, {b, {}}, {c}}");
}

TEST(CantorSetStreamTest, OutputMatchesToString) {
    const CantorSet set = parse("{a, {b}}");
    std::ostringstream out;
    out << set;
    EXPECT_EQ(out.str(), set.toString());
}

TEST(CantorSetStreamTest, InputReadsSeveralSetsInARow) {
    std::istringstream in("  {a, {b}}  {c}");
    CantorSet first;
    CantorSet second;
    in >> first >> second;
    EXPECT_TRUE(in);
    EXPECT_EQ(first, parse("{a, {b}}"));
    EXPECT_EQ(second, parse("{c}"));
}

TEST(CantorSetStreamTest, InputFailsOnBadData) {
    for (const std::string text : {"abc", "{a", "{a,}", ""}) {
        SCOPED_TRACE(text);
        std::istringstream in(text);
        CantorSet set;
        in >> set;
        EXPECT_TRUE(in.fail());
    }
}
