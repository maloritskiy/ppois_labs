#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>

#include "fifteen/fifteen_puzzle.hpp"
#include "ui/console_io.hpp"
#include "ui/menu.hpp"

namespace lab1::ui {

/// Рисует поле в виде текста: пустая клетка обозначается точкой, числа выровнены по правому краю.
std::string renderBoard(const FifteenPuzzle& puzzle);

/// @brief Консольное меню игры «Пятнашки».
class FifteenCli {
public:
    FifteenCli(std::istream& in, std::ostream& out);
    FifteenCli(const FifteenCli&) = delete;
    FifteenCli& operator=(const FifteenCli&) = delete;

    /// Запускает меню.
    void run();

private:
    void registerItems();
    std::string describeGame() const;
    void startNewGame();
    void makeMove();
    void checkSolved();
    void saveGame();
    void loadGame();

    ConsoleIo io_;
    Menu menu_;
    FifteenPuzzle puzzle_;
    std::size_t moveCount_ = 0;
};

}  // namespace lab1::ui
