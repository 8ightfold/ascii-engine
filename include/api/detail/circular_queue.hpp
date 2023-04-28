#ifndef PROJECT3_TEST_CIRCULAR_QUEUE_HPP
#define PROJECT3_TEST_CIRCULAR_QUEUE_HPP

#include <cstddef>
#include <span>
#include <config.hpp>

namespace api {
    template <typename T>
    struct CircularQueue {
        explicit CircularQueue(std::size_t size) : _size(size) {
            _data = new T[size] {};
        }

        CircularQueue(const CircularQueue&) = delete;
        CircularQueue(CircularQueue&&) = delete;

        ~CircularQueue() {
            delete[] _data;
        }

        void apply(auto&& functor) NOEXCEPT {
            auto data_span = data();
            for(auto& t : data_span)
                functor(t);
        }

        T& next() NOEXCEPT {
            if(_location >= _size)
                _location = 0;
            T& t = _data[_location];
            ++_location;
            return t;
        }

        T& peek() NOEXCEPT {
            if(_location >= _size)
                _location = 0;
            T& t = _data[_location];
            return t;
        }

        NODISCARD std::span<T> data() CNOEXCEPT {
            return { _data, _size };
        }

    private:
        std::size_t _size;
        std::size_t _location = 0;
        T* _data = nullptr;
    };
}

#endif //PROJECT3_TEST_CIRCULAR_QUEUE_HPP
