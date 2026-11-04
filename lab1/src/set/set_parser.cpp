#include "set_parser.hpp"

#include <cctype>
#include <stdexcept>

#include "set/set_syntax.hpp"

namespace lab1 {

namespace {

constexpr std::size_t kMaxNestingDepth = 100;

}  // namespace

SetParser::SetParser(const std::string& text) : text_(text) {}

CantorSet SetParser::parse() {
    CantorSet result = parseSet(0);
    skipSpaces();
    if (position_ != text_.size()) {
        fail("unexpected characters after the closing brace");
    }
    return result;
}

CantorSet SetParser::parseSet(std::size_t depth) {
    if (depth > kMaxNestingDepth) {
        fail("nesting is too deep");
    }
    expect(syntax::kOpenBrace);
    CantorSet result;
    if (consumeIf(syntax::kCloseBrace)) {
        return result;
    }
    parseElementList(result, depth);
    expect(syntax::kCloseBrace);
    return result;
}

void SetParser::parseElementList(CantorSet& result, std::size_t depth) {
    do {
        Element element = parseElement(depth + 1);
        result.add(element);
    } while (consumeIf(syntax::kSeparator));
}

Element SetParser::parseElement(std::size_t depth) {
    skipSpaces();
    const bool startsSet = position_ < text_.size() && text_[position_] == syntax::kOpenBrace;
    if (startsSet) {
        const CantorSet nested = parseSet(depth);
        return Element(nested);
    }
    const std::string atom = parseAtom();
    return Element(atom);
}

std::string SetParser::parseAtom() {
    const std::size_t start = position_;
    while (position_ < text_.size() && Element::isValidAtom(std::string(1, text_[position_]))) {
        ++position_;
    }
    if (start == position_) {
        fail("atom expected");
    }
    return text_.substr(start, position_ - start);
}

void SetParser::skipSpaces() {
    while (position_ < text_.size() && std::isspace(static_cast<unsigned char>(text_[position_])) != 0) {
        ++position_;
    }
}

bool SetParser::consumeIf(char expected) {
    skipSpaces();
    if (position_ < text_.size() && text_[position_] == expected) {
        ++position_;
        return true;
    }
    return false;
}

void SetParser::expect(char expected) {
    if (!consumeIf(expected)) {
        fail(std::string("expected '") + expected + "'");
    }
}

void SetParser::fail(const std::string& message) const {
    throw std::invalid_argument("Invalid set at position " + std::to_string(position_) + ": " + message);
}

}  // namespace lab1
