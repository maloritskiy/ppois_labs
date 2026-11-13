#include "fifteen_menu.h"

#include <stdexcept>

#include "console_input.h"

namespace {

const char* const kEmptyCellSymbol = ".";
const char* const kChoicePrompt = "Выберите пункт: ";
const char* const kSizePrompt = "Размер поля (Enter - по умолчанию): ";
const char* const kTilePrompt = "Номер плитки: ";
const char* const kLayoutPrompt = "Размер и числа через пробел, например 3 1 2 3 4 5 6 7 0 8: ";
const char* const kNotNumberMessage = "Введите номер пункта";
const char* const kNoSuchItemMessage = "Нет такого пункта";
const char* const kErrorPrefix = "Ошибка: ";

const char* const kTitles[] = {"Новая игра", "Сделать ход", "Проверить правильность расстановки", "Ввести расстановку"};

std::string padLeft(const std::string& text, int width) {
    std::string result = text;
    while (static_cast<int>(result.size()) < width) {
        result = " " + result;
    }
    return result;
}

std::string cellText(int value) { return value == FifteenPuzzle::kEmptyCell ? kEmptyCellSymbol : std::to_string(value); }

}  // namespace

std::string renderBoard(const FifteenPuzzle& puzzle) {
    int size = puzzle.getSize();
    int width = static_cast<int>(std::to_string(size * size - 1).size());
    std::string text;
    for (int row = 0; row < size; ++row) {
        for (int column = 0; column < size; ++column) {
            text += column > 0 ? " " : "";
            text += padLeft(cellText(puzzle[row * size + column]), width);
        }
        text += "\n";
    }
    return text;
}

FifteenMenu::FifteenMenu(std::istream& in, std::ostream& out) : in_(in), out_(out), moveCount_(0) {}

void FifteenMenu::run() {
    bool running = true;
    while (running) {
        printMenu();
        std::string line;
        if (!readLine(in_, out_, kChoicePrompt, line)) {
            return;
        }
        running = handleChoice(line);
    }
}

void FifteenMenu::printMenu() {
    out_ << '\n' << renderBoard(puzzle_) << "Ходов: " << moveCount_ << '\n';
    for (int item = kNewGame; item <= kLoadLayout; ++item) {
        out_ << item << ". " << kTitles[item - 1] << '\n';
    }
    out_ << kExit << ". Выход\n";
}

bool FifteenMenu::handleChoice(const std::string& line) {
    int choice = 0;
    if (!parseInt(line, choice)) {
        out_ << kNotNumberMessage << '\n';
        return true;
    }
    if (choice == kExit) {
        return false;
    }
    try {
        runAction(choice);
    } catch (const std::exception& error) {
        out_ << kErrorPrefix << error.what() << '\n';
    }
    return true;
}

void FifteenMenu::runAction(int choice) {
    switch (choice) {
        case kNewGame: startNewGame(); break;
        case kMakeMove: makeMove(); break;
        case kCheckSolved: checkSolved(); break;
        case kLoadLayout: loadLayout(); break;
        default: out_ << kNoSuchItemMessage << '\n';
    }
}

void FifteenMenu::startNewGame() {
    std::string line;
    if (!readLine(in_, out_, kSizePrompt, line)) {
        return;
    }
    int size = FifteenPuzzle::kDefaultSize;
    if (!line.empty() && !parseInt(line, size)) {
        throw std::invalid_argument("Размер должен быть числом");
    }
    puzzle_ = FifteenPuzzle(size);
    moveCount_ = 0;
}

void FifteenMenu::makeMove() {
    std::string line;
    if (!readLine(in_, out_, kTilePrompt, line)) {
        return;
    }
    int tile = 0;
    if (!parseInt(line, tile)) {
        throw std::invalid_argument("Номер плитки должен быть числом");
    }
    if (!puzzle_.moveTile(tile)) {
        out_ << "Эту плитку сейчас сдвинуть нельзя\n";
        return;
    }
    ++moveCount_;
    if (puzzle_.isSolved()) {
        out_ << "Поздравляем, головоломка собрана\n";
    }
}

void FifteenMenu::checkSolved() {
    out_ << (puzzle_.isSolved() ? "Расстановка правильная" : "Расстановка неправильная") << '\n';
}

void FifteenMenu::loadLayout() {
    out_ << kLayoutPrompt;
    FifteenPuzzle loaded;
    in_ >> loaded;
    bool success = !in_.fail();
    in_.clear();
    std::string restOfLine;
    std::getline(in_, restOfLine);
    if (!success) {
        throw std::invalid_argument("Некорректная расстановка");
    }
    puzzle_ = loaded;
    moveCount_ = 0;
}
