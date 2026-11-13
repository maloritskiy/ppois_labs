/**
 * @file fifteen_menu.h
 * @brief Объявление консольного меню игры "Пятнашки".
 */
#ifndef FIFTEEN_MENU_H
#define FIFTEEN_MENU_H

#include <iostream>
#include <string>

#include "fifteen_puzzle.h"

/**
 * @brief Рисует поле в виде текста.
 * @param puzzle Игра.
 * @return Строки поля, числа выровнены по правому краю, пустая клетка показана точкой.
 */
std::string renderBoard(const FifteenPuzzle& puzzle);

/**
 * @brief Консольное меню игры "Пятнашки".
 *
 * Меню зависит от класса FifteenPuzzle, но сам класс о меню ничего не знает.
 * Потоки ввода и вывода передаются снаружи, поэтому меню можно проверять в тестах.
 */
class FifteenMenu {
public:
    /// @brief Создаёт меню с игрой стандартного размера.
    /// @param in Поток ввода.
    /// @param out Поток вывода.
    FifteenMenu(std::istream& in, std::ostream& out);

    /// @brief Показывает поле и меню и выполняет выбранные пункты, пока не выбран выход или не закончился ввод.
    void run();

private:
    /// Пункты меню, номер пункта равен значению
    enum MenuItem { kExit = 0, kNewGame, kMakeMove, kCheckSolved, kLoadLayout };

    std::istream& in_;   ///< Поток ввода
    std::ostream& out_;  ///< Поток вывода
    FifteenPuzzle puzzle_;  ///< Текущая игра
    int moveCount_;      ///< Сколько ходов сделано

    void printMenu();
    bool handleChoice(const std::string& line);
    void runAction(int choice);
    void startNewGame();
    void makeMove();
    void checkSolved();
    void loadLayout();
};

#endif
