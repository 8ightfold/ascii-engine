#include "resource_locator.hpp"

#define HIDE_FILE(cond, attributes) (cond) ? (attributes | FILE_ATTRIBUTE_HIDDEN) : (attributes & ~FILE_ATTRIBUTE_HIDDEN)

namespace api {
    void ResourceLocator::initialize() NOEXCEPT {
        auto& resource_dir = _resource_dir();
        resource_dir = get_directory("resources");
        assertion(not resource_dir.empty(), "Resource directory could not be located.");
    }

    fs::path ResourceLocator::get_directory(const std::string& dirname) NOEXCEPT {
        fs::path folder;
        fs::path current_dir = fs::current_path();
        const fs::path top_level = current_dir.root_path();

        while(folder.empty()) {
            for(const auto& entry : fs::directory_iterator{ current_dir }) {
                if(entry.is_directory() && entry.path().filename() == dirname) {
                    folder = entry.path();
                    break;
                }
            }

            if(current_dir == top_level) break;
            current_dir = current_dir.parent_path();
        }

        return folder;
    }

    fs::path ResourceLocator::get_core_dir() NOEXCEPT {
        return _resource_dir().parent_path();
    }

    fs::path ResourceLocator::get_file(const fs::path& filepath) NOEXCEPT {
        return get_resource_dir() / filepath;
    }

    void ResourceLocator::set_hidden(const fs::path& filepath, bool hidden) NOEXCEPT {
        const auto* raw_path = filepath.c_str();
        const DWORD file_attributes = $invoke_winapi(GetFileAttributes, INVALID_FILE_ATTRIBUTES)(raw_path);

        DWORD new_file_attributes = HIDE_FILE(hidden, file_attributes);
        $invoke_winapi(SetFileAttributes, 0)(raw_path, new_file_attributes);
    }

    fs::path ResourceLocator::get_resource_dir() NOEXCEPT {
        debug_assert(not _resource_dir().empty());
        return _resource_dir();
    }

    fs::path& ResourceLocator::_resource_dir() NOEXCEPT {
        static fs::path resource_dir = {};
        return resource_dir;
    }
}
