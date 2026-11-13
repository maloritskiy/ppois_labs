#include "set_menu.h"

#include <stdexcept>

#include "console_input.h"

namespace {

const char kOpenBrace = '{';
const char* const kChoicePrompt = "Выберите пункт: ";
const char* const kElementPrompt = "Введите элемент (имя или {множество}): ";
const char* const kNotNumberMessage = "Введите номер пункта";
const char* const kNoSuchItemMessage = "Нет такого пункта";
const char* const kErrorPrefix = "Ошибка: ";
const char* const kExitTitle = "Выход";

const char* const kTitles[] = {"Задать множество A",
                               "Задать множество B",
                               "Добавить элемент в A",
                               "Удалить элемент из A",
                               "Проверить принадлежность элемента множеству A",
                               "Мощность A и проверка на пустоту",
                               "Объединение A + B",
                               "Пересечение A * B",
                               "Разность A - B",
                               "Булеан множества A",
                               "Сравнить A и B"};

bool isSubsetText(const std::string& text) { return !text.empty() && text[0] == kOpenBrace; }

}  // namespace

SetMenu::SetMenu(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

void SetMenu::run() {
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

void SetMenu::printMenu() {
    out_ << "\nA = " << setA_.toString() << "\nB = " << setB_.toString() << '\n';
    for (int item = kInputSetA; item <= kCompare; ++item) {
        out_ << item << ". " << kTitles[item - 1] << '\n';
    }
    out_ << kExit << ". " << kExitTitle << '\n';
}

bool SetMenu::handleChoice(const std::string& line) {
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

void SetMenu::runAction(int choice) {
    switch (choice) {
        case kInputSetA: inputSet("A", setA_); break;
        case kInputSetB: inputSet("B", setB_); break;
        case kAddElement: addElement(); break;
        case kRemoveElement: removeElement(); break;
        case kCheckElement: checkElement(); break;
        case kShowSize: showSize(); break;
        case kUnion: show("A + B", setA_ + setB_); break;
        case kIntersection: show("A * B", setA_ * setB_); break;
        case kDifference: show("A - B", setA_ - setB_); break;
        case kPowerSet: show("P(A)", setA_.powerSet()); break;
        case kCompare: compareSets(); break;
        default: out_ << kNoSuchItemMessage << '\n';
    }
}

void SetMenu::inputSet(const std::string& name, CantorSet& target) {
    std::string line;
    if (readLine(in_, out_, "Введите множество " + name + ", например {a, {b}}: ", line)) {
        target = CantorSet::fromString(line);
    }
}

void SetMenu::addElement() {
    std::string text;
    if (!askElement(text)) {
        return;
    }
    bool added = false;
    if (isSubsetText(text)) {
        CantorSet subset = CantorSet::fromString(text);
        added = setA_.add(subset);
    } else {
        added = setA_.add(text);
    }
    out_ << (added ? "Элемент добавлен" : "Такой элемент уже есть") << '\n';
}

void SetMenu::removeElement() {
    std::string text;
    if (!askElement(text)) {
        return;
    }
    bool removed = false;
    if (isSubsetText(text)) {
        CantorSet subset = CantorSet::fromString(text);
        removed = setA_.remove(subset);
    } else {
        removed = setA_.remove(text);
    }
    out_ << (removed ? "Элемент удалён" : "Такого элемента нет") << '\n';
}

void SetMenu::checkElement() {
    std::string text;
    if (!askElement(text)) {
        return;
    }
    bool found = false;
    if (isSubsetText(text)) {
        CantorSet subset = CantorSet::fromString(text);
        found = setA_[subset];
    } else {
        found = setA_[text];
    }
    out_ << (found ? "Элемент принадлежит A" : "Элемент не принадлежит A") << '\n';
}

void SetMenu::showSize() {
    std::string emptiness = setA_.isEmpty() ? "множество пустое" : "множество непустое";
    out_ << "Мощность A: " << setA_.size() << ", " << emptiness << '\n';
}

void SetMenu::compareSets() { out_ << (setA_ == setB_ ? "A и B равны" : "A и B различны") << '\n'; }

void SetMenu::show(const std::string& label, const CantorSet& value) {
    out_ << label << " = " << value.toString() << '\n';
}

bool SetMenu::askElement(std::string& text) { return readLine(in_, out_, kElementPrompt, text); }
