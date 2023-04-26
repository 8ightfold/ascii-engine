#include "resource_locator.hpp"

namespace api {
    void ResourceLocator::initialize() NOEXCEPT {
        auto& resource_dir = _resource_dir();
        fs::path current_dir = fs::current_path();
        const fs::path top_level = current_dir.root_path();

        while(resource_dir.empty()) {
            for(const auto& entry : fs::directory_iterator{ current_dir }) {
                if(entry.is_directory() && entry.path().filename() == "resources") {
                    resource_dir = entry.path();
                    break;
                }
            }

            if(current_dir == top_level) {
                FATAL("Resource directory could not be located.");
            }

            current_dir = current_dir.parent_path();
        }
    }

    fs::path ResourceLocator::get_file(const fs::path& filepath) NOEXCEPT {
        return get_resource_dir() / filepath;
    }

    fs::path ResourceLocator::get_resource_dir() NOEXCEPT {
        return _resource_dir();
    }

    fs::path& ResourceLocator::_resource_dir() NOEXCEPT {
        static fs::path resource_dir = {};
        return resource_dir;
    }
}
