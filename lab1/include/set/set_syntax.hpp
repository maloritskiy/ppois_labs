#pragma once

/// @file
/// @brief Синтаксические константы текстового представления множества.

namespace lab1::syntax {

/// Символ начала множества.
constexpr char kOpenBrace = '{';
/// Символ конца множества.
constexpr char kCloseBrace = '}';
/// Разделитель элементов при разборе строки.
constexpr char kSeparator = ',';
/// Символы, которые не могут входить в имя атома.
constexpr const char* kReservedChars = "{},";
/// Разделитель элементов при выводе множества.
constexpr const char* kElementDelimiter = ", ";

}  // namespace lab1::syntax
