#include <gtest/gtest.h>

#include <string>

#include "test_support.hpp"
#include "ui/set_cli.hpp"

using lab1::testing_support::contains;
using lab1::testing_support::runCli;
using lab1::ui::SetCli;

namespace {

// Номера пунктов меню SetCli.
const std::string kSetA = "1\n";
const std::string kSetB = "2\n";
const std::string kAddElement = "3\n";
const std::string kRemoveElement = "4\n";
const std::string kCheckMembership = "5\n";
const std::string kCardinality = "6\n";
const std::string kUnion = "7\n";
const std::string kIntersection = "8\n";
const std::string kDifference = "9\n";
const std::string kPowerSet = "10\n";
const std::string kCompare = "11\n";
const std::string kExit = "0\n";

std::string run(const std::string& script) { return runCli<SetCli>(script + kExit); }

}  // namespace

TEST(SetCliTest, EntersSetsFromStrings) {
    const std::string output = run(kSetA + "{a, {b}}\n" + kSetB + "{c}\n");
    EXPECT_TRUE(contains(output, "A = {a, {b}}"));
    EXPECT_TRUE(contains(output, "B = {c}"));
}

TEST(SetCliTest, ShowsUnionIntersectionAndDifference) {
    const std::string sets = kSetA + "{a, b}\n" + kSetB + "{b, c}\n";
    const std::string output = run(sets + kUnion + kIntersection + kDifference);
    EXPECT_TRUE(contains(output, "A + B = {a, b, c}"));
    EXPECT_TRUE(contains(output, "A * B = {b}"));
    EXPECT_TRUE(contains(output, "A - B = {a}"));
}

TEST(SetCliTest, AddsAndRemovesElements) {
    const std::string output =
        run(kAddElement + "x\n" + kAddElement + "x\n" + kRemoveElement + "x\n" + kRemoveElement + "x\n");
    EXPECT_TRUE(contains(output, "Элемент добавлен"));
    EXPECT_TRUE(contains(output, "Элемент уже есть в множестве"));
    EXPECT_TRUE(contains(output, "Элемент удалён"));
    EXPECT_TRUE(contains(output, "Такого элемента в множестве нет"));
}

TEST(SetCliTest, ChecksMembershipOfAtomsAndSets) {
    const std::string output =
        run(kSetA + "{a, {b}}\n" + kCheckMembership + "a\n" + kCheckMembership + "{b}\n" + kCheckMembership + "b\n");
    EXPECT_TRUE(contains(output, "Элемент принадлежит A"));
    EXPECT_TRUE(contains(output, "Элемент не принадлежит A"));
}

TEST(SetCliTest, ShowsCardinalityAndEmptiness) {
    const std::string output = run(kCardinality + kSetA + "{a, b}\n" + kCardinality);
    EXPECT_TRUE(contains(output, "Мощность A: 0, множество пустое"));
    EXPECT_TRUE(contains(output, "Мощность A: 2, множество непустое"));
}

TEST(SetCliTest, BuildsPowerSet) {
    const std::string output = run(kSetA + "{a, b}\n" + kPowerSet);
    EXPECT_TRUE(contains(output, "P(A) = {{}, {a}, {b}, {a, b}}"));
}

TEST(SetCliTest, ComparesSets) {
    const std::string equal = run(kSetA + "{a, b}\n" + kSetB + "{b, a}\n" + kCompare);
    const std::string different = run(kSetA + "{a}\n" + kCompare);
    EXPECT_TRUE(contains(equal, "A и B равны"));
    EXPECT_TRUE(contains(different, "A и B различны"));
}

TEST(SetCliTest, ReportsSyntaxErrorsAndKeepsRunning) {
    const std::string output = run(kSetA + "{a,\n" + kCardinality);
    EXPECT_TRUE(contains(output, "Ошибка:"));
    EXPECT_TRUE(contains(output, "Мощность A: 0"));
}

TEST(SetCliTest, ReportsInvalidElement) {
    const std::string output = run(kAddElement + "a b\n");
    EXPECT_TRUE(contains(output, "Ошибка:"));
}

TEST(SetCliTest, ReportsTooLargePowerSet) {
    std::string text = "{";
    for (int index = 0; index < 17; ++index) {
        text += (index > 0 ? ", x" : "x") + std::to_string(index);
    }
    text += "}\n";
    const std::string output = run(kSetA + text + kPowerSet);
    EXPECT_TRUE(contains(output, "Ошибка:"));
}

TEST(SetCliTest, EndOfInputInsideActionsEndsQuietly) {
    for (const std::string& action : {kSetA, kAddElement, kRemoveElement, kCheckMembership}) {
        SCOPED_TRACE(action);
        const std::string output = runCli<SetCli>(action);
        EXPECT_FALSE(contains(output, "Ошибка"));
    }
}
