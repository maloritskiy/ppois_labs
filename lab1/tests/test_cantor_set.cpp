#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "cantor_set.h"

namespace {

CantorSet parse(const std::string& text) { return CantorSet::fromString(text); }

CantorSet makeAtoms(int count) {
    CantorSet result;
    for (int i = 0; i < count; ++i) {
        result.add("x" + std::to_string(i));
    }
    return result;
}

}  // namespace

TEST(CantorSetTest, NewSetIsEmpty) {
    CantorSet set;
    EXPECT_TRUE(set.isEmpty());
    EXPECT_EQ(set.size(), 0);
}

TEST(CantorSetTest, AddRejectsDuplicates) {
    CantorSet set;
    EXPECT_TRUE(set.add("a"));
    EXPECT_FALSE(set.add("a"));
    EXPECT_FALSE(set.isEmpty());
    EXPECT_EQ(set.size(), 1);
}

TEST(CantorSetTest, AtomAndSetWithSameNameAreDifferentElements) {
    CantorSet set;
    EXPECT_TRUE(set.add("a"));
    EXPECT_TRUE(set.add(parse("{a}")));
    EXPECT_EQ(set.size(), 2);
}

TEST(CantorSetTest, SetCanContainSets) {
    CantorSet set;
    EXPECT_TRUE(set.add(parse("{a, b}")));
    EXPECT_FALSE(set.add(parse("{b, a}")));
    EXPECT_TRUE(set.add(CantorSet()));
    EXPECT_EQ(set.size(), 2);
}

TEST(CantorSetTest, InvalidAtomIsRejected) {
    CantorSet set;
    EXPECT_THROW(set.add(""), std::invalid_argument);
    EXPECT_THROW(set.add("a b"), std::invalid_argument);
    EXPECT_THROW(set.add("a,b"), std::invalid_argument);
    EXPECT_THROW(set.add("{a}"), std::invalid_argument);
    EXPECT_TRUE(set.isEmpty());
}

TEST(CantorSetTest, SetGrowsBeyondInitialCapacity) {
    CantorSet set = makeAtoms(20);
    EXPECT_EQ(set.size(), 20);
    EXPECT_TRUE(set["x0"]);
    EXPECT_TRUE(set["x19"]);
}

TEST(CantorSetTest, RemoveDeletesExistingElementOnly) {
    CantorSet set = parse("{a, {b}, c}");
    EXPECT_TRUE(set.remove("a"));
    EXPECT_TRUE(set.remove(parse("{b}")));
    EXPECT_FALSE(set.remove("missing"));
    EXPECT_FALSE(set.remove(parse("{missing}")));
    EXPECT_EQ(set, parse("{c}"));
}

TEST(CantorSetTest, MembershipIsCheckedByIndexOperator) {
    CantorSet set = parse("{a, {b, c}}");
    EXPECT_TRUE(set["a"]);
    EXPECT_TRUE(set[parse("{c, b}")]);
    EXPECT_FALSE(set["b"]);
    EXPECT_FALSE(set[parse("{a}")]);
}

TEST(CantorSetTest, UnionContainsElementsOfBoth) {
    CantorSet result = parse("{a, b}") + parse("{b, c}");
    EXPECT_EQ(result, parse("{a, b, c}"));
}

TEST(CantorSetTest, IntersectionKeepsCommonElements) {
    CantorSet result = parse("{a, b, {c}}") * parse("{b, {c}, d}");
    EXPECT_EQ(result, parse("{b, {c}}"));
}

TEST(CantorSetTest, DifferenceRemovesElementsOfSecond) {
    CantorSet result = parse("{a, b, c}") - parse("{b, d}");
    EXPECT_EQ(result, parse("{a, c}"));
}

TEST(CantorSetTest, CompoundOperatorsChangeLeftOperand) {
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

TEST(CantorSetTest, EqualityIgnoresOrder) {
    EXPECT_EQ(parse("{{a, b}, c}"), parse("{c, {b, a}}"));
    EXPECT_NE(parse("{a, b}"), parse("{a, c}"));
    EXPECT_NE(parse("{a}"), parse("{a, b}"));
    EXPECT_NE(parse("{{a}}"), parse("{{b}}"));
}

TEST(CantorSetTest, CopyConstructorMakesIndependentCopy) {
    CantorSet original = parse("{a, {b}}");
    CantorSet copy(original);
    original.add("c");
    original.add(parse("{d}"));
    EXPECT_EQ(copy, parse("{a, {b}}"));
}

TEST(CantorSetTest, AssignmentMakesIndependentCopy) {
    CantorSet original = parse("{a, {b}}");
    CantorSet assigned = parse("{x, y, z, {w}}");
    assigned = original;
    original.remove("a");
    EXPECT_EQ(assigned, parse("{a, {b}}"));
}

TEST(CantorSetTest, SelfAssignmentKeepsValue) {
    CantorSet set = parse("{a, {b}}");
    CantorSet& alias = set;
    set = alias;
    EXPECT_EQ(set, parse("{a, {b}}"));
}

TEST(CantorSetTest, PowerSetOfEmptySetHasOneElement) {
    CantorSet empty;
    EXPECT_EQ(empty.powerSet(), parse("{{}}"));
}

TEST(CantorSetTest, PowerSetOfTwoElements) {
    EXPECT_EQ(parse("{a, b}").powerSet(), parse("{{}, {a}, {b}, {a, b}}"));
}

TEST(CantorSetTest, PowerSetHasTwoToTheNElements) {
    EXPECT_EQ(makeAtoms(5).powerSet().size(), 32);
}

TEST(CantorSetTest, PowerSetOfNestedSetsWorks) {
    CantorSet result = parse("{a, {b}}").powerSet();
    EXPECT_TRUE(result[parse("{{b}}")]);
    EXPECT_TRUE(result[parse("{a, {b}}")]);
    EXPECT_EQ(result.size(), 4);
}

TEST(CantorSetTest, PowerSetAtLimitIsBuilt) {
    CantorSet result = makeAtoms(CantorSet::kMaxPowerSetSize).powerSet();
    EXPECT_EQ(result.size(), 1 << CantorSet::kMaxPowerSetSize);
}

TEST(CantorSetTest, PowerSetAboveLimitThrows) {
    CantorSet large = makeAtoms(CantorSet::kMaxPowerSetSize + 1);
    EXPECT_THROW(large.powerSet(), std::length_error);
}

TEST(CantorSetParseTest, ParsesExampleFromTheTask) {
    CantorSet set = parse("{a, b, c, {a, b}, {}, {a, {c}}}");
    EXPECT_EQ(set.size(), 6);
    EXPECT_TRUE(set[parse("{a, {c}}")]);
    EXPECT_TRUE(set[parse("{}")]);
}

TEST(CantorSetParseTest, ParsesEmptySetAndIgnoresSpaces) {
    EXPECT_TRUE(parse("  {  }  ").isEmpty());
    EXPECT_EQ(parse("{ a ,b,{ c } }"), parse("{a, b, {c}}"));
}

TEST(CantorSetParseTest, DuplicatesCollapse) {
    EXPECT_EQ(parse("{a, a, {b}, {b}}").size(), 2);
}

TEST(CantorSetParseTest, MalformedTextIsRejected) {
    const char* const bad[] = {"", "a", "{a", "a}", "{a,}", "{,a}", "{a b}", "{a}}", "{{a}", "{a,,b}", "{a} {b}"};
    for (const char* text : bad) {
        SCOPED_TRACE(text);
        EXPECT_THROW(CantorSet::fromString(text), std::invalid_argument);
    }
}

TEST(CantorSetParseTest, TooDeepNestingIsRejected) {
    std::string text = std::string(300, '{') + std::string(300, '}');
    EXPECT_THROW(CantorSet::fromString(text), std::invalid_argument);
}

TEST(CantorSetParseTest, ToStringRoundTrips) {
    CantorSet set = parse("{a, {b, {}}, {c}}");
    EXPECT_EQ(set.toString(), "{a, {b, {}}, {c}}");
    EXPECT_EQ(parse(set.toString()), set);
}

TEST(CantorSetStreamTest, OutputMatchesToString) {
    CantorSet set = parse("{a, {b}}");
    std::ostringstream out;
    out << set;
    EXPECT_EQ(out.str(), set.toString());
}

TEST(CantorSetStreamTest, InputReadsOneSetPerLine) {
    std::istringstream in("{a, {b}}\n{c}\n");
    CantorSet first;
    CantorSet second;
    in >> first >> second;
    EXPECT_FALSE(in.fail());
    EXPECT_EQ(first, parse("{a, {b}}"));
    EXPECT_EQ(second, parse("{c}"));
}

TEST(CantorSetStreamTest, InputFailsOnBadData) {
    const char* const bad[] = {"abc\n", "{a\n", "{a,}\n", ""};
    for (const char* text : bad) {
        SCOPED_TRACE(text);
        std::istringstream in(text);
        CantorSet set;
        in >> set;
        EXPECT_TRUE(in.fail());
    }
}
