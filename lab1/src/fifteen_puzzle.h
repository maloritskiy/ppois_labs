/**
 * @file fifteen_puzzle.h
 * @brief Объявление класса FifteenPuzzle - игры "Пятнашки".
 */
#ifndef FIFTEEN_PUZZLE_H
#define FIFTEEN_PUZZLE_H

#include <iostream>

/**
 * @brief Игра "Пятнашки" на квадратном поле.
 *
 * Плитки пронумерованы от 1 до size*size-1, пустая клетка обозначается kEmptyCell.
 * Клетки нумеруются слева направо и сверху вниз начиная с нуля. Игра считается
 * собранной, когда плитки идут по порядку, а пустая клетка стоит последней.
 * Начальная расстановка получается случайными ходами из собранного состояния,
 * поэтому её всегда можно собрать.
 */
class FifteenPuzzle {
public:
    static constexpr int kMinSize = 2;       ///< Наименьшая сторона поля
    static constexpr int kMaxSize = 10;      ///< Наибольшая сторона поля
    static constexpr int kDefaultSize = 4;   ///< Сторона поля по умолчанию
    static constexpr int kEmptyCell = 0;     ///< Значение пустой клетки

    /**
     * @brief Создаёт игру со случайной расстановкой.
     * @param size Сторона поля от kMinSize до kMaxSize.
     * @throws std::invalid_argument Если размер вне допустимых границ.
     */
    explicit FifteenPuzzle(int size = kDefaultSize);

    /**
     * @brief Создаёт игру с заданной расстановкой.
     * @param size Сторона поля.
     * @param cells Массив из size*size чисел по строкам, каждое число от 0 до size*size-1 ровно один раз.
     * @throws std::invalid_argument Если размер недопустим, числа повторяются или выходят за границы,
     *         либо расстановку нельзя собрать.
     */
    FifteenPuzzle(int size, const int* cells);

    /// @brief Проверяет допустимость размера поля.
    /// @param size Сторона поля.
    /// @return true, если размер от kMinSize до kMaxSize.
    static bool isValidSize(int size);

    /**
     * @brief Проверяет, можно ли собрать расстановку.
     * @param size Сторона поля.
     * @param cells Массив из size*size чисел, которые должны быть перестановкой чисел от 0 до size*size-1.
     * @return true, если расстановку можно привести к собранной.
     */
    static bool isSolvable(int size, const int* cells);

    /// @brief Возвращает сторону поля.
    /// @return Количество клеток в строке.
    int getSize() const;

    /**
     * @brief Возвращает значение клетки.
     * @param index Номер клетки от 0 до size*size-1.
     * @return Номер плитки или kEmptyCell.
     * @throws std::out_of_range Если номер вне поля.
     */
    int operator[](int index) const;

    /**
     * @brief Сдвигает плитку на место пустой клетки.
     * @param tile Номер плитки.
     * @return true, если плитка стояла рядом с пустой клеткой и сдвинута, иначе false.
     */
    bool moveTile(int tile);

    /// @brief Проверяет правильность расстановки.
    /// @return true, если плитки идут по порядку, а пустая клетка последняя.
    bool isSolved() const;

    /// @brief Сравнивает игры на равенство.
    /// @param other Вторая игра.
    /// @return true, если размер и расстановка совпадают.
    bool operator==(const FifteenPuzzle& other) const;

    /// @brief Сравнивает игры на неравенство.
    /// @param other Вторая игра.
    /// @return true, если игры различаются.
    bool operator!=(const FifteenPuzzle& other) const;

private:
    int size_;                        ///< Сторона поля
    int cells_[kMaxSize * kMaxSize];  ///< Значения клеток по строкам
    int emptyIndex_;                  ///< Номер пустой клетки

    void fillSolved();
    void shuffle();
    bool randomStep();
    void swapWithEmpty(int index);
    bool areNeighbours(int first, int second) const;
};

/// @brief Записывает игру в поток: в первой строке размер, дальше строки поля.
/// @param out Выходной поток.
/// @param puzzle Записываемая игра.
/// @return Выходной поток.
std::ostream& operator<<(std::ostream& out, const FifteenPuzzle& puzzle);

/// @brief Читает игру из потока: размер, затем size*size чисел.
/// @param in Входной поток.
/// @param puzzle Игра, в которую записывается результат.
/// @return Входной поток; при неверных данных выставляется failbit, а игра не меняется.
std::istream& operator>>(std::istream& in, FifteenPuzzle& puzzle);

#endif
