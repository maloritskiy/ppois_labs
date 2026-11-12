#pragma once

#include <iosfwd>
#include <memory>
#include <string>

namespace lab1 {

class CantorSet;

/// @brief Элемент множества: атом (непустая строка) либо вложенное множество.
///
/// Класс владеет вложенным множеством, поэтому копирование глубокое.
class Element {
public:
    /// Создаёт атом. @throw std::invalid_argument если строка не является корректным атомом.
    // cppcheck-suppress noExplicitConstructor
    Element(const std::string& atom);
    /// Создаёт атом из C-строки. @throw std::invalid_argument если атом некорректен.
    // cppcheck-suppress noExplicitConstructor
    Element(const char* atom);
    /// Создаёт элемент-множество (копия переданного множества).
    // cppcheck-suppress noExplicitConstructor
    Element(const CantorSet& subset);

    Element(const Element& other);
    Element(Element&& other) noexcept;
    Element& operator=(const Element& other);
    Element& operator=(Element&& other) noexcept;
    ~Element();

    /// Разбирает элемент из текста: `{...}` — множество, иначе атом.
    /// @throw std::invalid_argument при пустом тексте или синтаксической ошибке.
    static Element parse(const std::string& text);

    /// Проверяет, что строка может быть именем атома (непустая, без пробелов и символов `{},`).
    static bool isValidAtom(const std::string& text);

    /// Возвращает true, если элемент — вложенное множество.
    bool isSet() const;
    /// Возвращает имя атома. @throw std::logic_error если элемент — множество.
    const std::string& atom() const;
    /// Возвращает вложенное множество. @throw std::logic_error если элемент — атом.
    const CantorSet& subset() const;

    /// Текстовое представление элемента.
    std::string toString() const;

    bool operator==(const Element& other) const;
    bool operator!=(const Element& other) const;

private:
    std::string atom_;
    std::unique_ptr<CantorSet> subset_;
};

/// Записывает элемент в поток.
std::ostream& operator<<(std::ostream& out, const Element& element);

}  // namespace lab1
