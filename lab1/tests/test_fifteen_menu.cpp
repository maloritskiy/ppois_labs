#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "fifteen_menu.h"

namespace {

const std::string kNewGame = "1\n";
const std::string kMakeMove = "2\n";
const std::string kCheckSolved = "3\n";
const std::string kLoadLayout = "4\n";
const std::string kExit = "0\n";

const std::string kNearSolvedLayout = "3 1 2 3 4 5 6 7 0 8\n";

std::string runMenu(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    FifteenMenu menu(in, out);
    menu.run();
    return out.str();
}

bool contains(const std::string& text, const std::string& part) { return text.find(part) != std::string::npos; }

}  // namespace

TEST(RenderBoardTest, DrawsEmptyCellAsDot) {
    const int layout[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
    EXPECT_EQ(renderBoard(FifteenPuzzle(3, layout)), "1 2 3\n4 5 6\n7 8 .\n");
}

TEST(RenderBoardTest, AlignsNumbersByWidestValue) {
    const int layout[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
    std::string board = renderBoard(FifteenPuzzle(4, layout));
    EXPECT_TRUE(contains(board, " 1  2  3  4\n"));
    EXPECT_TRUE(contains(board, "13 14 15  .\n"));
}

TEST(FifteenMenuTest, ShowsBoardMoveCounterAndItems) {
    std::string output = runMenu(kExit);
    EXPECT_TRUE(contains(output, "Ходов: 0"));
    EXPECT_TRUE(contains(output, "1. Новая игра"));
    EXPECT_TRUE(contains(output, "0. Выход"));
}

TEST(FifteenMenuTest, StartsNewGameWithChosenOrDefaultSize) {
    EXPECT_FALSE(contains(runMenu(kNewGame + "3\n" + kExit), "Ошибка"));
    EXPECT_FALSE(contains(runMenu(kNewGame + "\n" + kExit), "Ошибка"));
}

TEST(FifteenMenuTest, RejectsBadBoardSize) {
    EXPECT_TRUE(contains(runMenu(kNewGame + "1\n" + kExit), "Ошибка"));
    EXPECT_TRUE(contains(runMenu(kNewGame + "abc\n" + kExit), "Ошибка"));
}

TEST(FifteenMenuTest, PlaysToVictoryFromEnteredLayout) {
    std::string output = runMenu(kLoadLayout + kNearSolvedLayout + kMakeMove + "8\n" + kCheckSolved + kExit);
    EXPECT_TRUE(contains(output, "Поздравляем"));
    EXPECT_TRUE(contains(output, "Ходов: 1"));
    EXPECT_TRUE(contains(output, "Расстановка правильная"));
}

TEST(FifteenMenuTest, ReportsUnsolvedPuzzle) {
    std::string output = runMenu(kLoadLayout + kNearSolvedLayout + kCheckSolved + kExit);
    EXPECT_TRUE(contains(output, "Расстановка неправильная"));
}

TEST(FifteenMenuTest, RejectsImpossibleMove) {
    std::string output = runMenu(kLoadLayout + kNearSolvedLayout + kMakeMove + "1\n" + kExit);
    EXPECT_TRUE(contains(output, "нельзя"));
    EXPECT_TRUE(contains(output, "Ходов: 0"));
}

TEST(FifteenMenuTest, MoveWithoutVictoryDoesNotCongratulate) {
    std::string output = runMenu(kLoadLayout + kNearSolvedLayout + kMakeMove + "5\n" + kExit);
    EXPECT_FALSE(contains(output, "Поздравляем"));
    EXPECT_TRUE(contains(output, "Ходов: 1"));
}

TEST(FifteenMenuTest, RejectsNonNumericMove) {
    EXPECT_TRUE(contains(runMenu(kMakeMove + "abc\n" + kExit), "Ошибка"));
}

TEST(FifteenMenuTest, RejectsBadLayoutAndKeepsRunning) {
    std::string unsolvable = runMenu(kLoadLayout + "3 1 2 3 4 5 6 8 7 0\n" + kExit);
    std::string garbage = runMenu(kLoadLayout + "abc\n" + kCheckSolved + kExit);
    EXPECT_TRUE(contains(unsolvable, "Ошибка"));
    EXPECT_TRUE(contains(garbage, "Ошибка"));
    EXPECT_TRUE(contains(garbage, "Расстановка"));
}

TEST(FifteenMenuTest, ReportsWrongMenuInput) {
    std::string output = runMenu("abc\n99\n" + kExit);
    EXPECT_TRUE(contains(output, "Введите номер пункта"));
    EXPECT_TRUE(contains(output, "Нет такого пункта"));
}

TEST(FifteenMenuTest, EndOfInputInsideActionEndsQuietly) {
    EXPECT_FALSE(contains(runMenu(kNewGame), "Ошибка"));
    EXPECT_FALSE(contains(runMenu(kMakeMove), "Ошибка"));
    EXPECT_TRUE(contains(runMenu(kLoadLayout), "Ошибка"));
}
