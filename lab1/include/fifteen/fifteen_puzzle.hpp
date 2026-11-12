#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <random>
#include <vector>

namespace lab1 {

/// Позиция клетки на поле (нумерация с нуля).
struct Position {
    std::size_t row = 0;
    std::size_t column = 0;

    bool operator==(const Position& other) const { return row == other.row && column == other.column; }
};

/// @brief Игра «Пятнашки» на квадратном поле.
///
/// Плитки пронумерованы от 1 до size*size-1, пустая клетка обозначается значением kEmptyCell.
/// Начальное размещение случайное, но всегда решаемое: оно получается серией случайных
/// допустимых ходов из собранного состояния.
class FifteenPuzzle {
public:
    /// Размер поля по умолчанию.
    static constexpr std::size_t kDefaultSize = 4;
    /// Минимальный размер поля.
    static constexpr std::size_t kMinSize = 2;
    /// Максимальный размер поля.
    static constexpr std::size_t kMaxSize = 10;
    /// Значение пустой клетки.
    static constexpr int kEmptyCell = 0;

    /// Создаёт игру со случайным размещением. @throw std::invalid_argument при недопустимом размере.
    explicit FifteenPuzzle(std::size_t size = kDefaultSize);
    /// Создаёт игру со случайным размещением, определяемым зерном генератора.
    FifteenPuzzle(std::size_t size, std::uint32_t seed);
    /// Создаёт игру с заданным размещением (по строкам, слева направо).
    /// @throw std::invalid_argument если размещение не является перестановкой или нерешаемо.
    FifteenPuzzle(std::size_t size, const std::vector<int>& cells);

    FifteenPuzzle(const FifteenPuzzle& other) = default;
    FifteenPuzzle(FifteenPuzzle&& other) noexcept = default;
    FifteenPuzzle& operator=(const FifteenPuzzle& other) = default;
    FifteenPuzzle& operator=(FifteenPuzzle&& other) noexcept = default;
    ~FifteenPuzzle() = default;

    /// Проверяет, допустим ли размер поля.
    static bool isValidSize(std::size_t size);
    /// Проверяет, можно ли из размещения прийти к собранному состоянию (размещение должно быть перестановкой).
    static bool isSolvable(std::size_t size, const std::vector<int>& cells);

    /// Сторона поля.
    std::size_t size() const;
    /// Значение клетки. @throw std::out_of_range если позиция вне поля.
    int operator[](const Position& position) const;
    /// Позиция пустой клетки.
    Position emptyPosition() const;

    /// Можно ли сдвинуть плитку (она должна быть рядом с пустой клеткой).
    bool canMove(int tile) const;
    /// Сдвигает плитку на место пустой клетки. @return false, если ход невозможен.
    bool moveTile(int tile);
    /// Проверяет правильность расстановки: плитки по порядку, пустая клетка последняя.
    bool isSolved() const;

    bool operator==(const FifteenPuzzle& other) const;
    bool operator!=(const FifteenPuzzle& other) const;

private:
    static std::size_t requireValidSize(std::size_t size);
    static std::vector<int> solvedCells(std::size_t size);
    void requirePermutation() const;
    void shuffle(std::mt19937& generator);
    void randomStep(std::mt19937& generator, std::size_t& previousEmpty);
    void swapWithEmpty(std::size_t index);
    std::size_t findTile(int tile) const;
    std::vector<std::size_t> neighbours(std::size_t index) const;

    std::size_t size_;
    std::vector<int> cells_;
    std::size_t emptyIndex_;
};

/// Записывает игру в поток: в первой строке размер, далее строки поля.
std::ostream& operator<<(std::ostream& out, const FifteenPuzzle& puzzle);
/// Читает игру из потока в формате operator<<; при ошибке выставляет failbit.
std::istream& operator>>(std::istream& in, FifteenPuzzle& puzzle);

}  // namespace lab1
