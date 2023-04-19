#ifndef PROJECT3_TEST_FILESYSTEM_HPP
#define PROJECT3_TEST_FILESYSTEM_HPP

#include <iostream>
#include <memory>
#include <stack>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <core.hpp>

namespace parser {
    struct File {
        explicit File(fs::path filepath);
        [[nodiscard]] std::string_view get_view() const;
        [[nodiscard]] fs::path get_filename() const;

    private:
        fs::path _filepath;
        std::unique_ptr<std::string> _filedata;
    };

    struct FileTree {
        explicit FileTree(fs::path dir);
        FileTree(fs::path dir, FileTree* root);
        ~FileTree();

        File& add_file(const fs::path& filename);
        FileTree& add_dir(const std::string& str);
        FileTree* operator->();

        [[nodiscard]] bool is_root() const;

    private:
        fs::path _self_dir;
        FileTree* _root = nullptr;
        std::unordered_map<std::string, FileTree*> _subdirs;
        std::unordered_map<std::string, File> _files;
    };
}

#endif //PROJECT3_TEST_FILESYSTEM_HPP
