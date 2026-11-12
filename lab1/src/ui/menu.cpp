#include "ui/menu.hpp"

#include <exception>
#include <utility>

namespace lab1::ui {

namespace {

constexpr int kExitChoice = 0;
constexpr const char* kExitTitle = "Выход";
constexpr const char* kChoicePrompt = "Выберите пункт: ";
constexpr const char* kInvalidChoiceMessage = "Нет такого пункта меню";
constexpr const char* kErrorPrefix = "Ошибка: ";

}  // namespace

Menu::Menu(ConsoleIo& io) : io_(io) {}

void Menu::addItem(const std::string& title, Action action) { items_.push_back(Item{title, std::move(action)}); }

void Menu::setStatusProvider(StatusProvider provider) { statusProvider_ = std::move(provider); }

void Menu::run() {
    bool running = true;
    while (running) {
        printMenu();
        const std::optional<std::string> line = io_.prompt(kChoicePrompt);
        running = line.has_value() && handleChoice(*line);
    }
}

void Menu::printMenu() const {
    if (statusProvider_) {
        io_.write(statusProvider_());
    }
    io_.writeLine("");
    for (std::size_t index = 0; index < items_.size(); ++index) {
        io_.writeLine(std::to_string(index + 1) + ". " + items_[index].title);
    }
    io_.writeLine(std::to_string(kExitChoice) + ". " + kExitTitle);
}

bool Menu::handleChoice(const std::string& line) {
    const std::optional<int> choice = parseInteger(line);
    if (choice == kExitChoice) {
        return false;
    }
    if (!choice || *choice < 1 || *choice > static_cast<int>(items_.size())) {
        io_.writeLine(kInvalidChoiceMessage);
        return true;
    }
    runItem(items_[static_cast<std::size_t>(*choice - 1)]);
    return true;
}

void Menu::runItem(const Item& item) {
    try {
        item.action();
    } catch (const std::exception& error) {
        io_.writeLine(std::string(kErrorPrefix) + error.what());
    }
}

}  // namespace lab1::ui
