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
#include <api/input.hpp>
#include <api/resource_locator.hpp>

#define FPS 90
#define MSPF (1000 / (FPS))
#define DFPS (double)(FPS)

#define RES_X render::screen_coords.x
#define RES_Y render::screen_coords.y
#define TRES_X (RES_X)
#define TRES_Y ((RES_Y / 2))

#define ECS_MAX_SIZE 4096L
#define JOINTS_MAX_SIZE (ECS_MAX_SIZE * 8L)
#define CONNS_MAX_SIZE (JOINTS_MAX_SIZE * 2L)
#define TO_LUM(value) (255 * (value) / sizeof(render::ColorGrade))

template <typename T>
using ECSentry_t = std::array<T, ECS_MAX_SIZE>;

namespace fs = std::filesystem;
using modeASCII = char;
using modeRGBA = void;

void helper_set3DColor(uint8_t p, uint8_t a = 255);
void helper_drawModel(S3L_Model3D *model, TPE_Vec3 pos, TPE_Vec3 scale, TPE_Vec3 rot);

namespace TPE {
    using Bodies_t = std::array<TPE_Body, ECS_MAX_SIZE>;
    using Joints_t = std::array<TPE_Joint, JOINTS_MAX_SIZE>;
    using Connections_t = std::array<TPE_Connection, CONNS_MAX_SIZE>;

    inline constexpr TPE_Unit immovable = 100000;

    enum struct BodyFlag {
        eDeactivated        = TPE_BODY_FLAG_DEACTIVATED,
        eNonRotating        = TPE_BODY_FLAG_NONROTATING,
        eDisabled           = TPE_BODY_FLAG_DISABLED,
        eSoftbody           = TPE_BODY_FLAG_SOFT,
        eSimpleConnection   = TPE_BODY_FLAG_SIMPLE_CONN,
        eNoDeactivate       = TPE_BODY_FLAG_ALWAYS_ACTIVE,
    };

    enum struct WindingOrder {
        eClockwise, eCounterClockwise,
        eCW = eClockwise, eCCW = eCounterClockwise,
    };

    struct ObjectModel {
        ObjectModel() = default;
        ObjectModel(const fs::path& filepath, WindingOrder wo = WindingOrder::eClockwise);  // NOLINT

        bool load_model(const fs::path& filepath, WindingOrder wo = WindingOrder::eClockwise);
        bool compile_model() NOEXCEPT;
        void set_color(std::uint8_t color) NOEXCEPT { _color = color; }
        S3L_Model3D& get_model() CNOEXCEPT { return _model; }
        S3L_Model3D* pget_model() CNOEXCEPT { return &_model; }

        NODISCARD std::size_t vertex_count() CNOEXCEPT;
        NODISCARD std::size_t face_count() CNOEXCEPT;
        NODISCARD bool locked() CNOEXCEPT { return _locked; }
        NODISCARD std::uint8_t get_color() CNOEXCEPT { return _color; }

    private:
        NODISCARD bool _check_validity() CNOEXCEPT;

    private:
        std::string _name;
        std::vector<S3L_Unit> _vertices;
        std::vector<S3L_Index> _faces;

        mutable S3L_Model3D _model = {};
        std::uint8_t _color = 1;
        bool _locked = false;
    };

    inline void draw_model(ObjectModel& m, TPE_Vec3 pos, TPE_Vec3 scale, TPE_Vec3 rot) {
        helper_set3DColor(m.get_color());
        helper_drawModel(m.pget_model(), pos, scale, rot);
    }

    struct ECS;

    struct ECSentry {
        ECSentry(ECS* e, std::size_t idx);
        ECSentry(const ECSentry&) = delete;
        ECSentry(ECSentry&& rhs) NOEXCEPT;
        ~ECSentry();

        void move_by(TPE_Unit x, TPE_Unit y, TPE_Unit z) NOEXCEPT {
            TPE_bodyMoveBy(_get_body(), { x, y, z });
        }

        void rotate_by_axis(TPE_Unit x, TPE_Unit y, TPE_Unit z) NOEXCEPT {
            TPE_bodyRotateByAxis(_get_body(), { x, y, z });
        }

        void multiply_net_speed(TPE_Unit factor) NOEXCEPT {
            TPE_bodyMultiplyNetSpeed(_get_body(), factor);
        }

        TPE_Body* operator->() NOEXCEPT { return _get_body(); }

        NODISCARD TPE_Vec3 get_center_of_mass() CNOEXCEPT {
            return TPE_bodyGetCenterOfMass(_get_body());
        }

        NODISCARD TPE_Vec3 get_rotation(TPE_Unit joint1, TPE_Unit joint2, TPE_Unit joint3) CNOEXCEPT {
            return TPE_bodyGetRotation(_get_body(), joint1, joint2, joint3);
        }

    protected:
        TPE_Body* _get_body() NOEXCEPT;
        NODISCARD TPE_Body* _get_body() CNOEXCEPT;

    private:
        ECS* _bound_ecs = nullptr;
        const std::size_t _entity;
        friend struct ECS;
    };

    struct PlayerBody : ECSentry {
        PlayerBody(ECSentry&& e) : ECSentry(std::move(e)) {}    // NOLINT

        TPE_Joint& foot_joint() NOEXCEPT { return _get_body()->joints[0]; }
        TPE_Joint& head_joint() NOEXCEPT { return _get_body()->joints[1]; }
        TPE_Vec3& foot_position() NOEXCEPT { return foot_joint().position; }
        TPE_Vec3& head_position() NOEXCEPT { return head_joint().position; }
        TPE_JointVelocity& foot_velocity() NOEXCEPT { return foot_joint().velocity; }
        TPE_JointVelocity& head_velocity() NOEXCEPT { return head_joint().velocity; }
    };

    struct ECS {
        ECS() NOEXCEPT;
        ECS(const ECS&) = delete;
        ECS(ECS&&) = delete;

        static std::size_t get_microseconds() NOEXCEPT;

        std::size_t add_triangle(TPE_Unit s, TPE_Unit d, TPE_Unit mass) NOEXCEPT;
        std::size_t add_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_2Line(TPE_Unit w, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_centered_rect_full(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT;
        std::size_t add_ball(TPE_Unit s, TPE_Unit mass) NOEXCEPT;

        ECSentry bind(std::size_t idx) NOEXCEPT;
        ECSentry bind(std::string name) NOEXCEPT;
        PlayerBody get_player() NOEXCEPT;

        void register_env(TPE_ClosestPointFunction func) NOEXCEPT;
        void tick() NOEXCEPT;

        NODISCARD TPE_ClosestPointFunction get_env() CNOEXCEPT;
        TPE_World& get_world() NOEXCEPT;
        NODISCARD const TPE_World& get_world() CNOEXCEPT;

    private:
        std::size_t _next_free_index() NOEXCEPT;
        TPE_Joint* _joint_data() NOEXCEPT { return _joints.data() + _assigned_joints; }
        TPE_Connection* _connect_data() NOEXCEPT { return _conns.data() + _assigned_conns; }

        std::size_t _add_body(int joints, int conns, TPE_Unit mass) NOEXCEPT;
        std::size_t _add_body(std::string name, int joints, int conns, TPE_Unit mass) NOEXCEPT;
        void _body_added(int joints, int conns, TPE_Unit mass) NOEXCEPT;
        std::size_t _remove_body(std::size_t idx) NOEXCEPT;
        std::size_t _body_removed(TPE_Unit idx) NOEXCEPT;

        TPE_Body* _get_body(std::size_t idx) NOEXCEPT {
            auto body_idx = _bodies_idx[idx];
            return &_bodies[body_idx];
        }

    protected:
        ECSentry_t<bool> _skiplist = {};
        ECSentry_t<TPE_Unit> _bodies_idx = {};
        ECSentry_t<std::string> _names;
        ECSentry_t<TPE_Unit> _mass = {};
        ECSentry_t<api::iVec2> _color = {};

        ECSentry_t<bool> _disabled = {};
        ECSentry_t<bool> _has_gravity = {};
        ECSentry_t<bool> _always_active = {};
        ECSentry_t<bool> _is_sphere = {};
        ECSentry_t<bool> _do_rotation = {};

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
        std::size_t current_frame = 0;
        TPE_ClosestPointFunction _environment_function = nullptr;
        TPE_Unit _gravity = 5;

        friend struct ECSentry;
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
