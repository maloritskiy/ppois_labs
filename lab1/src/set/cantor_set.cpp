#include "set/cantor_set.hpp"

#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "set/set_syntax.hpp"
#include "set_parser.hpp"

namespace lab1 {

namespace {

/// Читает из потока текст одного множества со сбалансированными скобками.
bool readBalancedText(std::istream& in, std::string& text) {
    in >> std::ws;
    int depth = 0;
    char symbol = 0;
    while (in.get(symbol)) {
        if (depth == 0 && symbol != syntax::kOpenBrace) {
            return false;
        }
        text.push_back(symbol);
        if (symbol == syntax::kOpenBrace) {
            ++depth;
        }
        if (symbol == syntax::kCloseBrace && --depth == 0) {
            return true;
        }
    }
    return false;
}

}  // namespace

CantorSet::CantorSet(std::initializer_list<Element> elements) {
    for (const Element& element : elements) {
        add(element);
    }
}

CantorSet CantorSet::parse(const std::string& text) {
    SetParser parser(text);
    return parser.parse();
}

bool CantorSet::isEmpty() const { return elements_.empty(); }

std::size_t CantorSet::cardinality() const { return elements_.size(); }

bool CantorSet::add(const Element& element) {
    if (contains(element)) {
        return false;
    }
    elements_.push_back(element);
    return true;
}

bool CantorSet::remove(const Element& element) {
    const std::size_t index = indexOf(element);
    if (index == elements_.size()) {
        return false;
    }
    elements_.erase(elements_.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool CantorSet::contains(const Element& element) const { return indexOf(element) != elements_.size(); }

bool CantorSet::operator[](const Element& element) const { return contains(element); }

CantorSet& CantorSet::operator+=(const CantorSet& other) {
    for (const Element& element : other.elements_) {
        add(element);
    }
    return *this;
}

CantorSet& CantorSet::operator*=(const CantorSet& other) {
    keepWhere(other, true);
    return *this;
}

CantorSet& CantorSet::operator-=(const CantorSet& other) {
    keepWhere(other, false);
    return *this;
}

CantorSet CantorSet::powerSet() const {
    if (elements_.size() > kMaxPowerSetCardinality) {
        throw std::length_error("Power set is too large to build");
    }
    const std::size_t subsetCount = std::size_t{1} << elements_.size();
    CantorSet result;
    for (std::size_t mask = 0; mask < subsetCount; ++mask) {
        const CantorSet subset = subsetFromMask(mask);
        result.elements_.emplace_back(subset);
    }
    return result;
}

std::string CantorSet::toString() const {
    std::string text(1, syntax::kOpenBrace);
    for (std::size_t index = 0; index < elements_.size(); ++index) {
        if (index > 0) {
            text += syntax::kElementDelimiter;
        }
        text += elements_[index].toString();
    }
    text += syntax::kCloseBrace;
    return text;
}

bool CantorSet::operator==(const CantorSet& other) const {
    if (elements_.size() != other.elements_.size()) {
        return false;
    }
    return std::all_of(elements_.begin(), elements_.end(), [&other](const Element& element) { return other.contains(element); });
}

bool CantorSet::operator!=(const CantorSet& other) const { return !(*this == other); }

std::size_t CantorSet::indexOf(const Element& element) const {
    const auto found = std::find(elements_.begin(), elements_.end(), element);
    return static_cast<std::size_t>(found - elements_.begin());
}

void CantorSet::keepWhere(const CantorSet& other, bool keepContained) {
    std::vector<Element> kept;
    for (const Element& element : elements_) {
        if (other.contains(element) == keepContained) {
            kept.push_back(element);
        }
    }
    elements_ = std::move(kept);
}

CantorSet CantorSet::subsetFromMask(std::size_t mask) const {
    CantorSet subset;
    for (std::size_t index = 0; index < elements_.size(); ++index) {
        if (((mask >> index) & 1U) != 0) {
            subset.elements_.push_back(elements_[index]);
        }
    }
    return subset;
}

CantorSet operator+(CantorSet lhs, const CantorSet& rhs) {
    lhs += rhs;
    return lhs;
}

CantorSet operator*(CantorSet lhs, const CantorSet& rhs) {
    lhs *= rhs;
    return lhs;
}

CantorSet operator-(CantorSet lhs, const CantorSet& rhs) {
    lhs -= rhs;
    return lhs;
}

std::ostream& operator<<(std::ostream& out, const CantorSet& set) { return out << set.toString(); }

std::istream& operator>>(std::istream& in, CantorSet& set) {
    std::string text;
    if (!readBalancedText(in, text)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    try {
        set = CantorSet::parse(text);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}

}  // namespace lab1
