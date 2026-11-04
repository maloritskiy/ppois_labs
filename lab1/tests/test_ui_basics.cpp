#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>

#include "test_support.hpp"
#include "ui/console_io.hpp"
#include "ui/menu.hpp"

using lab1::testing_support::contains;
using lab1::ui::ConsoleIo;
using lab1::ui::Menu;

TEST(ParseIntegerTest, AcceptsWholeNumbers) {
    EXPECT_EQ(lab1::ui::parseInteger("42"), 42);
    EXPECT_EQ(lab1::ui::parseInteger("-7"), -7);
}

TEST(ParseIntegerTest, RejectsEverythingElse) {
    for (const std::string text : {"", "abc", "12abc", "1.5", "99999999999"}) {
        SCOPED_TRACE(text);
        EXPECT_FALSE(lab1::ui::parseInteger(text).has_value());
    }
}

TEST(ParseIntegerTest, RequireIntegerThrowsOnBadText) {
    EXPECT_EQ(lab1::ui::requireInteger("5"), 5);
    EXPECT_THROW(lab1::ui::requireInteger("five"), std::invalid_argument);
}

TEST(ConsoleIoTest, PromptPrintsMessageAndReadsLine) {
    std::istringstream in("hello\r\nsecond\n");
    std::ostringstream out;
    ConsoleIo io(in, out);
    EXPECT_EQ(io.prompt("name: "), "hello");
    EXPECT_EQ(io.prompt("next: "), "second");
    EXPECT_EQ(out.str(), "name: next: ");
}

TEST(ConsoleIoTest, PromptReturnsNothingAtEndOfInput) {
    std::istringstream in("");
    std::ostringstream out;
    ConsoleIo io(in, out);
    EXPECT_FALSE(io.prompt("> ").has_value());
}

TEST(ConsoleIoTest, WriteLineAppendsNewline) {
    std::istringstream in("");
    std::ostringstream out;
    ConsoleIo io(in, out);
    io.writeLine("line");
    EXPECT_EQ(out.str(), "line\n");
}

TEST(MenuTest, RunsChosenItemUntilExit) {
    std::istringstream in("1\n1\n2\n0\n");
    std::ostringstream out;
    ConsoleIo io(in, out);
    Menu menu(io);
    int firstCalls = 0;
    int secondCalls = 0;
    menu.addItem("first", [&firstCalls] { ++firstCalls; });
    menu.addItem("second", [&secondCalls] { ++secondCalls; });
    menu.run();
    EXPECT_EQ(firstCalls, 2);
    EXPECT_EQ(secondCalls, 1);
    EXPECT_TRUE(contains(out.str(), "1. first"));
    EXPECT_TRUE(contains(out.str(), "2. second"));
    EXPECT_TRUE(contains(out.str(), "0. Выход"));
}

TEST(MenuTest, InvalidChoicesAreReported) {
    std::istringstream in("abc\n7\n-1\n0\n");
    std::ostringstream out;
    ConsoleIo io(in, out);
    Menu menu(io);
    menu.addItem("only", [] {});
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Нет такого пункта меню"));
}

TEST(MenuTest, StopsAtEndOfInput) {
    std::istringstream in("");
    std::ostringstream out;
    ConsoleIo io(in, out);
    Menu menu(io);
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Выберите пункт"));
}

TEST(MenuTest, ActionErrorsAreReportedAndMenuContinues) {
    std::istringstream in("1\n2\n0\n");
    std::ostringstream out;
    ConsoleIo io(in, out);
    Menu menu(io);
    bool secondRan = false;
    menu.addItem("fails", [] { throw std::runtime_error("boom"); });
    menu.addItem("works", [&secondRan] { secondRan = true; });
    menu.run();
    EXPECT_TRUE(contains(out.str(), "Ошибка: boom"));
    EXPECT_TRUE(secondRan);
}

TEST(MenuTest, StatusIsPrintedBeforeEveryMenu) {
    std::istringstream in("9\n0\n");
    std::ostringstream out;
    ConsoleIo io(in, out);
    Menu menu(io);
    menu.setStatusProvider([] { return std::string("STATUS\n"); });
    menu.run();
    const std::string text = out.str();
    const std::size_t first = text.find("STATUS");
    ASSERT_NE(first, std::string::npos);
    EXPECT_NE(text.find("STATUS", first + 1), std::string::npos);
}
