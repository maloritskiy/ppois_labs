#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "fifteen_puzzle.h"

namespace {

const int kSolvedThree[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
const int kNearSolvedThree[] = {1, 2, 3, 4, 5, 6, 7, 0, 8};
const int kRandomPuzzlesPerSize = 20;
const int kMaxCells = FifteenPuzzle::kMaxSize * FifteenPuzzle::kMaxSize;

void copyCells(const FifteenPuzzle& puzzle, int* cells) {
    for (int i = 0; i < puzzle.getSize() * puzzle.getSize(); ++i) {
        cells[i] = puzzle[i];
    }
}

}  // namespace

TEST(FifteenPuzzleTest, DefaultPuzzleIsFourByFour) {
    FifteenPuzzle puzzle;
    EXPECT_EQ(puzzle.getSize(), FifteenPuzzle::kDefaultSize);
    EXPECT_FALSE(puzzle.isSolved());
}

TEST(FifteenPuzzleTest, RandomPuzzlesAreSolvableAndShuffled) {
    for (int size = FifteenPuzzle::kMinSize; size <= 6; ++size) {
        for (int attempt = 0; attempt < kRandomPuzzlesPerSize; ++attempt) {
            FifteenPuzzle puzzle(size);
            int cells[kMaxCells];
            copyCells(puzzle, cells);
            EXPECT_FALSE(puzzle.isSolved()) << "size " << size;
            EXPECT_TRUE(FifteenPuzzle::isSolvable(size, cells)) << "size " << size;
        }
    }
}

TEST(FifteenPuzzleTest, RandomPuzzleContainsEveryNumberOnce) {
    FifteenPuzzle puzzle(4);
    int counts[kMaxCells] = {0};
    for (int i = 0; i < 16; ++i) {
        ++counts[puzzle[i]];
    }
    for (int value = 0; value < 16; ++value) {
        EXPECT_EQ(counts[value], 1);
    }
}

TEST(FifteenPuzzleTest, InvalidSizeIsRejected) {
    EXPECT_THROW(FifteenPuzzle(1), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(FifteenPuzzle::kMaxSize + 1), std::invalid_argument);
    EXPECT_TRUE(FifteenPuzzle::isValidSize(FifteenPuzzle::kMinSize));
    EXPECT_FALSE(FifteenPuzzle::isValidSize(0));
}

TEST(FifteenPuzzleTest, ExplicitLayoutIsValidated) {
    const int duplicate[] = {1, 1, 3, 4, 5, 6, 7, 8, 0};
    const int tooBig[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    const int negative[] = {1, 2, 3, 4, 5, 6, 7, 8, -1};
    EXPECT_NO_THROW(FifteenPuzzle(3, kSolvedThree));
    EXPECT_THROW(FifteenPuzzle(3, duplicate), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, tooBig), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(3, negative), std::invalid_argument);
    EXPECT_THROW(FifteenPuzzle(1, kSolvedThree), std::invalid_argument);
}

TEST(FifteenPuzzleTest, UnsolvableLayoutIsRejected) {
    const int unsolvable[] = {1, 2, 3, 4, 5, 6, 8, 7, 0};
    EXPECT_THROW(FifteenPuzzle(3, unsolvable), std::invalid_argument);
}

TEST(FifteenPuzzleTest, SolvabilityOfOddAndEvenBoards) {
    const int oddUnsolvable[] = {1, 2, 3, 4, 5, 6, 8, 7, 0};
    const int smallSolved[] = {1, 2, 3, 0};
    const int smallSolvable[] = {1, 2, 0, 3};
    const int smallUnsolvable[] = {2, 1, 3, 0};
    const int classicUnsolvable[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 14, 0};
    const int oneMoveFromSolved[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0, 15};
    EXPECT_TRUE(FifteenPuzzle::isSolvable(3, kSolvedThree));
    EXPECT_FALSE(FifteenPuzzle::isSolvable(3, oddUnsolvable));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(2, smallSolved));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(2, smallSolvable));
    EXPECT_FALSE(FifteenPuzzle::isSolvable(2, smallUnsolvable));
    EXPECT_FALSE(FifteenPuzzle::isSolvable(4, classicUnsolvable));
    EXPECT_TRUE(FifteenPuzzle::isSolvable(4, oneMoveFromSolved));
}

TEST(FifteenPuzzleTest, CellAccessReturnsLayoutValues) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_EQ(puzzle[0], 1);
    EXPECT_EQ(puzzle[6], 7);
    EXPECT_EQ(puzzle[7], FifteenPuzzle::kEmptyCell);
}

TEST(FifteenPuzzleTest, CellAccessOutsideBoardThrows) {
    FifteenPuzzle puzzle(3, kSolvedThree);
    EXPECT_THROW(puzzle[-1], std::out_of_range);
    EXPECT_THROW(puzzle[9], std::out_of_range);
}

TEST(FifteenPuzzleTest, MovingNeighbourTileSwapsItWithEmptyCell) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_TRUE(puzzle.moveTile(8));
    EXPECT_TRUE(puzzle.isSolved());
}

TEST(FifteenPuzzleTest, MovingTileAboveEmptyCellWorks) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    EXPECT_TRUE(puzzle.moveTile(5));
    EXPECT_EQ(puzzle[7], 5);
    EXPECT_EQ(puzzle[4], FifteenPuzzle::kEmptyCell);
}

TEST(FifteenPuzzleTest, ImpossibleMovesAreRejected) {
    FifteenPuzzle puzzle(3, kNearSolvedThree);
    FifteenPuzzle before = puzzle;
    EXPECT_FALSE(puzzle.moveTile(1));
    EXPECT_FALSE(puzzle.moveTile(FifteenPuzzle::kEmptyCell));
    EXPECT_FALSE(puzzle.moveTile(-3));
    EXPECT_FALSE(puzzle.moveTile(9));
    EXPECT_EQ(puzzle, before);
}

TEST(FifteenPuzzleTest, TilesOnTheNextRowAreNotNeighbours) {
    const int layout[] = {1, 2, 3, 4, 5, 0, 7, 8, 6};
    FifteenPuzzle puzzle(3, layout);
    EXPECT_FALSE(puzzle.moveTile(7));
    EXPECT_TRUE(puzzle.moveTile(6));
}

TEST(FifteenPuzzleTest, EmptyCellInTheMiddleHasFourNeighbours) {
    const int layout[] = {1, 2, 3, 4, 0, 6, 7, 5, 8};
    FifteenPuzzle puzzle(3, layout);
    EXPECT_TRUE(puzzle.moveTile(2));
    EXPECT_TRUE(puzzle.moveTile(1));
    EXPECT_TRUE(puzzle.moveTile(4));
    EXPECT_FALSE(puzzle.moveTile(8));
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
    const int smallSolved[] = {1, 2, 3, 0};
    EXPECT_EQ(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(3, kSolvedThree));
    EXPECT_NE(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(3, kNearSolvedThree));
    EXPECT_NE(FifteenPuzzle(3, kSolvedThree), FifteenPuzzle(2, smallSolved));
}

TEST(FifteenPuzzleStreamTest, OutputFormatIsSizeThenRows) {
    std::ostringstream out;
    out << FifteenPuzzle(3, kNearSolvedThree);
    EXPECT_EQ(out.str(), "3\n1 2 3\n4 5 6\n7 0 8\n");
}

TEST(FifteenPuzzleStreamTest, InputRoundTrips) {
    FifteenPuzzle original(4);
    std::stringstream stream;
    stream << original;
    FifteenPuzzle loaded(3, kSolvedThree);
    stream >> loaded;
    EXPECT_FALSE(stream.fail());
    EXPECT_EQ(loaded, original);
}

TEST(FifteenPuzzleStreamTest, InputFailsOnBadData) {
    const char* const bad[] = {"",
                               "abc",
                               "99\n",
                               "3\n1 2 3\n",
                               "3\n1 2 3 4 5 6 7 8 x\n",
                               "3\n1 1 3 4 5 6 7 8 0\n",
                               "3\n1 2 3 4 5 6 8 7 0\n"};
    for (const char* text : bad) {
        SCOPED_TRACE(text);
        std::istringstream in(text);
        FifteenPuzzle puzzle(3, kSolvedThree);
        in >> puzzle;
        EXPECT_TRUE(in.fail());
    }
}
