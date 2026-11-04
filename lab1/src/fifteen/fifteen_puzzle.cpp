#include "fifteen/fifteen_puzzle.hpp"

#include <algorithm>
#include <istream>
#include <numeric>
#include <ostream>
#include <stdexcept>

namespace lab1 {

namespace {

constexpr std::size_t kShuffleMovesPerCell = 50;

bool isOdd(std::size_t value) { return value % 2 == 1; }

std::size_t countInversions(const std::vector<int>& cells) {
    std::size_t inversions = 0;
    for (std::size_t first = 0; first < cells.size(); ++first) {
        for (std::size_t second = first + 1; second < cells.size(); ++second) {
            if (cells[first] > cells[second] && cells[second] != FifteenPuzzle::kEmptyCell) {
                ++inversions;
            }
        }
    }
    return inversions;
}

void writeRow(std::ostream& out, const FifteenPuzzle& puzzle, std::size_t row) {
    for (std::size_t column = 0; column < puzzle.size(); ++column) {
        if (column > 0) {
            out << ' ';
        }
        out << puzzle[Position{row, column}];
    }
    out << '\n';
}

}  // namespace

FifteenPuzzle::FifteenPuzzle(std::size_t size) : FifteenPuzzle(size, std::random_device{}()) {}

FifteenPuzzle::FifteenPuzzle(std::size_t size, std::uint32_t seed)
    : size_(requireValidSize(size)), cells_(solvedCells(size_)), emptyIndex_(size_ * size_ - 1) {
    std::mt19937 generator(seed);
    shuffle(generator);
}

FifteenPuzzle::FifteenPuzzle(std::size_t size, const std::vector<int>& cells)
    : size_(requireValidSize(size)), cells_(cells), emptyIndex_(0) {
    requirePermutation();
    if (!isSolvable(size_, cells_)) {
        throw std::invalid_argument("The layout cannot be solved");
    }
    emptyIndex_ = findTile(kEmptyCell);
}

bool FifteenPuzzle::isValidSize(std::size_t size) { return size >= kMinSize && size <= kMaxSize; }

bool FifteenPuzzle::isSolvable(std::size_t size, const std::vector<int>& cells) {
    const std::size_t inversions = countInversions(cells);
    if (isOdd(size)) {
        return !isOdd(inversions);
    }
    const auto blank = std::find(cells.begin(), cells.end(), kEmptyCell);
    const std::size_t blankRow = static_cast<std::size_t>(blank - cells.begin()) / size;
    return isOdd(inversions + blankRow);
}

std::size_t FifteenPuzzle::size() const { return size_; }

int FifteenPuzzle::operator[](const Position& position) const {
    if (position.row >= size_ || position.column >= size_) {
        throw std::out_of_range("Cell position is outside the board");
    }
    return cells_[position.row * size_ + position.column];
}

Position FifteenPuzzle::emptyPosition() const { return Position{emptyIndex_ / size_, emptyIndex_ % size_}; }

bool FifteenPuzzle::canMove(int tile) const {
    const bool isTile = tile > kEmptyCell && static_cast<std::size_t>(tile) < cells_.size();
    if (!isTile) {
        return false;
    }
    const std::vector<std::size_t> adjacent = neighbours(emptyIndex_);
    return std::find(adjacent.begin(), adjacent.end(), findTile(tile)) != adjacent.end();
}

bool FifteenPuzzle::moveTile(int tile) {
    if (!canMove(tile)) {
        return false;
    }
    swapWithEmpty(findTile(tile));
    return true;
}

bool FifteenPuzzle::isSolved() const { return cells_ == solvedCells(size_); }

bool FifteenPuzzle::operator==(const FifteenPuzzle& other) const {
    return size_ == other.size_ && cells_ == other.cells_;
}

bool FifteenPuzzle::operator!=(const FifteenPuzzle& other) const { return !(*this == other); }

std::size_t FifteenPuzzle::requireValidSize(std::size_t size) {
    if (!isValidSize(size)) {
        throw std::invalid_argument("Board size must be between " + std::to_string(kMinSize) + " and " +
                                    std::to_string(kMaxSize));
    }
    return size;
}

std::vector<int> FifteenPuzzle::solvedCells(std::size_t size) {
    std::vector<int> cells(size * size);
    std::iota(cells.begin(), cells.end() - 1, 1);
    cells.back() = kEmptyCell;
    return cells;
}

void FifteenPuzzle::requirePermutation() const {
    if (cells_.size() != size_ * size_) {
        throw std::invalid_argument("Wrong number of cells");
    }
    std::vector<bool> seen(cells_.size(), false);
    for (const int value : cells_) {
        const bool inRange = value >= 0 && static_cast<std::size_t>(value) < cells_.size();
        if (!inRange || seen[static_cast<std::size_t>(value)]) {
            throw std::invalid_argument("Cells must contain every number from 0 to size*size-1 exactly once");
        }
        seen[static_cast<std::size_t>(value)] = true;
    }
}

void FifteenPuzzle::shuffle(std::mt19937& generator) {
    const std::size_t moveCount = cells_.size() * kShuffleMovesPerCell;
    std::size_t previousEmpty = emptyIndex_;
    for (std::size_t step = 0; step < moveCount; ++step) {
        randomStep(generator, previousEmpty);
    }
    while (isSolved()) {
        randomStep(generator, previousEmpty);
    }
}

void FifteenPuzzle::randomStep(std::mt19937& generator, std::size_t& previousEmpty) {
    std::vector<std::size_t> options = neighbours(emptyIndex_);
    options.erase(std::remove(options.begin(), options.end(), previousEmpty), options.end());
    std::uniform_int_distribution<std::size_t> pick(0, options.size() - 1);
    const std::size_t target = options[pick(generator)];
    previousEmpty = emptyIndex_;
    swapWithEmpty(target);
}

void FifteenPuzzle::swapWithEmpty(std::size_t index) {
    std::swap(cells_[index], cells_[emptyIndex_]);
    emptyIndex_ = index;
}

std::size_t FifteenPuzzle::findTile(int tile) const {
    const auto found = std::find(cells_.begin(), cells_.end(), tile);
    return static_cast<std::size_t>(found - cells_.begin());
}

std::vector<std::size_t> FifteenPuzzle::neighbours(std::size_t index) const {
    const std::size_t row = index / size_;
    const std::size_t column = index % size_;
    std::vector<std::size_t> result;
    if (row > 0) {
        result.push_back(index - size_);
    }
    if (row + 1 < size_) {
        result.push_back(index + size_);
    }
    if (column > 0) {
        result.push_back(index - 1);
    }
    if (column + 1 < size_) {
        result.push_back(index + 1);
    }
    return result;
}

std::ostream& operator<<(std::ostream& out, const FifteenPuzzle& puzzle) {
    out << puzzle.size() << '\n';
    for (std::size_t row = 0; row < puzzle.size(); ++row) {
        writeRow(out, puzzle, row);
    }
    return out;
}

std::istream& operator>>(std::istream& in, FifteenPuzzle& puzzle) {
    std::size_t size = 0;
    if (!(in >> size) || !FifteenPuzzle::isValidSize(size)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    std::vector<int> cells(size * size);
    for (int& cell : cells) {
        in >> cell;
    }
    if (!in) {
        return in;
    }
    try {
        puzzle = FifteenPuzzle(size, cells);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}

}  // namespace lab1
