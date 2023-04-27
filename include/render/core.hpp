#ifndef PROJECT3_TEST_RENDER_CORE_HPP
#define PROJECT3_TEST_RENDER_CORE_HPP

/*
    tinyphysicsengine and small3dlib are NOT made by me.
    They were both uploaded by jordan4ibanez.
    The opinions/views of drummyfish do not reflect my own, he just makes neat software
    You can find the repo here: https://github.com/jordan4ibanez/tinyphysicsengine
 */

#include <array>
#include <chrono>
#include <filesystem>

#include <render/tinyphysicsengine.hpp>
#include <render/small3dlib.hpp>

#include <api/core.hpp>
#include <api/framebuffer.hpp>

#define ECS_MAX_SIZE 2048
#define JOINTS_MAX_SIZE (ECS_MAX_SIZE * 8)
#define CONNS_MAX_SIZE (JOINTS_MAX_SIZE * 2)
#define TO_LUM(value) (255 * (value) / sizeof(render::ColorGrade))

template <typename T>
using ECSentry_t = std::array<T, ECS_MAX_SIZE>;

namespace fs = std::filesystem;

namespace TPE {
    using Bodies_t = std::array<TPE_Body, ECS_MAX_SIZE>;
    using Joints_t = std::array<TPE_Joint, JOINTS_MAX_SIZE>;
    using Connections_t = std::array<TPE_Connection, CONNS_MAX_SIZE>;

    enum struct BodyFlag {
        eDeactivated        = TPE_BODY_FLAG_DEACTIVATED,
        eNonRotating        = TPE_BODY_FLAG_NONROTATING,
        eDisabled           = TPE_BODY_FLAG_DISABLED,
        eSoftbody           = TPE_BODY_FLAG_SOFT,
        eSimpleConnection   = TPE_BODY_FLAG_SIMPLE_CONN,
        eNoDeactivate       = TPE_BODY_FLAG_ALWAYS_ACTIVE,
    };

    struct ObjectModel {
        ObjectModel() = default;
        ObjectModel(const fs::path& filepath);

        void load_model(const fs::path& filepath);
    private:
        std::string _name;
        std::vector<S3L_Unit> _vertices;
        std::vector<S3L_Index> _faces;
    };

    struct ECS {
        struct ECSentry {
            ECSentry(ECS* e, std::size_t idx);
            ECSentry(const ECSentry&) = delete;
            ECSentry(ECSentry&& rhs) NOEXCEPT;
            ~ECSentry() { if(_bound_ecs) _bound_ecs->_remove_body(_entity); }
        private:
            ECS* _bound_ecs = nullptr;
            const std::size_t _entity;
            friend struct ECS;
        };

        ECS() { TPE_worldInit(&_game_world, _bodies.data(), 0, nullptr); }
        ECS(const ECS&) = delete;

        static std::size_t get_microseconds() NOEXCEPT;

        std::size_t add_triangle(TPE_Unit s, TPE_Unit d, TPE_Unit mass) NOEXCEPT;
        std::size_t add_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_2Line(TPE_Unit w, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_rect_full(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_ball(TPE_Unit s, TPE_Unit mass) NOEXCEPT;

        ECSentry bind_index(std::size_t idx) NOEXCEPT;

    private:
        std::size_t _next_free_index() NOEXCEPT;
        TPE_Joint* _joint_data() NOEXCEPT { return _joints.data() + _assigned_joints; }
        TPE_Connection* _connect_data() NOEXCEPT { return _conns.data() + _assigned_conns; }

        std::size_t _add_body(int joints, int conns, TPE_Unit mass) NOEXCEPT;
        std::size_t _add_body(std::string name, int joints, int conns, TPE_Unit mass) NOEXCEPT;
        void _body_added(int joints, int conns, TPE_Unit mass) NOEXCEPT;
        std::size_t _remove_body(std::size_t idx) NOEXCEPT;
        std::size_t _body_removed(TPE_Unit idx) NOEXCEPT;

    protected:
        ECSentry_t<bool> _skiplist = {};
        ECSentry_t<TPE_Unit> _bodies_idx = {};
        ECSentry_t<bool> _disabled = {};
        ECSentry_t<bool> _has_gravity = {};
        ECSentry_t<bool> _always_active = {};
        ECSentry_t<std::string> _names;
        ECSentry_t<TPE_Unit> _mass = {};
        ECSentry_t<api::iVec2> _color = {};
        std::size_t _assigned_indices = 0;

        Bodies_t _bodies;
        Joints_t _joints;
        Connections_t _conns;
        TPE_Unit _active_bodies = 0;
        TPE_Unit _assigned_joints = 0;
        TPE_Unit _assigned_conns = 0;

        api::Map<std::string, std::size_t> _name_map;   /// Name -> ECS idx
        api::Map<TPE_Unit, std::size_t> _index_map;     /// World idx -> ECS idx

        TPE_World _game_world = {};
        TPE_Unit _gravity = 5;

        friend struct ECSentry;
    };

    using ECSentry = ECS::ECSentry;
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
            {  30,  31,  16,  17,  24,  25,  26,  27,  21,  15, 248, 249,   3,   4,   5,   6 }, // "▲▼►◄↑↓→←§☼°∙♥♦♣♠"
    };

    void initialize_screen(api::Coords screen_size) NOEXCEPT;

    void initialize_buffer(api::Framebuffer<char>& framebuffer) NOEXCEPT;

    /**
     * Draws a pixel to the current screen buffer. Only supports ASCII color at the moment.
     * @param color The color gradient. 0 is used for the world color, all others are from color_grades.
     * @param luminance A value in the range [0, 255] (this will be scaled appropriately)
     */
    void draw_pixel(int x, int y, uint8_t color, uint8_t luminance);
}

#endif //PROJECT3_TEST_RENDER_CORE_HPP
