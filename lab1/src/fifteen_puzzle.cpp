#include "fifteen_puzzle.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace {

const int kShuffleMovesPerCell = 50;
const int kDirectionCount = 4;
const int kRowShift[kDirectionCount] = {-1, 1, 0, 0};
const int kColumnShift[kDirectionCount] = {0, 0, -1, 1};
const int kNotFound = -1;

bool isOdd(int value) { return value % 2 == 1; }

int checkSize(int size) {
    if (!FifteenPuzzle::isValidSize(size)) {
        throw std::invalid_argument("Размер поля должен быть от " + std::to_string(FifteenPuzzle::kMinSize) + " до " +
                                    std::to_string(FifteenPuzzle::kMaxSize));
    }
    return size;
}

void checkPermutation(int size, const int* cells) {
    int total = size * size;
    bool seen[FifteenPuzzle::kMaxSize * FifteenPuzzle::kMaxSize] = {false};
    for (int i = 0; i < total; ++i) {
        int value = cells[i];
        if (value < 0 || value >= total || seen[value]) {
            throw std::invalid_argument("В расстановке должны быть все числа от 0 до size*size-1 по одному разу");
        }
        seen[value] = true;
    }
}

int findIndex(const int* cells, int total, int value) {
    for (int i = 0; i < total; ++i) {
        if (cells[i] == value) {
            return i;
        }
    }
    return kNotFound;
}

int countInversions(const int* cells, int total) {
    int inversions = 0;
    for (int first = 0; first < total; ++first) {
        for (int second = first + 1; second < total; ++second) {
            bool bothTiles = cells[first] != FifteenPuzzle::kEmptyCell && cells[second] != FifteenPuzzle::kEmptyCell;
            if (bothTiles && cells[first] > cells[second]) {
                ++inversions;
            }
        }
    }
    return inversions;
}

}  // namespace

FifteenPuzzle::FifteenPuzzle(int size) : size_(checkSize(size)), emptyIndex_(0) {
    fillSolved();
    shuffle();
}

FifteenPuzzle::FifteenPuzzle(int size, const int* cells) : size_(checkSize(size)), emptyIndex_(0) {
    checkPermutation(size_, cells);
    if (!isSolvable(size_, cells)) {
        throw std::invalid_argument("Эту расстановку нельзя собрать");
    }
    int total = size_ * size_;
    for (int i = 0; i < total; ++i) {
        cells_[i] = cells[i];
    }
    emptyIndex_ = findIndex(cells_, total, kEmptyCell);
}

bool FifteenPuzzle::isValidSize(int size) { return size >= kMinSize && size <= kMaxSize; }

bool FifteenPuzzle::isSolvable(int size, const int* cells) {
    int total = size * size;
    int inversions = countInversions(cells, total);
    if (isOdd(size)) {
        return !isOdd(inversions);
    }
    int emptyRow = findIndex(cells, total, kEmptyCell) / size;
    return isOdd(inversions + emptyRow);
}

int FifteenPuzzle::getSize() const { return size_; }

int FifteenPuzzle::operator[](int index) const {
    if (index < 0 || index >= size_ * size_) {
        throw std::out_of_range("Клетки с таким номером нет");
    }
    return cells_[index];
}

bool FifteenPuzzle::moveTile(int tile) {
    if (tile == kEmptyCell) {
        return false;
    }
    int tileIndex = findIndex(cells_, size_ * size_, tile);
    if (tileIndex == kNotFound || !areNeighbours(tileIndex, emptyIndex_)) {
        return false;
    }
    swapWithEmpty(tileIndex);
    return true;
}

bool FifteenPuzzle::isSolved() const {
    int total = size_ * size_;
    for (int i = 0; i < total - 1; ++i) {
        if (cells_[i] != i + 1) {
            return false;
        }
    }
    return cells_[total - 1] == kEmptyCell;
}

bool FifteenPuzzle::operator==(const FifteenPuzzle& other) const {
    if (size_ != other.size_) {
        return false;
    }
    for (int i = 0; i < size_ * size_; ++i) {
        if (cells_[i] != other.cells_[i]) {
            return false;
        }
    }
    return true;
}

bool FifteenPuzzle::operator!=(const FifteenPuzzle& other) const { return !(*this == other); }

void FifteenPuzzle::fillSolved() {
    int total = size_ * size_;
    for (int i = 0; i < total - 1; ++i) {
        cells_[i] = i + 1;
    }
    cells_[total - 1] = kEmptyCell;
    emptyIndex_ = total - 1;
}

void FifteenPuzzle::shuffle() {
    int movesLeft = size_ * size_ * kShuffleMovesPerCell;
    while (movesLeft > 0 || isSolved()) {
        if (randomStep()) {
            --movesLeft;
        }
    }
}

bool FifteenPuzzle::randomStep() {
    int direction = std::rand() % kDirectionCount;
    int row = emptyIndex_ / size_ + kRowShift[direction];
    int column = emptyIndex_ % size_ + kColumnShift[direction];
    bool insideBoard = row >= 0 && row < size_ && column >= 0 && column < size_;
    if (!insideBoard) {
        return false;
    }
    swapWithEmpty(row * size_ + column);
    return true;
}

void FifteenPuzzle::swapWithEmpty(int index) {
    cells_[emptyIndex_] = cells_[index];
    cells_[index] = kEmptyCell;
    emptyIndex_ = index;
}

bool FifteenPuzzle::areNeighbours(int first, int second) const {
    int rowDistance = std::abs(first / size_ - second / size_);
    int columnDistance = std::abs(first % size_ - second % size_);
    return rowDistance + columnDistance == 1;
}

std::ostream& operator<<(std::ostream& out, const FifteenPuzzle& puzzle) {
    int size = puzzle.getSize();
    out << size << '\n';
    for (int row = 0; row < size; ++row) {
        for (int column = 0; column < size; ++column) {
            out << (column > 0 ? " " : "") << puzzle[row * size + column];
        }
        out << '\n';
    }
    return out;
}

std::istream& operator>>(std::istream& in, FifteenPuzzle& puzzle) {
    int size = 0;
    if (!(in >> size)) {
        return in;
    }
    if (!FifteenPuzzle::isValidSize(size)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    int cells[FifteenPuzzle::kMaxSize * FifteenPuzzle::kMaxSize];
    for (int i = 0; i < size * size; ++i) {
        if (!(in >> cells[i])) {
            return in;
        }
    }
    try {
        puzzle = FifteenPuzzle(size, cells);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}
