#include "console_input.h"

#include <stdexcept>

namespace {

const char kCarriageReturn = '\r';

}  // namespace

bool readLine(std::istream& in, std::ostream& out, const std::string& prompt, std::string& line) {
    out << prompt;
    if (!std::getline(in, line)) {
        return false;
    }
    if (!line.empty() && line[line.size() - 1] == kCarriageReturn) {
        line.erase(line.size() - 1);
    }
    return true;
}

bool parseInt(const std::string& text, int& value) {
    try {
        std::size_t parsedLength = 0;
        int parsed = std::stoi(text, &parsedLength);
        if (parsedLength != text.size()) {
            return false;
        }
        value = parsed;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
