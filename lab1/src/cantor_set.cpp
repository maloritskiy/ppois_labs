#include "cantor_set.h"

#include <stdexcept>

namespace {

const int kInitialCapacity = 4;
const int kGrowthFactor = 2;
const int kMaxNestingDepth = 100;
const int kNotFound = -1;
const char kOpenBrace = '{';
const char kCloseBrace = '}';
const char kSeparator = ',';
const char* const kElementDelimiter = ", ";

int lengthOf(const std::string& text) { return static_cast<int>(text.size()); }

bool isSpace(char symbol) { return symbol == ' ' || symbol == '\t' || symbol == '\n' || symbol == '\r'; }

bool isAtomChar(char symbol) {
    bool isReserved = symbol == kOpenBrace || symbol == kCloseBrace || symbol == kSeparator;
    return !isSpace(symbol) && !isReserved;
}

bool isValidAtom(const std::string& atom) {
    if (atom.empty()) {
        return false;
    }
    for (int i = 0; i < lengthOf(atom); ++i) {
        if (!isAtomChar(atom[i])) {
            return false;
        }
    }
    return true;
}

void fail(int position, const std::string& message) {
    throw std::invalid_argument("Ошибка в позиции " + std::to_string(position) + ": " + message);
}

void skipSpaces(const std::string& text, int& position) {
    while (position < lengthOf(text) && isSpace(text[position])) {
        ++position;
    }
}

bool consume(const std::string& text, int& position, char expected) {
    skipSpaces(text, position);
    if (position < lengthOf(text) && text[position] == expected) {
        ++position;
        return true;
    }
    return false;
}

std::string parseAtom(const std::string& text, int& position) {
    int start = position;
    while (position < lengthOf(text) && isAtomChar(text[position])) {
        ++position;
    }
    if (start == position) {
        fail(position, "ожидался элемент");
    }
    return text.substr(start, position - start);
}

CantorSet parseSet(const std::string& text, int& position, int depth);

void parseElement(const std::string& text, int& position, int depth, CantorSet& result) {
    skipSpaces(text, position);
    bool isSubset = position < lengthOf(text) && text[position] == kOpenBrace;
    if (isSubset) {
        CantorSet subset = parseSet(text, position, depth + 1);
        result.add(subset);
    } else {
        std::string atom = parseAtom(text, position);
        result.add(atom);
    }
}

CantorSet parseSet(const std::string& text, int& position, int depth) {
    if (depth > kMaxNestingDepth) {
        fail(position, "слишком глубокая вложенность");
    }
    if (!consume(text, position, kOpenBrace)) {
        fail(position, "ожидалась '{'");
    }
    CantorSet result;
    if (consume(text, position, kCloseBrace)) {
        return result;
    }
    do {
        parseElement(text, position, depth, result);
    } while (consume(text, position, kSeparator));
    if (!consume(text, position, kCloseBrace)) {
        fail(position, "ожидалась '}'");
    }
    return result;
}

}  // namespace

CantorSet::CantorSet() : items_(new Element[kInitialCapacity]), count_(0), capacity_(kInitialCapacity) {}

CantorSet::CantorSet(const CantorSet& other)
    : items_(new Element[other.capacity_]), count_(0), capacity_(other.capacity_) {
    copyItems(other);
}

CantorSet& CantorSet::operator=(const CantorSet& other) {
    if (this != &other) {
        clear();
        items_ = new Element[other.capacity_];
        capacity_ = other.capacity_;
        copyItems(other);
    }
    return *this;
}

CantorSet::~CantorSet() { clear(); }

CantorSet CantorSet::fromString(const std::string& text) {
    int position = 0;
    CantorSet result = parseSet(text, position, 0);
    skipSpaces(text, position);
    if (position != lengthOf(text)) {
        fail(position, "лишние символы после '}'");
    }
    return result;
}

std::string CantorSet::toString() const {
    std::string text(1, kOpenBrace);
    for (int i = 0; i < count_; ++i) {
        if (i > 0) {
            text += kElementDelimiter;
        }
        bool isSubset = items_[i].subset != nullptr;
        text += isSubset ? items_[i].subset->toString() : items_[i].atom;
    }
    text += kCloseBrace;
    return text;
}

bool CantorSet::isEmpty() const { return count_ == 0; }

int CantorSet::size() const { return count_; }

bool CantorSet::add(const std::string& atom) {
    if (!isValidAtom(atom)) {
        throw std::invalid_argument("Недопустимое имя элемента: '" + atom + "'");
    }
    if (findAtom(atom) != kNotFound) {
        return false;
    }
    append(atom, nullptr);
    return true;
}

bool CantorSet::add(const CantorSet& subset) {
    if (findSubset(subset) != kNotFound) {
        return false;
    }
    append("", &subset);
    return true;
}

bool CantorSet::remove(const std::string& atom) {
    int index = findAtom(atom);
    if (index == kNotFound) {
        return false;
    }
    removeAt(index);
    return true;
}

bool CantorSet::remove(const CantorSet& subset) {
    int index = findSubset(subset);
    if (index == kNotFound) {
        return false;
    }
    removeAt(index);
    return true;
}

bool CantorSet::operator[](const std::string& atom) const { return findAtom(atom) != kNotFound; }

bool CantorSet::operator[](const CantorSet& subset) const { return findSubset(subset) != kNotFound; }

CantorSet& CantorSet::operator+=(const CantorSet& other) {
    for (int i = 0; i < other.count_; ++i) {
        if (!containsItem(other.items_[i])) {
            append(other.items_[i].atom, other.items_[i].subset);
        }
    }
    return *this;
}

CantorSet& CantorSet::operator*=(const CantorSet& other) {
    keepItems(other, true);
    return *this;
}

CantorSet& CantorSet::operator-=(const CantorSet& other) {
    keepItems(other, false);
    return *this;
}

CantorSet CantorSet::powerSet() const {
    if (count_ > kMaxPowerSetSize) {
        throw std::length_error("Булеан слишком большой");
    }
    CantorSet result;
    int subsetCount = 1 << count_;
    for (int mask = 0; mask < subsetCount; ++mask) {
        CantorSet subset = subsetByMask(mask);
        result.append("", &subset);
    }
    return result;
}

bool CantorSet::operator==(const CantorSet& other) const {
    if (count_ != other.count_) {
        return false;
    }
    for (int i = 0; i < count_; ++i) {
        if (!other.containsItem(items_[i])) {
            return false;
        }
    }
    return true;
}

bool CantorSet::operator!=(const CantorSet& other) const { return !(*this == other); }

void CantorSet::clear() {
    for (int i = 0; i < count_; ++i) {
        delete items_[i].subset;
    }
    delete[] items_;
    items_ = nullptr;
    count_ = 0;
    capacity_ = 0;
}

void CantorSet::copyItems(const CantorSet& other) {
    for (int i = 0; i < other.count_; ++i) {
        append(other.items_[i].atom, other.items_[i].subset);
    }
}

void CantorSet::grow() {
    int newCapacity = capacity_ * kGrowthFactor;
    Element* newItems = new Element[newCapacity];
    for (int i = 0; i < count_; ++i) {
        newItems[i] = items_[i];
    }
    delete[] items_;
    items_ = newItems;
    capacity_ = newCapacity;
}

void CantorSet::append(const std::string& atom, const CantorSet* subset) {
    if (count_ == capacity_) {
        grow();
    }
    items_[count_].atom = atom;
    items_[count_].subset = subset == nullptr ? nullptr : new CantorSet(*subset);
    ++count_;
}

void CantorSet::removeAt(int index) {
    delete items_[index].subset;
    for (int i = index; i < count_ - 1; ++i) {
        items_[i] = items_[i + 1];
    }
    --count_;
    items_[count_] = Element();
}

int CantorSet::findAtom(const std::string& atom) const {
    for (int i = 0; i < count_; ++i) {
        if (items_[i].subset == nullptr && items_[i].atom == atom) {
            return i;
        }
    }
    return kNotFound;
}

int CantorSet::findSubset(const CantorSet& subset) const {
    for (int i = 0; i < count_; ++i) {
        if (items_[i].subset != nullptr && *items_[i].subset == subset) {
            return i;
        }
    }
    return kNotFound;
}

bool CantorSet::containsItem(const Element& item) const {
    if (item.subset != nullptr) {
        return findSubset(*item.subset) != kNotFound;
    }
    return findAtom(item.atom) != kNotFound;
}

void CantorSet::keepItems(const CantorSet& other, bool keepCommon) {
    for (int i = count_ - 1; i >= 0; --i) {
        if (other.containsItem(items_[i]) != keepCommon) {
            removeAt(i);
        }
    }
}

CantorSet CantorSet::subsetByMask(int mask) const {
    CantorSet subset;
    for (int i = 0; i < count_; ++i) {
        bool selected = ((mask >> i) & 1) != 0;
        if (selected) {
            subset.append(items_[i].atom, items_[i].subset);
        }
    }
    return subset;
}

CantorSet operator+(CantorSet left, const CantorSet& right) {
    left += right;
    return left;
}

CantorSet operator*(CantorSet left, const CantorSet& right) {
    left *= right;
    return left;
}

CantorSet operator-(CantorSet left, const CantorSet& right) {
    left -= right;
    return left;
}

std::ostream& operator<<(std::ostream& out, const CantorSet& set) { return out << set.toString(); }

std::istream& operator>>(std::istream& in, CantorSet& set) {
    std::string line;
    if (!std::getline(in, line)) {
        return in;
    }
    try {
        set = CantorSet::fromString(line);
    } catch (const std::invalid_argument&) {
        in.setstate(std::ios::failbit);
    }
    return in;
}
