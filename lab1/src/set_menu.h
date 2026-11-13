/**
 * @file set_menu.h
 * @brief Объявление консольного меню для работы с множествами.
 */
#ifndef SET_MENU_H
#define SET_MENU_H

#include <iostream>
#include <string>

#include "cantor_set.h"

/**
 * @brief Консольное меню для работы с двумя множествами A и B.
 *
 * Меню зависит от класса CantorSet, но сам класс о меню ничего не знает.
 * Потоки ввода и вывода передаются снаружи, поэтому меню можно проверять в тестах.
 * Ошибки действий перехватываются и выводятся как сообщение, меню продолжает работу.
 */
class SetMenu {
public:
    /// @brief Создаёт меню с двумя пустыми множествами.
    /// @param in Поток ввода.
    /// @param out Поток вывода.
    SetMenu(std::istream& in, std::ostream& out);

    /// @brief Показывает меню и выполняет выбранные пункты, пока не выбран выход или не закончился ввод.
    void run();

private:
    /// Пункты меню, номер пункта равен значению
    enum MenuItem {
        kExit = 0,
        kInputSetA,
        kInputSetB,
        kAddElement,
        kRemoveElement,
        kCheckElement,
        kShowSize,
        kUnion,
        kIntersection,
        kDifference,
        kPowerSet,
        kCompare
    };

    std::istream& in_;  ///< Поток ввода
    std::ostream& out_; ///< Поток вывода
    CantorSet setA_;    ///< Множество A
    CantorSet setB_;    ///< Множество B

    void printMenu();
    bool handleChoice(const std::string& line);
    void runAction(int choice);
    void inputSet(const std::string& name, CantorSet& target);
    void addElement();
    void removeElement();
    void checkElement();
    void showSize();
    void compareSets();
    void show(const std::string& label, const CantorSet& value);
    bool askElement(std::string& text);
};

#endif
