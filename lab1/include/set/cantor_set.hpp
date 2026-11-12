#pragma once

#include <cstddef>
#include <initializer_list>
#include <iosfwd>
#include <string>
#include <vector>

#include "set/element.hpp"

namespace lab1 {

/// @brief Неориентированное канторовское множество: элементы не повторяются и не упорядочены.
///
/// Элементом множества может быть атом или другое множество.
/// Внимание: запись `CantorSet{other}` с одним множеством в фигурных скобках создаёт множество
/// из одного элемента, а не копию — для копирования используйте круглые скобки или `=`.
class CantorSet {
public:
    /// Максимальная мощность множества, для которого разрешено строить булеан.
    static constexpr std::size_t kMaxPowerSetCardinality = 16;

    CantorSet() = default;
    /// Создаёт множество из списка элементов (повторы отбрасываются).
    CantorSet(std::initializer_list<Element> elements);
    CantorSet(const CantorSet& other) = default;
    CantorSet(CantorSet&& other) noexcept = default;
    CantorSet& operator=(const CantorSet& other) = default;
    CantorSet& operator=(CantorSet&& other) noexcept = default;
    ~CantorSet() = default;

    /// Разбирает множество из строки вида `{a, b, {a, b}, {}}`.
    /// Повторяющиеся элементы объединяются. @throw std::invalid_argument при ошибке синтаксиса.
    static CantorSet parse(const std::string& text);

    /// Проверка на пустое множество.
    bool isEmpty() const;
    /// Мощность множества.
    std::size_t cardinality() const;

    /// Добавляет элемент. @return true, если элемента ещё не было.
    bool add(const Element& element);
    /// Удаляет элемент. @return true, если элемент был в множестве.
    bool remove(const Element& element);
    /// Проверяет принадлежность элемента множеству.
    bool contains(const Element& element) const;
    /// Проверка принадлежности элемента множеству (то же, что contains).
    bool operator[](const Element& element) const;

    /// Объединение.
    CantorSet& operator+=(const CantorSet& other);
    /// Пересечение.
    CantorSet& operator*=(const CantorSet& other);
    /// Разность.
    CantorSet& operator-=(const CantorSet& other);

    /// Булеан — множество всех подмножеств.
    /// @throw std::length_error если мощность больше kMaxPowerSetCardinality.
    CantorSet powerSet() const;

    /// Текстовое представление вида `{a, {b}}`.
    std::string toString() const;

    /// Равенство множеств (порядок элементов не важен).
    bool operator==(const CantorSet& other) const;
    bool operator!=(const CantorSet& other) const;

private:
    std::size_t indexOf(const Element& element) const;
    void keepWhere(const CantorSet& other, bool keepContained);
    CantorSet subsetFromMask(std::size_t mask) const;

    std::vector<Element> elements_;
};

/// Объединение двух множеств.
CantorSet operator+(CantorSet lhs, const CantorSet& rhs);
/// Пересечение двух множеств.
CantorSet operator*(CantorSet lhs, const CantorSet& rhs);
/// Разность двух множеств.
CantorSet operator-(CantorSet lhs, const CantorSet& rhs);

/// Записывает множество в поток.
std::ostream& operator<<(std::ostream& out, const CantorSet& set);
/// Читает из потока одно множество `{...}`; при ошибке выставляет failbit.
std::istream& operator>>(std::istream& in, CantorSet& set);

}  // namespace lab1
