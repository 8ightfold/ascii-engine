#ifndef PROJECT3_TEST_RENDER_CORE_HPP
#define PROJECT3_TEST_RENDER_CORE_HPP

/*
    tinyphysicsengine and small3dlib are NOT made by me.
    They were both uploaded by jordan4ibanez.
    The opinions/views of drummyfish do not reflect my own, he just makes neat software
    You can find the repo here: https://github.com/jordan4ibanez/tinyphysicsengine
 */

#include <filesystem>

#include <render/tinyphysicsengine.hpp>
#include <render/small3dlib.hpp>

#include <api/core.hpp>
#include <api/framebuffer.hpp>

namespace fs = std::filesystem;

namespace TPE {
    enum struct BodyFlag {
        eDeactivated        = TPE_BODY_FLAG_DEACTIVATED,
        eNonRotating        = TPE_BODY_FLAG_NONROTATING,
        eDisabled           = TPE_BODY_FLAG_DISABLED,
        eSoftbody           = TPE_BODY_FLAG_SOFT,
        eSimpleConnection   = TPE_BODY_FLAG_SIMPLE_CONN,
        eNoDeactivate       = TPE_BODY_FLAG_ALWAYS_ACTIVE,
    };
}

namespace render {
    inline api::Coords screen_coords {};

    template <typename T>
    inline api::ObjectBinding<api::Framebuffer<T>> internal_buffer;

    /**
     * Color palette type (essentially 16 bit color). Graded from darkest to lightest.
     */
    using ColorGrade = unsigned char[16];

    /**
     * The defined world color palette. Provides more color definition than other palettes
     * as the detail is more important. Color 0 is used for this.
     */
    inline constexpr char world_color[] = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

    /**
     * The defined color palettes. When adding custom grades, ensure you supply all 16 values.
     * Values are indexed in the range [1, 255]. Color 0 is the world color.
     */
    inline constexpr ColorGrade color_grades[] {
            { 255, 176, 176, 176, 176, 177, 177, 177, 177, 177, 178, 178, 178, 178, 219, 219 }, // " ░░░░▒▒▒▒▒▓▓▓▓██"
            { 255, '.', '.', ':', ':', '-', '=', '+', '*', '&', '#', '%', '@', '@', '$', '$' }, // " ..::-=+*&#%@@$$"
            { 255, 191, 187, 179, 179, 197, 197, 240, '#', '#', 186, 186, 215, 215, 206, 206 }, // " ┐╗││┼┼≡##║║╫╫╬╬"
    };

    void initialize_screen(api::Coords screen_size) NOEXCEPT;

    void initialize_buffer(api::Framebuffer<char>& framebuffer) NOEXCEPT;

    /**
     * Draws a pixel to the current screen buffer. Only supports ASCII color at the moment.
     * @param color The color gradient. 0 is used for the world color, all others are from color_grades.
     * @param luminance A value in the range [0, 255] (this will be scaled appropriately)
     */
    void draw_pixel(int x, int y, uint8_t color, uint8_t luminance);


    struct ObjectModel {
        ObjectModel() = default;
        ObjectModel(const fs::path& filepath);

        void load_model(const fs::path& filepath);
    private:
        std::string _name;
        std::vector<S3L_Unit> _vertices;
        std::vector<S3L_Index> _faces;
    };
}

#endif //PROJECT3_TEST_RENDER_CORE_HPP
