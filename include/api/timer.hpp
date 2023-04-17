#ifndef PROJECT3_TEST_TIMER_HPP
#define PROJECT3_TEST_TIMER_HPP

#include <chrono>
#include <thread>

namespace api {
    using time_point = std::chrono::time_point<std::chrono::steady_clock>;
    using steady_clock = std::chrono::steady_clock;

    using ms_duration = std::chrono::duration<double, std::milli>;
    using us_duration = std::chrono::duration<double, std::micro>;
    using ns_duration = std::chrono::duration<double, std::nano>;

    struct Timer {
        void start() noexcept;
        void restart() noexcept;
        void stop() noexcept;

        double elapsed_ms() noexcept;
        double elapsed_us() noexcept;
        double elapsed_ns() noexcept;

        ns_duration elapsed() noexcept;

        bool running() const noexcept;

    protected:
        time_point _start {}, _end {};
        bool _running = false;
    };
}

#endif //PROJECT3_TEST_TIMER_HPP
