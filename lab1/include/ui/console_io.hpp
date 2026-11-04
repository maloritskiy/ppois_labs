#pragma once

#include <iosfwd>
#include <optional>
#include <string>

namespace lab1::ui {

/// Тонкая обёртка над потоками ввода-вывода консольного интерфейса.
class ConsoleIo {
public:
    ConsoleIo(std::istream& in, std::ostream& out);

    /// Выводит текст без перевода строки.
    void write(const std::string& text);
    /// Выводит текст и переводит строку.
    void writeLine(const std::string& text);
    /// Выводит приглашение и читает строку. @return std::nullopt при конце ввода.
    std::optional<std::string> prompt(const std::string& message);

private:
    std::istream& in_;
    std::ostream& out_;
};

/// Разбирает целое число; @return std::nullopt, если текст не является целым числом.
std::optional<int> parseInteger(const std::string& text);
/// Разбирает целое число. @throw std::invalid_argument если текст не является целым числом.
int requireInteger(const std::string& text);

}  // namespace lab1::ui
