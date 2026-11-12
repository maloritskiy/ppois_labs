#include "ui/set_cli.hpp"

namespace lab1::ui {

namespace {

constexpr const char* kElementPrompt = "Введите элемент (атом или {множество}): ";

}  // namespace

SetCli::SetCli(std::istream& in, std::ostream& out) : io_(in, out), menu_(io_) { registerItems(); }

void SetCli::run() { menu_.run(); }

void SetCli::registerItems() {
    menu_.setStatusProvider([this] { return describeSets(); });
    registerInputItems();
    registerElementItems();
    registerOperationItems();
}

void SetCli::registerInputItems() {
    menu_.addItem("Задать множество A из строки", [this] { inputSet("A", setA_); });
    menu_.addItem("Задать множество B из строки", [this] { inputSet("B", setB_); });
}

void SetCli::registerElementItems() {
    menu_.addItem("Добавить элемент в A", [this] { addElement(); });
    menu_.addItem("Удалить элемент из A", [this] { removeElement(); });
    menu_.addItem("Проверить принадлежность элемента множеству A", [this] { checkMembership(); });
    menu_.addItem("Мощность A и проверка на пустоту", [this] { showCardinality(); });
}

void SetCli::registerOperationItems() {
    menu_.addItem("Объединение A + B", [this] { unite(); });
    menu_.addItem("Пересечение A * B", [this] { intersect(); });
    menu_.addItem("Разность A - B", [this] { subtract(); });
    menu_.addItem("Булеан множества A", [this] { showPowerSet(); });
    menu_.addItem("Сравнить A и B на равенство", [this] { compareSets(); });
}

std::string SetCli::describeSets() const {
    return "A = " + setA_.toString() + "\nB = " + setB_.toString() + "\n";
}

void SetCli::inputSet(const std::string& name, CantorSet& target) {
    const std::optional<std::string> line = io_.prompt("Введите множество " + name + ", например {a, {b}, {}}: ");
    if (!line) {
        return;
    }
    target = CantorSet::parse(*line);
    io_.writeLine(name + " = " + target.toString());
}

std::optional<Element> SetCli::promptElement() {
    const std::optional<std::string> line = io_.prompt(kElementPrompt);
    if (!line) {
        return std::nullopt;
    }
    return Element::parse(*line);
}

void SetCli::showResult(const std::string& label, const CantorSet& result) {
    io_.writeLine(label + " = " + result.toString());
}

void SetCli::addElement() {
    const std::optional<Element> element = promptElement();
    if (!element) {
        return;
    }
    const bool added = setA_.add(*element);
    io_.writeLine(added ? "Элемент добавлен" : "Элемент уже есть в множестве");
}

void SetCli::removeElement() {
    const std::optional<Element> element = promptElement();
    if (!element) {
        return;
    }
    const bool removed = setA_.remove(*element);
    io_.writeLine(removed ? "Элемент удалён" : "Такого элемента в множестве нет");
}

void SetCli::checkMembership() {
    const std::optional<Element> element = promptElement();
    if (!element) {
        return;
    }
    const bool present = setA_[*element];
    io_.writeLine(present ? "Элемент принадлежит A" : "Элемент не принадлежит A");
}

void SetCli::showCardinality() {
    const std::string emptiness = setA_.isEmpty() ? "множество пустое" : "множество непустое";
    io_.writeLine("Мощность A: " + std::to_string(setA_.cardinality()) + ", " + emptiness);
}

void SetCli::unite() {
    const CantorSet result = setA_ + setB_;
    showResult("A + B", result);
}

void SetCli::intersect() {
    const CantorSet result = setA_ * setB_;
    showResult("A * B", result);
}

void SetCli::subtract() {
    const CantorSet result = setA_ - setB_;
    showResult("A - B", result);
}

void SetCli::showPowerSet() {
    const CantorSet result = setA_.powerSet();
    showResult("P(A)", result);
}

void SetCli::compareSets() { io_.writeLine(setA_ == setB_ ? "A и B равны" : "A и B различны"); }

}  // namespace lab1::ui
