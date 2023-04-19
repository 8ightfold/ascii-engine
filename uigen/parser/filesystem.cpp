#include "filesystem.hpp"

namespace parser {
    File::File(fs::path filepath) : _filepath(std::move(filepath)) {
        _filedata = std::make_unique<std::string>();
        load_file(_filepath, _filedata.get());
    }

    std::string_view File::get_view() const {
        return { *_filedata };
    }

    fs::path File::get_filename() const {
        return _filepath.filename();
    }


    FileTree::FileTree(fs::path dir)
    : _self_dir(std::move(dir)) {}

    FileTree::FileTree(fs::path dir, FileTree* root)
    : _self_dir(std::move(dir)), _root(root) {}

    FileTree::~FileTree() {
        for(auto&& [name, tree] : _subdirs)
            delete tree;
    }

    File& FileTree::add_file(const fs::path& filename) {
        fs::path new_file = _self_dir / filename;
        std::string filename_str = filename.string();

        if(fs::exists(new_file)) {
            if(not _files.contains(filename_str)) {
                _files.emplace(filename_str, new_file);
            }
            return _files.at(filename_str);
        }
        else throw FailedOpening(new_file);
    }

    FileTree& FileTree::add_dir(const std::string& str) {
        if(not _subdirs.contains(str)) {
            FileTree* new_tree = nullptr;
            FileTree* root = is_root() ? this : _root;

            new_tree = new FileTree(_self_dir / str, root);
            _subdirs[str] = new_tree;
        }

        return *_subdirs[str];
    }

    FileTree* FileTree::operator->() {
        return is_root() ? this : _root;
    }

    bool FileTree::is_root() const {
        return (not _root);
    }
}
