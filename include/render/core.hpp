#ifndef PROJECT3_TEST_RENDER_CORE_HPP
#define PROJECT3_TEST_RENDER_CORE_HPP

/*
    tinyphysicsengine and small3dlib are NOT made by me.
    They were both uploaded by jordan4ibanez.
    The opinions/views of drummyfish do not reflect my own, he just makes neat software
    You can find the repo here: https://github.com/jordan4ibanez/tinyphysicsengine
 */

#include <filesystem>

#define TPE_LOG puts
#define TPE_RESHAPE_ITERATIONS 4
#include <cstdio>
#include <render/tinyphysicsengine.h>

#define S3L_USE_WIDER_TYPES 1
#define S3L_MAX_PIXELS 3686400
#define S3L_PIXEL_FUNCTION S3L_draw_pixel
#define S3L_PERSPECTIVE_CORRECTION 2
#define SCALE_3D_RENDERING 1
#define S3L_NEAR (S3L_FRACTIONS_PER_UNIT / (4 * SCALE_3D_RENDERING))
#define S3L_Z_BUFFER 1
#define S3L_NEAR_CROSS_STRATEGY 3
#include <render/small3dlib.h>

#include <api/core.hpp>
#include <api/framebuffer.hpp>

namespace fs = std::filesystem;

namespace render {
    enum struct BodyFlag {
        eDeactivated        = TPE_BODY_FLAG_DEACTIVATED,
        eNonRotating        = TPE_BODY_FLAG_NONROTATING,
        eDisabled           = TPE_BODY_FLAG_DISABLED,
        eSoftbody           = TPE_BODY_FLAG_SOFT,
        eSimpleConnection   = TPE_BODY_FLAG_SIMPLE_CONN,
        eNoDeactivate       = TPE_BODY_FLAG_ALWAYS_ACTIVE,
    };

    inline api::Coords screen_coords {};

    template <typename T>
    inline api::ObjectBinding<api::Framebuffer<T>> internal_buffer;

    void initialize_screen(api::Coords screen_size) NOEXCEPT {
        screen_coords = screen_size;
        S3L_resolutionX = screen_size.x;
        S3L_resolutionY = screen_size.y;
    }

    void initialize_buffer(api::Framebuffer<char>& framebuffer) NOEXCEPT {
        initialize_screen(framebuffer.get_coords());
        internal_buffer<char> = framebuffer;
    }

    struct ObjectModel {
        ObjectModel() = default;

        void load_model(const fs::path& filepath);


    private:
        std::string _name;
        std::vector<S3L_Unit> _vertices;
        std::vector<S3L_Index> _faces;
    };
}

// TODO: Implement this
//inline void S3L_PIXEL_FUNCTION(S3L_PixelInfo *pixel) {
//    const api::Coords position = { pixel->x, pixel->y };
//
//    render::internal_buffer<char>->get_active_buffer()->set_value(position, s3l_rr);
//}

#endif //PROJECT3_TEST_RENDER_CORE_HPP
