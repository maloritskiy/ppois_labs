#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <vector>

#include "fifteen/fifteen_puzzle.hpp"

using lab1::FifteenPuzzle;
using lab1::Position;

namespace {

const std::vector<int> kSolvedThree = {1, 2, 3, 4, 5, 6, 7, 8, 0};
const std::vector<int> kNearSolvedThree = {1, 2, 3, 4, 5, 6, 7, 0, 8};

/// Значение клетки; обёртка нужна, чтобы запятая в Position{...} не ломала аргументы макросов gtest.
int cell(const FifteenPuzzle& puzzle, std::size_t row, std::size_t column) {
    return puzzle[Position{row, column}];
}

std::vector<int> cellsOf(const FifteenPuzzle& puzzle) {
    std::vector<int> cells;
    for (std::size_t row = 0; row < puzzle.size(); ++row) {
        for (std::size_t column = 0; column < puzzle.size(); ++column) {
            cells.push_back(puzzle[Position{row, column}]);
        }
    }
    return cells;
}

}  // namespace

TEST(FifteenPuzzleTest, DefaultPuzzleIsFourByFour) {
    const FifteenPuzzle puzzle;
    EXPECT_EQ(puzzle.size(), FifteenPuzzle::kDefaultSize);
    EXPECT_FALSE(puzzle.isSolved());
}

TEST(FifteenPuzzleTest, RandomPuzzlesAreSolvableAndShuffled) {
    for (std::size_t size = FifteenPuzzle::kMinSize; size <= 6; ++size) {
        for (std::uint32_t seed = 0; seed < 10; ++seed) {
            const FifteenPuzzle puzzle(size, seed);
            EXPECT_FALSE(puzzle.isSolved()) << "size " << size << " seed " << seed;
            EXPECT_TRUE(FifteenPuzzle::isSolvable(size, cellsOf(puzzle))) << "size " << size << " seed " << seed;
        }
    }
}

TEST(FifteenPuzzleTest, RandomPuzzleContainsEveryNumberOnce) {
    const FifteenPuzzle puzzle(4, 7U);
    std::vector<int> cells = cellsOf(puzzle);
    std::vector<bool> seen(cells.size(), false);
    for (const int value : cells) {
        ASSERT_GE(value, 0);
        ASSERT_LT(static_cast<std::size_t>(value), cells.size());
        EXPECT_FALSE(seen[static_cast<std::size_t>(value)]);
        seen[static_cast<std::size_t>(value)] = true;
    }
}

TEST(FifteenPuzzleTest, SameSeedGivesSamePuzzle) {
    EXPECT_EQ(FifteenPuzzle(4, 42U), FifteenPuzzle(4, 42U));
    EXPECT_NE(FifteenPuzzle(4, 1U), FifteenPuzzle(4, 2U));
}

TEST(FifteenPuzzleTest, RandomDeviceConstructorWorks) {
    const FifteenPuzzle puzzle(3);
    EXPECT_EQ(puzzle.size(), 3U);
}

TEST(FifteenPuzzleTest, InvalidSizeIsRejected) {
    EXPECT_THROW(FifteenPuzzle(1), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(FifteenPuzzle::kMaxSize + 1), std::invalid_argument);
    EXPECT_TRUE(FifteenPuzzle::isValidSize(FifteenPuzzle::kMinSize));
    EXPECT_FALSE(FifteenPuzzle::isValidSize(0));
}

TEST(FifteenPuzzleTest, ExplicitLayoutIsValidated) {
    EXPECT_NO_THROW(FifteenPuzzle(3, kSolvedThree));
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 1, 3, 4, 5, 6, 7, 8, 0}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9}), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, -1}), std::invalid_argument);
}

TEST(FifteenPuzzleTest, UnsolvableLayoutIsRejected) {
    EXPECT_THROW(FifteenPuzzle(3, std::vector<int>{1, 2, 3, 4, 5, 6, 8, 7, 0}), std::invalid_argument);
}

TEST(FifteenPuzzleTest, SolvabilityOfOddAndEvenBoards) {
    EXPECT_TRUE(FifteenPuzzle::isSolvable(3, kSolvedThree));
    EXPECT_FALSE(FifteenPuzzle::isSolvable(3, std::vector<int>{1, 2, 3, 4, 5, 6, 8, 7, 0}));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(2, std::vector<int>{1, 2, 3, 0}));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(2, std::vector<int>{1, 2, 0, 3}));
    EXPECT_FALSE(FifteenPuzzle::isSolvable(2, std::vector<int>{2, 1, 3, 0}));
    const std::vector<int> classicUnsolvable = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 14, 0};
    const std::vector<int> oneMoveFromSolved = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15};
    EXPECT_FALSE(FifteenPuzzle::isSolvable(4, classicUnsolvable));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(4, oneMoveFromSolved));
}

TEST(FifteenPuzzleTest, CellAccessReturnsLayoutValues) {
    const FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_EQ(cell(puzzle, 0, 0), 1);
    EXPECT_EQ(cell(puzzle, 2, 0), 7);
    EXPECT_EQ(cell(puzzle, 2, 1), FifteenPuzzle::kEmptyCell);
    EXPECT_EQ(puzzle.emptyPosition(), (Position{2, 1}));
}

TEST(FifteenPuzzleTest, CellAccessOutsideBoardThrows) {
    const FifteenPuzzle puzzle(3, kSolvedThree);
    EXPECT_THROW(cell(puzzle, 3, 0), std::out_of_range);
    EXPECT_THROW(cell(puzzle, 0, 3), std::out_of_range);
}

TEST(FifteenPuzzleTest, MovingNeighbourTileSwapsItWithEmptyCell) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_TRUE(puzzle.canMove(8));
    EXPECT_TRUE(puzzle.moveTile(8));
    EXPECT_TRUE(puzzle.isSolved());
    EXPECT_EQ(puzzle.emptyPosition(), (Position{2, 2}));
}

TEST(FifteenPuzzleTest, MovingVerticalNeighbourWorks) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_TRUE(puzzle.moveTile(5));
    EXPECT_EQ(cell(puzzle, 2, 1), 5);
    EXPECT_EQ(puzzle.emptyPosition(), (Position{1, 1}));
}

TEST(FifteenPuzzleTest, ImpossibleMovesAreRejected) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    const FifteenPuzzle before = puzzle;
    EXPECT_FALSE(puzzle.moveTile(1));
    EXPECT_FALSE(puzzle.moveTile(FifteenPuzzle::kEmptyCell));
    EXPECT_FALSE(puzzle.moveTile(-3));
    EXPECT_FALSE(puzzle.moveTile(9));
    EXPECT_FALSE(puzzle.canMove(100));
    EXPECT_EQ(puzzle, before);
}

TEST(FifteenPuzzleTest, EmptyCellInTheCornerHasTwoNeighbours) {
    FifteenPuzzle puzzle(3, kSolvedThree);
    EXPECT_TRUE(puzzle.canMove(6));
    EXPECT_TRUE(puzzle.canMove(8));
    EXPECT_FALSE(puzzle.canMove(5));
    EXPECT_FALSE(puzzle.canMove(1));
}

TEST(FifteenPuzzleTest, EmptyCellInTheTopRowCanMoveAllWays) {
    const std::vector<int> layout = {1, 0, 3, 4, 2, 6, 7, 5, 8};
    FifteenPuzzle puzzle(3, layout);
    EXPECT_TRUE(puzzle.canMove(1));
    EXPECT_TRUE(puzzle.canMove(3));
    EXPECT_TRUE(puzzle.canMove(2));
    EXPECT_FALSE(puzzle.canMove(4));
}

TEST(FifteenPuzzleTest, CopyIsIndependentOfOriginal) {
    FifteenPuzzle original(3, kNearSolvedThree);
    FifteenPuzzle copy(original);
    FifteenPuzzle assigned(3, kSolvedThree);
    assigned = original;
    original.moveTile(8);
    EXPECT_NE(copy, original);
    EXPECT_FALSE(copy.isSolved());
    EXPECT_EQ(assigned, copy);
}

TEST(FifteenPuzzleTest, EqualityComparesSizeAndCells) {
    EXPECT_EQ(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(3, kSolvedThree));
    EXPECT_NE(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(3, kNearSolvedThree));
    EXPECT_NE(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(2, std::vector<int>{1, 2, 3, 0}));
}

TEST(FifteenPuzzleStreamTest, OutputFormatIsSizeThenRows) {
    std::ostringstream out;
    out << FifteenPuzzle(3, kNearSolvedThree);
    EXPECT_EQ(out.str(), "3\n1 2 3\n4 5 6\n7 0 8\n");
}

TEST(FifteenPuzzleStreamTest, InputRoundTrips) {
    const FifteenPuzzle original(4, 5U);
    std::stringstream stream;
    stream << original;
    FifteenPuzzle loaded(3, kSolvedThree);
    stream >> loaded;
    EXPECT_TRUE(stream);
    EXPECT_EQ(loaded, original);
}

TEST(FifteenPuzzleStreamTest, InputFailsOnBadData) {
    const std::vector<std::string> bad = {"",
                                          "abc",
                                          "1\n0\n",
                                          "99\n",
                                          "3\n1 2 3\n",
                                          "3\n1 2 3 4 5 6 7 8 x\n",
                                          "3\n1 1 3 4 5 6 7 8 0\n",
                                          "3\n1 2 3 4 5 6 8 7 0\n"};
    for (const std::string& text : bad) {
        SCOPED_TRACE(text);
        std::istringstream in(text);
        FifteenPuzzle puzzle(3, kSolvedThree);
        in >> puzzle;
        EXPECT_TRUE(in.fail());
    }
}
