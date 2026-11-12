#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "set/cantor_set.hpp"
#include "set/element.hpp"

using lab1::CantorSet;
using lab1::Element;

TEST(ElementTest, AtomKeepsItsName) {
    const Element element("alpha");
    EXPECT_FALSE(element.isSet());
    EXPECT_EQ(element.atom(), "alpha");
    EXPECT_EQ(element.toString(), "alpha");
}

TEST(ElementTest, AtomCanBeBuiltFromStdString) {
    const std::string name = "beta";
    const Element element(name);
    EXPECT_EQ(element.atom(), "beta");
}

TEST(ElementTest, InvalidAtomsAreRejected) {
    for (const std::string text : {"", "a b", "a,b", "{a", "a}", " "}) {
        SCOPED_TRACE(text);
        EXPECT_FALSE(Element::isValidAtom(text));
        EXPECT_THROW(Element{text}, std::invalid_argument);
    }
}

TEST(ElementTest, SubsetElementKeepsNestedSet) {
    const CantorSet inner{"a", "b"};
    const Element element(inner);
    EXPECT_TRUE(element.isSet());
    EXPECT_EQ(element.subset(), inner);
    EXPECT_EQ(element.toString(), "{a, b}");
}

TEST(ElementTest, WrongAccessorThrowsLogicError) {
    const CantorSet inner{"a"};
    const Element atom("x");
    const Element nested(inner);
    EXPECT_THROW(atom.subset(), std::logic_error);
    EXPECT_THROW(nested.atom(), std::logic_error);
}

TEST(ElementTest, EqualityComparesKindAndContent) {
    const CantorSet first{"a", "b"};
    const CantorSet reordered{"b", "a"};
    EXPECT_EQ(Element("a"), Element("a"));
    EXPECT_NE(Element("a"), Element("b"));
    EXPECT_NE(Element("a"), Element(CantorSet{"a"}));
    EXPECT_EQ(Element(first), Element(reordered));
}

TEST(ElementTest, CopyIsDeep) {
    CantorSet inner{"a"};
    const Element original(inner);
    Element copy(original);
    Element assigned("x");
    assigned = original;
    EXPECT_EQ(copy, original);
    EXPECT_EQ(assigned, original);
    EXPECT_NE(&copy.subset(), &original.subset());
}

TEST(ElementTest, SelfAssignmentKeepsValue) {
    const CantorSet inner{"a"};
    Element element(inner);
    const Element& alias = element;
    element = alias;
    EXPECT_EQ(element.toString(), "{a}");
}

TEST(ElementTest, MoveTransfersValue) {
    const CantorSet inner{"a"};
    Element source(inner);
    Element moved(std::move(source));
    EXPECT_EQ(moved.toString(), "{a}");
    Element target("x");
    target = std::move(moved);
    EXPECT_EQ(target.toString(), "{a}");
}

TEST(ElementTest, ParseDistinguishesAtomsAndSets) {
    EXPECT_EQ(Element::parse("  abc  "), Element("abc"));
    EXPECT_TRUE(Element::parse("{a, {b}}").isSet());
}

TEST(ElementTest, ParseRejectsBadText) {
    EXPECT_THROW(Element::parse("   "), std::invalid_argument);
    EXPECT_THROW(Element::parse("{a"), std::invalid_argument);
    EXPECT_THROW(Element::parse("a b"), std::invalid_argument);
}

TEST(ElementTest, StreamOutputUsesToString) {
    std::ostringstream out;
    out << Element("a");
    EXPECT_EQ(out.str(), "a");
}
