#include "ui/fifteen_cli.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace lab1::ui {

namespace {

constexpr const char* kEmptyCellSymbol = ".";
constexpr const char* kSolvedMessage = "Расстановка правильная: головоломка собрана";
constexpr const char* kNotSolvedMessage = "Расстановка неправильная: головоломка ещё не собрана";
constexpr const char* kCongratulations = "Поздравляем! Головоломка собрана";
constexpr const char* kCannotMoveMessage = "Эту плитку сейчас сдвинуть нельзя";

std::string cellText(int value) {
    return value == FifteenPuzzle::kEmptyCell ? kEmptyCellSymbol : std::to_string(value);
}

constexpr const char* kPathPrompt = "Путь к файлу: ";

}  // namespace

std::string renderBoard(const FifteenPuzzle& puzzle) {
    const std::size_t size = puzzle.size();
    const std::size_t width = std::to_string(size * size - 1).size();
    std::ostringstream text;
    for (std::size_t row = 0; row < size; ++row) {
        for (std::size_t column = 0; column < size; ++column) {
            const Position position{row, column};
            text << (column > 0 ? " " : "") << std::setw(static_cast<int>(width)) << cellText(puzzle[position]);
        }
        text << '\n';
    }
    return text.str();
}

FifteenCli::FifteenCli(std::istream& in, std::ostream& out) : io_(in, out), menu_(io_) { registerItems(); }

void FifteenCli::run() { menu_.run(); }

void FifteenCli::registerItems() {
    menu_.setStatusProvider([this] { return describeGame(); });
    menu_.addItem("Новая игра", [this] { startNewGame(); });
    menu_.addItem("Сделать ход (номер плитки)", [this] { makeMove(); });
    menu_.addItem("Проверить правильность расстановки", [this] { checkSolved(); });
    menu_.addItem("Сохранить игру в файл", [this] { saveGame(); });
    menu_.addItem("Загрузить игру из файла", [this] { loadGame(); });
}

std::string FifteenCli::describeGame() const {
    return renderBoard(puzzle_) + "Ходов: " + std::to_string(moveCount_) + "\n";
}

void FifteenCli::startNewGame() {
    const std::optional<std::string> line = io_.prompt("Размер поля (Enter — размер по умолчанию): ");
    if (!line) {
        return;
    }
    std::size_t size = FifteenPuzzle::kDefaultSize;
    if (!line->empty()) {
        size = static_cast<std::size_t>(requireInteger(*line));
    }
    puzzle_ = FifteenPuzzle(size);
    moveCount_ = 0;
}

void FifteenCli::makeMove() {
    const std::optional<std::string> line = io_.prompt("Номер плитки: ");
    if (!line) {
        return;
    }
    const int tile = requireInteger(*line);
    if (!puzzle_.moveTile(tile)) {
        io_.writeLine(kCannotMoveMessage);
        return;
    }
    ++moveCount_;
    if (puzzle_.isSolved()) {
        io_.writeLine(kCongratulations);
    }
}

void FifteenCli::checkSolved() { io_.writeLine(puzzle_.isSolved() ? kSolvedMessage : kNotSolvedMessage); }

void FifteenCli::saveGame() {
    const std::optional<std::string> path = io_.prompt(kPathPrompt);
    if (!path) {
        return;
    }
    std::ofstream file(*path);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл для записи: " + *path);
    }
    file << puzzle_;
    io_.writeLine("Игра сохранена");
}

void FifteenCli::loadGame() {
    const std::optional<std::string> path = io_.prompt(kPathPrompt);
    if (!path) {
        return;
    }
    std::ifstream file(*path);
    if (!file) {
        throw std::runtime_error("Не удалось открыть файл для чтения: " + *path);
    }
    FifteenPuzzle loaded;
    if (!(file >> loaded)) {
        throw std::invalid_argument("Файл не содержит корректной игры");
    }
    puzzle_ = loaded;
    moveCount_ = 0;
    io_.writeLine("Игра загружена");
}

}  // namespace lab1::ui
