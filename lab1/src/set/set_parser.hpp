#pragma once

#include <cstddef>
#include <string>

#include "set/cantor_set.hpp"

namespace lab1 {

/// Рекурсивный разбор текстового представления множества (внутренний класс библиотеки).
class SetParser {
public:
    explicit SetParser(const std::string& text);

    /// Разбирает всю строку как одно множество. @throw std::invalid_argument при ошибке.
    CantorSet parse();

private:
    CantorSet parseSet(std::size_t depth);
    void parseElementList(CantorSet& result, std::size_t depth);
    Element parseElement(std::size_t depth);
    std::string parseAtom();
    void skipSpaces();
    bool consumeIf(char expected);
    void expect(char expected);
    [[noreturn]] void fail(const std::string& message) const;

    std::string text_;
    std::size_t position_ = 0;
};

}  // namespace lab1
