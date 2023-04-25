#include "core.hpp"

namespace render {
    void initialize_screen(api::Coords screen_size) NOEXCEPT {
        screen_coords = screen_size;
        S3L_resolutionX = screen_size.x;
        S3L_resolutionY = screen_size.y;
    }

    void initialize_buffer(api::Framebuffer<char>& framebuffer) NOEXCEPT {
        initialize_screen(framebuffer.get_coords());
        internal_buffer<char> = framebuffer;
    }

    void draw_pixel(int x, int y, uint8_t color, uint8_t luminance) {
        uint8_t c;
        if(color == 0) {
            long offset = (luminance / 255.0) * sizeof(world_color);
            c = world_color[offset];
        }
        else {
            luminance = luminance / sizeof(render::ColorGrade);
            luminance = (luminance > 15) ? 15 : luminance;
            c = render::color_grades[color - 1][luminance];
        }
        internal_buffer<char>->get_active_buffer()->set_value({ x, y }, c);
    }

    ObjectModel::ObjectModel(const fs::path& filepath) {
        load_model(filepath);
    }

    void ObjectModel::load_model(const fs::path& filepath) {

    }
}