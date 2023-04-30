#include "core.hpp"

namespace ui {
    void Rect::set_position(int x, int y) {
        zero_orient();
        api::Coords offset { x, y };
        top_left += offset;
        bottom_right += offset;
    }

    void Rect::set_size(int x, int y) {
        bottom_right = api::Coords { x, y } + top_left;
    }

    void Rect::set_position(api::Coords c) { set_position(c.x, c.y); }
    void Rect::set_size(api::Coords c) { set_size(c.x, c.y); }

    void Rect::shrink(int s) {
        api::Coords new_top = top_left + s;
        api::Coords new_bottom = bottom_right - s;
        if((new_top.x <= new_bottom.x) and (new_top.y <= new_bottom.y)) {
            top_left = new_top;
            bottom_right = new_bottom;
        }
    }

    void Rect::zero_orient() {
        api::Coords offset = top_left;
        top_left -= offset;
        bottom_right -= offset;
    }

    bool Rect::intersects(api::Coords c) CNOEXCEPT {
        bool intersects_x = (c.x >= top_left.x) and (c.x <= bottom_right.x);
        bool intersects_y = (c.y > top_left.y) and (c.y < bottom_right.y);
        return intersects_x and intersects_y;
    }

    api::Coords Rect::size() CNOEXCEPT {
        return bottom_right - top_left;
    }

    int Rect::area() CNOEXCEPT {
        api::Coords isize = size();
        return isize.x * isize.y;
    }


    std::size_t text_occurrences(const std::string& str, char delim) {
        return std::count(str.begin(), str.end(), delim);
    }

    std::deque<std::string> split_string(const std::string& str, char delim) {
        std::size_t occurrences = text_occurrences(str, delim);
        if(not occurrences) return { str };

        std::size_t start_pos = 0;
        std::size_t end_pos;
        const std::size_t end = std::string::npos;
        std::deque<std::string> substrings;

        while((end_pos = str.find(delim, start_pos)) != end) {
            std::string substr = str.substr(start_pos, end_pos - start_pos);
            substrings.emplace_back(std::move(substr));
            start_pos = end_pos + 1;
        }

        return substrings;
    }
}
