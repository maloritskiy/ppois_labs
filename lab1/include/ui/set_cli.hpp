#pragma once

#include <iosfwd>
#include <optional>
#include <string>

#include "set/cantor_set.hpp"
#include "ui/console_io.hpp"
#include "ui/menu.hpp"

namespace lab1::ui {

/// @brief Консольное меню для работы с двумя канторовскими множествами A и B.
///
/// Интерфейс зависит от класса CantorSet, но сам класс о нём ничего не знает.
class SetCli {
public:
    SetCli(std::istream& in, std::ostream& out);
    SetCli(const SetCli&) = delete;
    SetCli& operator=(const SetCli&) = delete;

    /// Запускает меню.
    void run();

private:
    void registerItems();
    void registerInputItems();
    void registerElementItems();
    void registerOperationItems();

    std::string describeSets() const;
    void inputSet(const std::string& name, CantorSet& target);
    std::optional<Element> promptElement();
    void showResult(const std::string& label, const CantorSet& result);

    void addElement();
    void removeElement();
    void checkMembership();
    void showCardinality();
    void unite();
    void intersect();
    void subtract();
    void showPowerSet();
    void compareSets();

    ConsoleIo io_;
    Menu menu_;
    CantorSet setA_;
    CantorSet setB_;
};

}  // namespace lab1::ui
