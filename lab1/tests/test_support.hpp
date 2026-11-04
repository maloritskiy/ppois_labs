#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <system_error>

namespace lab1::testing_support {

/// Временный файл, удаляемый при выходе из области видимости.
class TempFile {
public:
    explicit TempFile(const std::string& name)
        : path_(std::filesystem::temp_directory_path() / ("lab1_test_" + name)) {}

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    ~TempFile() {
        std::error_code ignored;
        std::filesystem::remove(path_, ignored);
    }

    std::string path() const { return path_.string(); }

    void write(const std::string& text) const {
        std::ofstream file(path_);
        file << text;
    }

    std::string read() const {
        std::ifstream file(path_);
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

private:
    std::filesystem::path path_;
};

/// Запускает консольный интерфейс на заданном входном тексте и возвращает весь вывод.
template <typename Cli>
std::string runCli(const std::string& input) {
    std::istringstream in(input);
    std::ostringstream out;
    Cli cli(in, out);
    cli.run();
    return out.str();
}

inline bool contains(const std::string& text, const std::string& part) {
    return text.find(part) != std::string::npos;
}

}  // namespace lab1::testing_support
