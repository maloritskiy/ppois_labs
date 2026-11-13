/**
 * @file console_input.h
 * @brief Вспомогательные функции ввода для консольных меню.
 */
#ifndef CONSOLE_INPUT_H
#define CONSOLE_INPUT_H

#include <iostream>
#include <string>

/**
 * @brief Выводит приглашение и читает строку.
 * @param in Входной поток.
 * @param out Выходной поток для приглашения.
 * @param prompt Текст приглашения.
 * @param line Сюда записывается прочитанная строка без символа возврата каретки.
 * @return false, если ввод закончился.
 */
bool readLine(std::istream& in, std::ostream& out, const std::string& prompt, std::string& line);

/**
 * @brief Переводит строку в целое число.
 * @param text Текст, в котором должно быть только число.
 * @param value Сюда записывается число, если разбор удался.
 * @return false, если текст не число или оно не помещается в int.
 */
bool parseInt(const std::string& text, int& value);

#endif
