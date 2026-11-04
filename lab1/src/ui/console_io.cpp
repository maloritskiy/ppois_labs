#include "ui/console_io.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>

namespace lab1::ui {

namespace {

constexpr char kCarriageReturn = '\r';

}  // namespace

ConsoleIo::ConsoleIo(std::istream& in, std::ostream& out) : in_(in), out_(out) {}

void ConsoleIo::write(const std::string& text) {
    out_ << text;
    out_.flush();
}

void ConsoleIo::writeLine(const std::string& text) { out_ << text << '\n'; }

std::optional<std::string> ConsoleIo::prompt(const std::string& message) {
    write(message);
    std::string line;
    if (!std::getline(in_, line)) {
        return std::nullopt;
    }
    if (!line.empty() && line.back() == kCarriageReturn) {
        line.pop_back();
    }
    return line;
}

std::optional<int> parseInteger(const std::string& text) {
    try {
        std::size_t consumed = 0;
        const int value = std::stoi(text, &consumed);
        if (consumed != text.size()) {
            return std::nullopt;
        }
        return value;
    } catch (const std::logic_error&) {
        return std::nullopt;
    }
}

int requireInteger(const std::string& text) {
    const std::optional<int> value = parseInteger(text);
    if (!value) {
        throw std::invalid_argument("Ожидается целое число");
    }
    return *value;
}

}  // namespace lab1::ui
