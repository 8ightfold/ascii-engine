#ifndef PROJECT3_TEST_ENGINE_CORE_HPP
#define PROJECT3_TEST_ENGINE_CORE_HPP

// tinyphysicsengine and small3dlib are NOT made by me
// You can find the repo here: https://github.com/jordan4ibanez/tinyphysicsengine
#define S3L_USE_WIDER_TYPES 1
#define S3L_MAX_PIXELS 3686400
#define S3L_PIXEL_FUNCTION s3l_draw_pixel
#define S3L_PERSPECTIVE_CORRECTION 2

#include <engine/tinyphysicsengine.h>
#include <engine/small3dlib.h>

#include <api/core.hpp>

// TODO: Implement this
inline void S3L_PIXEL_FUNCTION(S3L_PixelInfo *pixel) {}

namespace engine {
    void initialize_screen(api::Coords screen_size) NOEXCEPT {
        S3L_resolutionX = screen_size.x;
        S3L_resolutionY = screen_size.y;
    }
}

#endif //PROJECT3_TEST_ENGINE_CORE_HPP
