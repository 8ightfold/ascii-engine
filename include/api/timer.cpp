#include "timer.hpp"

namespace api {
    void Timer::start() noexcept {
        if(not _running) {
            _start = steady_clock::now();
            _running = true;
        }
    }

    void Timer::restart() noexcept {
        _start = steady_clock::now();
        _running = true;
    }

    void Timer::stop() noexcept {
        if(_running) {
            _end = steady_clock::now();
            _running = false;
        }
    }

    double Timer::elapsed_ms() noexcept {
        auto time = elapsed().count();
        return time / 1000000.0;
    }

    double Timer::elapsed_us() noexcept {
        auto time = elapsed().count();
        return time / 1000.0;
    }

    double Timer::elapsed_ns() noexcept {
        auto time = elapsed().count();
        return time / 1000.0;
    }

    ns_duration Timer::elapsed() noexcept {
        if(_running) _end = steady_clock::now();
        auto difference = _end - _start;
        return difference;
    }

    bool Timer::running() const noexcept {
        return _running;
    }
}