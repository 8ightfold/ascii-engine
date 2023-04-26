#ifndef PROJECT3_TEST_RESOURCE_LOCATOR_HPP
#define PROJECT3_TEST_RESOURCE_LOCATOR_HPP

#include <filesystem>
#include <api/core.hpp>

namespace fs = std::filesystem;

namespace api {
    struct ResourceLocator {
        static void initialize() NOEXCEPT;
        static fs::path get_file(const fs::path& filepath) NOEXCEPT;
        static fs::path get_resource_dir() NOEXCEPT;

    private:
        static fs::path& _resource_dir() NOEXCEPT;
    };
}

#endif //PROJECT3_TEST_RESOURCE_LOCATOR_HPP
