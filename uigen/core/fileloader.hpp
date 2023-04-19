#ifndef PROJECT3_TEST_FILELOADER_HPP
#define PROJECT3_TEST_FILELOADER_HPP

#include <exception>
#include <filesystem>
#include <string>
#include <utility>

namespace fs = std::filesystem;

namespace parser {
    std::string narrow(const char* str);
    std::string narrow(const wchar_t* str);

    struct FailedOpening : std::exception {
        [[nodiscard]] const char* what() const noexcept override { return local.c_str(); }
        explicit FailedOpening(fs::path fp) : filepath(std::move(fp)), local(narrow(filepath.c_str())) {}

    private:
        fs::path filepath;
        std::string local;
    };

    void load_file(const fs::path& filepath, std::string& internal_file_data);
    void load_file(const fs::path& filepath, std::string* internal_file_data);
}

#endif //PROJECT3_TEST_FILELOADER_HPP
