#include <gtest/gtest.h>

#include <string>

#include "fifteen/fifteen_puzzle.hpp"
#include "test_support.hpp"
#include "ui/fifteen_cli.hpp"

using lab1::FifteenPuzzle;
using lab1::testing_support::contains;
using lab1::testing_support::runCli;
using lab1::testing_support::TempFile;
using lab1::ui::FifteenCli;

namespace {

// Номера пунктов меню FifteenCli.
const std::string kNewGame = "1\n";
const std::string kMove = "2\n";
const std::string kCheck = "3\n";
const std::string kSave = "4\n";
const std::string kLoad = "5\n";
const std::string kExit = "0\n";

const std::string kNearSolvedText = "3\n1 2 3\n4 5 6\n7 0 8\n";
const std::string kSolvedText = "3\n1 2 3\n4 5 6\n7 8 0\n";

std::string run(const std::string& script) { return runCli<FifteenCli>(script + kExit); }

}  // namespace

TEST(RenderBoardTest, DrawsEmptyCellAsDot) {
    const FifteenPuzzle puzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 0});
    EXPECT_EQ(lab1::ui::renderBoard(puzzle), "1 2 3\n4 5 6\n7 8 .\n");
}

TEST(RenderBoardTest, AlignsNumbersByWidestValue) {
    const std::vector<int> layout = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};
    const std::string board = lab1::ui::renderBoard(FifteenPuzzle(4, layout));
    EXPECT_TRUE(contains(board, " 1  2  3  4\n"));
    EXPECT_TRUE(contains(board, "13 14 15  .\n"));
}

TEST(FifteenCliTest, ShowsBoardAndMoveCounterOnStart) {
    const std::string output = run("");
    EXPECT_TRUE(contains(output, "Ходов: 0"));
    EXPECT_TRUE(contains(output, "1. Новая игра"));
}

TEST(FifteenCliTest, StartsNewGameWithChosenSize) {
    const std::string output = run(kNewGame + "3\n");
    EXPECT_TRUE(contains(output, "Размер поля"));
    EXPECT_FALSE(contains(output, "Ошибка"));
}

TEST(FifteenCliTest, NewGameWithEmptyAnswerUsesDefaultSize) {
    const std::string output = run(kNewGame + "\n");
    EXPECT_FALSE(contains(output, "Ошибка"));
}

TEST(FifteenCliTest, RejectsBadBoardSize) {
    EXPECT_TRUE(contains(run(kNewGame + "1\n"), "Ошибка"));
    EXPECT_TRUE(contains(run(kNewGame + "abc\n"), "Ошибка"));
    EXPECT_TRUE(contains(run(kNewGame + "-4\n"), "Ошибка"));
}

TEST(FifteenCliTest, PlaysToVictoryFromLoadedGame) {
    TempFile file("play_to_victory");
    file.write(kNearSolvedText);
    const std::string output = run(kLoad + file.path() + "\n" + kMove + "8\n" + kCheck);
    EXPECT_TRUE(contains(output, "Игра загружена"));
    EXPECT_TRUE(contains(output, "Поздравляем! Головоломка собрана"));
    EXPECT_TRUE(contains(output, "Ходов: 1"));
    EXPECT_TRUE(contains(output, "Расстановка правильная"));
}

TEST(FifteenCliTest, ReportsUnsolvedPuzzle) {
    TempFile file("unsolved_check");
    file.write(kNearSolvedText);
    const std::string output = run(kLoad + file.path() + "\n" + kCheck);
    EXPECT_TRUE(contains(output, "Расстановка неправильная"));
}

TEST(FifteenCliTest, RejectsImpossibleMove) {
    TempFile file("impossible_move");
    file.write(kNearSolvedText);
    const std::string output = run(kLoad + file.path() + "\n" + kMove + "1\n");
    EXPECT_TRUE(contains(output, "нельзя"));
    EXPECT_TRUE(contains(output, "Ходов: 0"));
}

TEST(FifteenCliTest, RejectsNonNumericMove) {
    EXPECT_TRUE(contains(run(kMove + "abc\n"), "Ошибка"));
}

TEST(FifteenCliTest, SavesGameToFile) {
    TempFile source("save_source");
    TempFile target("save_target");
    source.write(kSolvedText);
    const std::string output = run(kLoad + source.path() + "\n" + kSave + target.path() + "\n");
    EXPECT_TRUE(contains(output, "Игра сохранена"));
    EXPECT_EQ(target.read(), kSolvedText);
}

TEST(FifteenCliTest, ReportsFileProblems) {
    TempFile broken("broken_file");
    broken.write("3\n1 1 1\n");
    const std::string missingPath = "/nonexistent_dir_for_lab1/game.txt";
    EXPECT_TRUE(contains(run(kLoad + missingPath + "\n"), "Ошибка"));
    EXPECT_TRUE(contains(run(kSave + missingPath + "\n"), "Ошибка"));
    EXPECT_TRUE(contains(run(kLoad + broken.path() + "\n"), "Ошибка"));
}

TEST(FifteenCliTest, EndOfInputInsideActionsEndsQuietly) {
    for (const std::string& action : {kNewGame, kMove, kSave, kLoad}) {
        SCOPED_TRACE(action);
        const std::string output = runCli<FifteenCli>(action);
        EXPECT_FALSE(contains(output, "Ошибка"));
    }
}
