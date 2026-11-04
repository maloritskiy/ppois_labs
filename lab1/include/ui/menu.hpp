#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/console_io.hpp"

namespace lab1::ui {

/// @brief Текстовое меню: нумерованные пункты и выход по нулю.
///
/// Исключения, брошенные действием, перехватываются и выводятся как сообщение об ошибке.
class Menu {
public:
    using Action = std::function<void()>;
    using StatusProvider = std::function<std::string()>;

    explicit Menu(ConsoleIo& io);

    /// Добавляет пункт меню; нумерация идёт с единицы в порядке добавления.
    void addItem(const std::string& title, Action action);
    /// Задаёт функцию, текст которой выводится перед меню (например, состояние игры).
    void setStatusProvider(StatusProvider provider);
    /// Выполняет цикл меню до выбора выхода или конца ввода.
    void run();

private:
    struct Item {
        std::string title;
        Action action;
    };

    void printMenu() const;
    bool handleChoice(const std::string& line);
    void runItem(const Item& item);

    ConsoleIo& io_;
    std::vector<Item> items_;
    StatusProvider statusProvider_;
};

}  // namespace lab1::ui
