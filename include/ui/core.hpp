#ifndef PROJECT3_TEST_UI_CORE_HPP
#define PROJECT3_TEST_UI_CORE_HPP

#include <api/core.hpp>
#include <api/framebuffer.hpp>
#include <ui/strided_memcpy.hpp>

namespace ui {
    struct Rect {
        void set_position(int x, int y);
        void set_size(int x, int y);
        void set_position(api::Coords c);
        void set_size(api::Coords c);
        void zero_orient();
        NODISCARD api::Coords size() CNOEXCEPT;
        NODISCARD int area() CNOEXCEPT;

        api::Coords top_left = { 0, 0 };
        api::Coords bottom_right = { 0, 0 };
    };

    struct UIColor {
        NODISCARD constexpr operator api::iVec3() CNOEXCEPT { // NOLINT
            return { palette, luminance, alpha };
        }

        int palette = 1;
        int luminance = 255;
        int alpha = 255;
    };

    template <typename T>
    void box_copy(api::Framebuffer<T>& fb, Rect box_size, const std::vector<T>& box_data) {
        T* dst = fb.get_active_buffer()->raw_data();
        const T* src = box_data.data();
        const api::Coords dst_size = fb->get_coords();
        const api::Coords dst_offset = box_size.top_left;
        const api::Coords src_size = box_size.size();
        static constexpr std::size_t size = sizeof(T);

        if(not src_size.area()) return;
        strided_memcpy(dst, src, dst_size, dst_offset, src_size, size);
    }
}

#endif //PROJECT3_TEST_UI_CORE_HPP
