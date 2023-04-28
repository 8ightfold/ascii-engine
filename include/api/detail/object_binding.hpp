#ifndef PROJECT3_TEST_OBJECT_BINDING_HPP
#define PROJECT3_TEST_OBJECT_BINDING_HPP

#include <memory>
#include <config.hpp>

namespace api {
    template <typename T>
    struct ObjectBinding {
        ObjectBinding() = default;
        ObjectBinding(T& data) : _data(std::addressof(data)) {}

        ObjectBinding& operator=(T& data) { _data = std::addressof(data); return *this; }

        T* operator->() NOEXCEPT { return _data; }
        const T* operator->() CNOEXCEPT { return _data; }
        T& operator*() NOEXCEPT { return *_data; }
        const T& operator*() CNOEXCEPT { return *_data; }
        NODISCARD bool active() CNOEXCEPT { return _data; }

    private:
        T* _data = nullptr;
    };
}

#endif //PROJECT3_TEST_OBJECT_BINDING_HPP
