#include <gtest/gtest.h>

#include <sstream>
#include <string>

#include "console_input.h"

TEST(ParseIntTest, AcceptsWholeNumbers) {
    int value = 0;
    EXPECT_TRUE(parseInt("42", value));
    EXPECT_EQ(value, 42);
    EXPECT_TRUE(parseInt("-7", value));
    EXPECT_EQ(value, -7);
}

TEST(ParseIntTest, RejectsEverythingElse) {
    const char* const bad[] = {"", "abc", "12abc", "1.5", "99999999999"};
    for (const char* text : bad) {
        SCOPED_TRACE(text);
        int value = 123;
        EXPECT_FALSE(parseInt(text, value));
        EXPECT_EQ(value, 123);
    }
}

TEST(ReadLineTest, PrintsPromptAndReadsLine) {
    std::istringstream in("hello\r\nsecond\n");
    std::ostringstream out;
    std::string line;
    EXPECT_TRUE(readLine(in, out, "name: ", line));
    EXPECT_EQ(line, "hello");
    EXPECT_TRUE(readLine(in, out, "next: ", line));
    EXPECT_EQ(line, "second");
    EXPECT_EQ(out.str(), "name: next: ");
}

TEST(ReadLineTest, ReturnsFalseAtEndOfInput) {
    std::istringstream in("");
    std::ostringstream out;
    std::string line;
    EXPECT_FALSE(readLine(in, out, "> ", line));
}
