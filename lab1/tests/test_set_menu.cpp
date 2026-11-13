#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "set_menu.h"

namespace {

const std::string kInputSetA = "1\n";
const std::string kInputSetB = "2\n";
const std::string kAddElement = "3\n";
const std::string kRemoveElement = "4\n";
const std::string kCheckElement = "5\n";
const std::string kShowSize = "6\n";
const std::string kUnion = "7\n";
const std::string kIntersection = "8\n";
const std::string kDifference = "9\n";
const std::string kPowerSet = "10\n";
const std::string kCompare = "11\n";
const std::string kExit = "0\n";

std::string runMenu(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    SetMenu menu(in, out);
    menu.run();
    return out.str();
}

bool contains(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

}  // namespace

TEST(SetMenuTest, ShowsSetsAndMenuItems) {
    std::string output = runMenu(kInputSetA + "{a, {b}}\n" + kInputSetB + "{c}\n" + kExit);
    EXPECT_TRUE(contains(output, "A = {a, {b}}"));
    EXPECT_TRUE(contains(output, "B = {c}"));
    EXPECT_TRUE(contains(output, "7. Объединение A + B"));
    EXPECT_TRUE(contains(output, "0. Выход"));
}

TEST(SetMenuTest, ShowsUnionIntersectionAndDifference) {
    std::string sets = kInputSetA + "{a, b}\n" + kInputSetB + "{b, c}\n";
    std::string output = runMenu(sets + kUnion + kIntersection + kDifference + kExit);
    EXPECT_TRUE(contains(output, "A + B = {a, b, c}"));
    EXPECT_TRUE(contains(output, "A * B = {b}"));
    EXPECT_TRUE(contains(output, "A - B = {a}"));
}

TEST(SetMenuTest, AddsAndRemovesAtoms) {
    std::string script = kAddElement + "x\n" + kAddElement + "x\n" + kRemoveElement + "x\n" + kRemoveElement + "x\n";
    std::string output = runMenu(script + kExit);
    EXPECT_TRUE(contains(output, "Элемент добавлен"));
    EXPECT_TRUE(contains(output, "Такой элемент уже есть"));
    EXPECT_TRUE(contains(output, "Элемент удалён"));
    EXPECT_TRUE(contains(output, "Такого элемента нет"));
}

TEST(SetMenuTest, AddsAndRemovesSets) {
    std::string script = kAddElement + "{a}\n" + kAddElement + "{a}\n" + kRemoveElement + "{a}\n" + kRemoveElement + "{a}\n";
    std::string output = runMenu(script + kExit);
    EXPECT_TRUE(contains(output, "Элемент добавлен"));
    EXPECT_TRUE(contains(output, "Такой элемент уже есть"));
    EXPECT_TRUE(contains(output, "Элемент удалён"));
    EXPECT_TRUE(contains(output, "Такого элемента нет"));
}

TEST(SetMenuTest, ChecksMembershipOfAtomsAndSets) {
    std::string script = kInputSetA + "{a, {b}}\n" + kCheckElement + "a\n" + kCheckElement + "{b}\n" + kCheckElement + "b\n" +
                         kCheckElement + "{c}\n";
    std::string output = runMenu(script + kExit);
    EXPECT_TRUE(contains(output, "Элемент принадлежит A"));
    EXPECT_TRUE(contains(output, "Элемент не принадлежит A"));
}

TEST(SetMenuTest, ShowsSizeAndEmptiness) {
    std::string output = runMenu(kShowSize + kInputSetA + "{a, b}\n" + kShowSize + kExit);
    EXPECT_TRUE(contains(output, "Мощность A: 0, множество пустое"));
    EXPECT_TRUE(contains(output, "Мощность A: 2, множество непустое"));
}

TEST(SetMenuTest, BuildsPowerSet) {
    std::string output = runMenu(kInputSetA + "{a, b}\n" + kPowerSet + kExit);
    EXPECT_TRUE(contains(output, "P(A) = {{}, {a}, {b}, {a, b}}"));
}

TEST(SetMenuTest, ComparesSets) {
    std::string equal = runMenu(kInputSetA + "{a, b}\n" + kInputSetB + "{b, a}\n" + kCompare + kExit);
    std::string different = runMenu(kInputSetA + "{a}\n" + kCompare + kExit);
    EXPECT_TRUE(contains(equal, "A и B равны"));
    EXPECT_TRUE(contains(different, "A и B различны"));
}

TEST(SetMenuTest, ReportsErrorsAndKeepsRunning) {
    EXPECT_TRUE(contains(runMenu(kInputSetA + "{a,\n" + kShowSize + kExit), "Ошибка:"));
    EXPECT_TRUE(contains(runMenu(kInputSetA + "{a,\n" + kShowSize + kExit), "Мощность A: 0"));
    EXPECT_TRUE(contains(runMenu(kAddElement + "a b\n" + kExit), "Ошибка:"));
}

TEST(SetMenuTest, ReportsTooLargePowerSet) {
    std::string text = "{";
    for (int i = 0; i < CantorSet::kMaxPowerSetSize + 1; ++i) {
        text += (i > 0 ? ", x" : "x") + std::to_string(i);
    }
    text += "}\n";
    EXPECT_TRUE(contains(runMenu(kInputSetA + text + kPowerSet + kExit), "Ошибка:"));
}

TEST(SetMenuTest, ReportsWrongMenuInput) {
    std::string output = runMenu("abc\n99\n" + kExit);
    EXPECT_TRUE(contains(output, "Введите номер пункта"));
    EXPECT_TRUE(contains(output, "Нет такого пункта"));
}

TEST(SetMenuTest, EndOfInputInsideActionEndsQuietly) {
    const std::string* actions[] = {&kInputSetA, &kInputSetB, &kAddElement, &kRemoveElement, &kCheckElement};
    for (const std::string* action : actions) {
        EXPECT_FALSE(contains(runMenu(*action), "Ошибка"));
    }
}
