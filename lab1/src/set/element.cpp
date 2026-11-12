#include "set/element.hpp"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ostream>
#include <stdexcept>

#include "set/cantor_set.hpp"
#include "set/set_syntax.hpp"

namespace lab1 {

namespace {

std::string trim(const std::string& text) {
    const auto isSpace = [](unsigned char symbol) { return std::isspace(symbol) != 0; };
    const auto first = std::find_if_not(text.begin(), text.end(), isSpace);
    const auto last = std::find_if_not(text.rbegin(), text.rend(), isSpace).base();
    return first < last ? std::string(first, last) : std::string();
}

}  // namespace

Element::Element(const std::string& atom) : atom_(atom) {
    if (!isValidAtom(atom_)) {
        throw std::invalid_argument("Invalid atom: '" + atom + "'");
    }
}

Element::Element(const char* atom) : Element(std::string(atom)) {}

Element::Element(const CantorSet& subset) : subset_(std::make_unique<CantorSet>(subset)) {}

Element::Element(const Element& other)
    : atom_(other.atom_), subset_(other.subset_ ? std::make_unique<CantorSet>(*other.subset_) : nullptr) {}

Element::Element(Element&& other) noexcept = default;

Element& Element::operator=(const Element& other) {
    if (this != &other) {
        Element copy(other);
        *this = std::move(copy);
    }
    return *this;
}

Element& Element::operator=(Element&& other) noexcept = default;

Element::~Element() = default;

Element Element::parse(const std::string& text) {
    const std::string trimmed = trim(text);
    if (trimmed.empty()) {
        throw std::invalid_argument("Element text is empty");
    }
    if (trimmed.front() == syntax::kOpenBrace) {
        const CantorSet subset = CantorSet::parse(trimmed);
        return Element(subset);
    }
    return Element(trimmed);
}

bool Element::isValidAtom(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    const auto isForbidden = [](unsigned char symbol) {
        return std::isspace(symbol) != 0 || std::strchr(syntax::kReservedChars, symbol) != nullptr;
    };
    return std::none_of(text.begin(), text.end(), isForbidden);
}

bool Element::isSet() const { return subset_ != nullptr; }

const std::string& Element::atom() const {
    if (isSet()) {
        throw std::logic_error("Element is a set, not an atom");
    }
    return atom_;
}

const CantorSet& Element::subset() const {
    if (!isSet()) {
        throw std::logic_error("Element is an atom, not a set");
    }
    return *subset_;
}

std::string Element::toString() const { return isSet() ? subset_->toString() : atom_; }

bool Element::operator==(const Element& other) const {
    if (isSet() != other.isSet()) {
        return false;
    }
    return isSet() ? *subset_ == *other.subset_ : atom_ == other.atom_;
}

bool Element::operator!=(const Element& other) const { return !(*this == other); }

std::ostream& operator<<(std::ostream& out, const Element& element) { return out << element.toString(); }

}  // namespace lab1
