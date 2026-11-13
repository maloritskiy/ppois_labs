/**
 * @file cantor_set.h
 * @brief Объявление класса CantorSet - неориентированного канторовского множества.
 */
#ifndef CANTOR_SET_H
#define CANTOR_SET_H

#include <iostream>
#include <string>

/**
 * @brief Неориентированное канторовское множество.
 *
 * Элементы не повторяются, их порядок не важен. Элементом множества может быть
 * строка (атом) или другое множество. Элементы хранятся в динамическом массиве,
 * поэтому у класса есть конструктор копирования, оператор присваивания и деструктор.
 * Вложенные множества копируются глубоко.
 */
class CantorSet {
public:
    /// Наибольшая мощность множества, для которого можно построить булеан
    static constexpr int kMaxPowerSetSize = 16;

    /// @brief Создаёт пустое множество.
    CantorSet();

    /// @brief Конструктор копирования, вложенные множества копируются глубоко.
    /// @param other Копируемое множество.
    CantorSet(const CantorSet& other);

    /// @brief Оператор присваивания копированием.
    /// @param other Присваиваемое множество.
    /// @return Ссылка на текущее множество.
    CantorSet& operator=(const CantorSet& other);

    /// @brief Освобождает память, занятую элементами.
    ~CantorSet();

    /**
     * @brief Строит множество из строки вида {a, b, {c}, {}}.
     * @param text Текст множества, пробелы между элементами игнорируются.
     * @return Построенное множество, повторяющиеся элементы учитываются один раз.
     * @throws std::invalid_argument Если строка записана неверно или вложенность больше 100.
     */
    static CantorSet fromString(const std::string& text);

    /// @brief Записывает множество в виде строки, например {a, {b}}.
    /// @return Текстовое представление множества.
    std::string toString() const;

    /// @brief Проверяет, что множество пустое.
    /// @return true, если элементов нет.
    bool isEmpty() const;

    /// @brief Возвращает мощность множества.
    /// @return Количество элементов.
    int size() const;

    /**
     * @brief Добавляет элемент-строку.
     * @param atom Имя элемента: не пустое, без пробелов и символов { } и запятой.
     * @return true, если элемент добавлен, false, если он уже был.
     * @throws std::invalid_argument Если имя недопустимо.
     */
    bool add(const std::string& atom);

    /// @brief Добавляет элемент-множество.
    /// @param subset Добавляемое множество, в множество кладётся его копия.
    /// @return true, если элемент добавлен, false, если он уже был.
    bool add(const CantorSet& subset);

    /// @brief Удаляет элемент-строку.
    /// @param atom Имя элемента.
    /// @return true, если элемент был и удалён.
    bool remove(const std::string& atom);

    /// @brief Удаляет элемент-множество.
    /// @param subset Удаляемое множество.
    /// @return true, если элемент был и удалён.
    bool remove(const CantorSet& subset);

    /// @brief Проверяет принадлежность элемента-строки множеству.
    /// @param atom Имя элемента.
    /// @return true, если элемент входит в множество.
    bool operator[](const std::string& atom) const;

    /// @brief Проверяет принадлежность элемента-множества.
    /// @param subset Искомое множество.
    /// @return true, если такой элемент входит в множество.
    bool operator[](const CantorSet& subset) const;

    /// @brief Объединение: добавляет элементы другого множества.
    /// @param other Второе множество.
    /// @return Ссылка на текущее множество.
    CantorSet& operator+=(const CantorSet& other);

    /// @brief Пересечение: оставляет только общие элементы.
    /// @param other Второе множество.
    /// @return Ссылка на текущее множество.
    CantorSet& operator*=(const CantorSet& other);

    /// @brief Разность: убирает элементы другого множества.
    /// @param other Второе множество.
    /// @return Ссылка на текущее множество.
    CantorSet& operator-=(const CantorSet& other);

    /**
     * @brief Строит булеан - множество всех подмножеств.
     * @return Множество из 2 в степени size() элементов, каждый из которых - подмножество.
     * @throws std::length_error Если в множестве больше kMaxPowerSetSize элементов.
     */
    CantorSet powerSet() const;

    /// @brief Сравнение множеств на равенство, порядок элементов не важен.
    /// @param other Второе множество.
    /// @return true, если множества состоят из одинаковых элементов.
    bool operator==(const CantorSet& other) const;

    /// @brief Сравнение множеств на неравенство.
    /// @param other Второе множество.
    /// @return true, если множества различаются.
    bool operator!=(const CantorSet& other) const;

private:
    /// Элемент множества: если subset равен nullptr, элемент - строка atom, иначе - вложенное множество
    struct Element {
        std::string atom;
        CantorSet* subset;
        Element() : subset(nullptr) {}
    };

    Element* items_;  ///< Массив элементов
    int count_;       ///< Сколько элементов занято
    int capacity_;    ///< Размер массива

    void clear();
    void copyItems(const CantorSet& other);
    void grow();
    void append(const std::string& atom, const CantorSet* subset);
    void removeAt(int index);
    int findAtom(const std::string& atom) const;
    int findSubset(const CantorSet& subset) const;
    bool containsItem(const Element& item) const;
    void keepItems(const CantorSet& other, bool keepCommon);
    CantorSet subsetByMask(int mask) const;
};

/// @brief Объединение двух множеств.
/// @return Новое множество, исходные не меняются.
CantorSet operator+(CantorSet left, const CantorSet& right);

/// @brief Пересечение двух множеств.
/// @return Новое множество, исходные не меняются.
CantorSet operator*(CantorSet left, const CantorSet& right);

/// @brief Разность двух множеств.
/// @return Новое множество, исходные не меняются.
CantorSet operator-(CantorSet left, const CantorSet& right);

/// @brief Записывает множество в поток.
/// @param out Выходной поток.
/// @param set Записываемое множество.
/// @return Выходной поток.
std::ostream& operator<<(std::ostream& out, const CantorSet& set);

/// @brief Читает одну строку из потока и разбирает её как множество.
/// @param in Входной поток.
/// @param set Множество, в которое записывается результат.
/// @return Входной поток; при ошибке в строке выставляется failbit, а множество не меняется.
std::istream& operator>>(std::istream& in, CantorSet& set);

#endif
