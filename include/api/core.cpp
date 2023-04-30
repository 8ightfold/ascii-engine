#include "core.hpp"

#include <algorithm>
#include <thread>
#include <locale>
#include <strsafe.h>

namespace api {
    Coords::operator POINT() CNOEXCEPT {
        return { x, y };
    }

    Coords::operator COORD() CNOEXCEPT {
        return { (SHORT)x, (SHORT)y };
    }

    Coords::operator iVec2() CNOEXCEPT {
        return { x, y };
    }

    Coords::operator dVec2() CNOEXCEPT {
        return { (double)x, (double)y };
    }

    Coords& Coords::operator=(POINT p) NOEXCEPT {
        x = p.x;
        y = p.y;
        return *this;
    }

    Coords& Coords::operator=(COORD c) NOEXCEPT {
        x = c.X;
        y = c.Y;
        return *this;
    }

    Coords& Coords::operator=(iVec2 p) NOEXCEPT {
        x = p.x;
        y = p.y;
        return *this;
    }

    Coords& Coords::operator=(dVec2 p) NOEXCEPT {
        x = int(p.x);
        y = int(p.y);
        return *this;
    }

    int Coords::area() CNOEXCEPT {
        return (x * y);
    }

    Coords Coords::operator+(int i) CNOEXCEPT {
        return { x + i, y + i };
    }

    Coords Coords::operator-(int i) CNOEXCEPT {
        return { x - i, y - i };
    }

    Coords Coords::operator+(Coords c) CNOEXCEPT {
        return { x + c.x, y + c.y };
    }

    Coords Coords::operator-(Coords c) CNOEXCEPT {
        return { x - c.x, y - c.y };
    }

    Coords Coords::operator*(Coords c) CNOEXCEPT {
        return { x * c.x, y * c.y };
    }

    Coords& Coords::operator+=(Coords c) NOEXCEPT {
        x += c.x;
        y += c.y;
        return *this;
    }

    Coords& Coords::operator-=(Coords c) NOEXCEPT {
        x -= c.x;
        y -= c.y;
        return *this;
    }

    Coords& Coords::operator/=(int i) NOEXCEPT {
        x /= i;
        y /= i;
        return *this;
    }

    Coords Coords::operator/(int i) CNOEXCEPT {
        return { x / i, y / i };
    }

    Coords& Coords::operator/=(double d) NOEXCEPT {
        x = static_cast<int>(x / d);
        y = static_cast<int>(y / d);
        return *this;
    }

    Coords& Coords::operator/=(Coords c) NOEXCEPT {
        x /= c.x;
        y /= c.y;
        return *this;
    }

    Coords& Coords::operator/=(dVec2 d) NOEXCEPT {
        x = static_cast<int>(x / d.x);
        y = static_cast<int>(y / d.y);
        return *this;
    }


    void assertion_impl(const char* filename, const char* func, int line, bool condition, const std::string& err) {
        if(not condition) fatal_error(filename, func, line, err);
    }

    NORETURN void waiting_exit(int code) {
        std::cout << "Press [esc/return] to continue..." << std::endl;
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
        while(not (WIN_PRESSED(VK_ESCAPE) or WIN_PRESSED(VK_RETURN))) {}
        std::exit(code);
    }

    static std::string parse_pretty_func(std::string str) NOEXCEPT /* NOLINT */ {
    #if COMPILER_FUNCTION_CLASSIC == 0
        std::size_t location = str.rfind(')');
        if (location == std::string::npos) {
            return str;
        } else {
            return str.substr(0, location + 1);
        }
    #endif
    }

    NORETURN void fatal_error(const char* filename, const char* func, int line, const std::string& err) {
        if(on_error) on_error();
        DEBUG_ONLY(
                std::cout << filename << ":" << line << '\n';
                std::cout << "In '" << parse_pretty_func(func) << "'\n";
        )
        std::cout << "FATAL: " << err << '\n';
        waiting_exit(-1);
    }


    static std::string narrow(LPCSTR buf, MAYBE_UNUSED std::size_t out_size) {
        return { buf };
    }

    static std::string narrow(LPWSTR buf, std::size_t out_size) {
        std::string new_str(out_size + 1, '\0');
        std::size_t num_converted;
        wcstombs_s(&num_converted, new_str.data(), new_str.size(), buf, _TRUNCATE);
        return new_str;
    }

    void handle_last_error(const char* filename, int line, LPCTSTR function, bool exit_on_error) {
        LPVOID msg_buf;
        LPTSTR out_buf;
        DWORD err_code = GetLastError();

        FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  // NOLINT
                (LPTSTR) &msg_buf, 0, NULL);  // NOLINT

        out_buf = (LPTSTR) LocalAlloc(LMEM_ZEROINIT,
                (lstrlen((LPCTSTR)msg_buf) + lstrlen(function) + 40) * sizeof(TCHAR));

        std::size_t out_size = LocalSize(out_buf) / sizeof(TCHAR);
        StringCchPrintf((LPTSTR)out_buf, out_size,
                TEXT("OS error: \"%s\" failed with error %d; %s"),
                        function, err_code, msg_buf);

        std::string throw_buf = narrow(out_buf, lstrlen(out_buf));
        LocalFree(msg_buf);
        LocalFree(out_buf);

        if(on_error) on_error();
        DEBUG_ONLY( std::cout << filename << ":" << line << '\n'; )
        std::cout << throw_buf << std::endl;
        if(exit_on_error) waiting_exit(-2);
    }
}
