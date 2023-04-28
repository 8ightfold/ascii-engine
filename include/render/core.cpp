#include "core.hpp"
#include <exception>
#include <fstream>

#if ENABLE_RENDER_MESSAGES
#  define MODEL_PRINT(...) std::printf(__VA_ARGS__);
#else
#  define MODEL_PRINT(...)
#endif

namespace TPE {
    ObjectModel::ObjectModel(const fs::path& filepath, WindingOrder wo) {
        load_model(filepath, wo);
    }

    bool ObjectModel::load_model(const fs::path& filepath, WindingOrder wo) {
        if(_locked) return false;

        auto abs_path = api::ResourceLocator::get_file(filepath);
        std::ifstream is { abs_path };
        debug_assert(is.is_open());
        if(not is.is_open()) return false;

        char type;
        std::string line;
        double x, y, z;
        TPE_Unit v1, v2, v3;

        auto skipline = [&] { std::getline(is, line); };

        while(not is.eof()) {
            if(not (is >> type)) {
                skipline();
                continue;
            }

            if(type == 'o') is >> _name;
            else if(type == 'v') {
                is >> x >> y >> z;
                constexpr std::size_t scale_factor = TPE_FRACTIONS_PER_UNIT;
                _vertices.push_back(TPE_Unit(x * scale_factor));
                _vertices.push_back(TPE_Unit(y * scale_factor));
                _vertices.push_back(TPE_Unit(z * scale_factor));
                MODEL_PRINT("vert: %lli, %lli, %lli\n",
                            TPE_Unit(x * scale_factor), TPE_Unit(y * scale_factor), TPE_Unit(z * scale_factor))
            }
            else if(type == 'f') {
                is >> v1 >> v2 >> v3;
                if(wo == WindingOrder::eClockwise) std::swap(v1, v3);
                _faces.push_back(--v1);
                _faces.push_back(--v2);
                _faces.push_back(--v3);
                MODEL_PRINT("face: %lli, %lli, %lli\n", v1, v2, v3)
            }
            else skipline();
        }

        return true;
    }

    bool ObjectModel::compile_model() NOEXCEPT {
        if(_locked) return false;
        if(not _check_validity()) return false;

        S3L_model3DInit(_vertices.data(), vertex_count(),
                        _faces.data(), face_count(), &_model);

        _locked = true;
        return true;
    }

    std::size_t ObjectModel::vertex_count() CNOEXCEPT {
        debug_assert(_check_validity());
        return _vertices.size() / 3;
    }

    std::size_t ObjectModel::face_count() CNOEXCEPT {
        debug_assert(_check_validity());
        return _faces.size() / 3;
    }

    bool ObjectModel::_check_validity() CNOEXCEPT {
        bool valid_verts = (not _vertices.empty()) and (_vertices.size() % 3 == 0);
        bool valid_faces = (not _faces.empty()) and (_faces.size() % 3 == 0);
        return valid_verts and valid_faces;
    }


    ECSentry::ECSentry(ECS* e, std::size_t idx) : _bound_ecs(e), _entity(idx) {
        debug_assert(e->_skiplist[idx]);
    }

    ECSentry::ECSentry(ECSentry&& rhs) NOEXCEPT
    : _bound_ecs(rhs._bound_ecs), _entity(rhs._entity) {
        rhs._bound_ecs = nullptr;
    }

    ECSentry::~ECSentry() { if(_bound_ecs) _bound_ecs->_remove_body(_entity); }

    TPE_Body* ECSentry::_get_body() NOEXCEPT { return _bound_ecs->_get_body(_entity); }
    TPE_Body* ECSentry::_get_body() CNOEXCEPT { return _bound_ecs->_get_body(_entity); }


    ECS::ECS() NOEXCEPT {
        TPE_worldInit(&_game_world, _bodies.data(), 0, nullptr);
        std::size_t player_pos = add_2Line(400, 300, 400);
        _name_map["$PLAYER"] = player_pos;
        _names[player_pos] = "$PLAYER";
    }

    std::size_t ECS::get_microseconds() NOEXCEPT {
        namespace sttm = std::chrono;
        auto us = sttm::duration_cast<sttm::microseconds>(sttm::system_clock::now().time_since_epoch());
        return us.count();
    }

    std::size_t ECS::add_triangle(TPE_Unit s, TPE_Unit d, TPE_Unit mass) NOEXCEPT {
        TPE_makeTriangle(_joint_data(),_connect_data(),s,d);
        return _add_body(3,3,mass);
    }

    std::size_t ECS::add_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_makeBox(_joint_data(),_connect_data(),w,h,d,joint_size);
        return _add_body(8,16,mass);
    }

    std::size_t ECS::add_centered_box(TPE_Unit w, TPE_Unit h, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_makeCenterBox(_joint_data(),_connect_data(),w,h,d,joint_size);
        return _add_body(9,18,mass);
    }

    std::size_t ECS::add_2Line(TPE_Unit w, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_make2Line(_joint_data(),_connect_data(),w,joint_size);
        return _add_body(2,1,mass);
    }

    std::size_t ECS::add_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_makeRect(_joint_data(),_connect_data(),w,d,joint_size);
        return _add_body(4,6,mass);
    }

    std::size_t ECS::add_centered_rect(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_makeCenterRect(_joint_data(),_connect_data(),w,d,joint_size);
        return _add_body(5,8,mass);
    }

    std::size_t ECS::add_centered_rect_full(TPE_Unit w, TPE_Unit d, TPE_Unit joint_size, TPE_Unit mass) NOEXCEPT {
        TPE_makeCenterRectFull(_joint_data(),_connect_data(),w,d,joint_size);
        return _add_body(5,10,mass);
    }

    std::size_t ECS::add_ball(TPE_Unit s, TPE_Unit mass) NOEXCEPT {
        (*_joint_data()) = TPE_joint(TPE_vec3(0,0,0),s);
        return _add_body(1,0,mass);
    }

    ECSentry ECS::bind(std::size_t idx) NOEXCEPT {
        return { this, idx };
    }

    ECSentry ECS::bind(std::string name) NOEXCEPT {
        debug_assert(_name_map.contains(name));
        std::size_t idx = _name_map[name];
        return { this, idx };
    }

    PlayerBody ECS::get_player() NOEXCEPT {
        return { bind(0) };
    }

    void ECS::register_env(TPE_ClosestPointFunction func) NOEXCEPT {
        _game_world.environmentFunction = func;
        _environment_function = func;
    }

    void ECS::tick() NOEXCEPT {
        TPE_worldStep(&_game_world);

        for(std::size_t idx = 0; idx < _assigned_indices; ++idx) {
            if(not _skiplist[idx] or _disabled[idx]) continue;
            TPE_Body* curr_body = &_bodies[_bodies_idx[idx]];

            if(_has_gravity[idx]) TPE_bodyApplyGravity(curr_body, _gravity);
        }

        ++current_frame;
    }

    TPE_ClosestPointFunction ECS::get_env() CNOEXCEPT {
        return _environment_function;
    }

    TPE_World& ECS::get_world() NOEXCEPT {
        return _game_world;
    }

    const TPE_World& ECS::get_world() CNOEXCEPT {
        return _game_world;
    }

    std::size_t ECS::_next_free_index() NOEXCEPT {
        std::size_t index = 0;
        for(bool b : _skiplist) {
            if(not b) break;
            ++index;
        }

        _skiplist[index] = true;
        if(_assigned_indices == index) ++_assigned_indices;
        return index;
    }

    std::size_t ECS::_add_body(int joints, int conns, TPE_Unit mass) NOEXCEPT {
        const std::size_t idx = _next_free_index();

        _body_added(joints, conns, mass);
        _bodies_idx[idx] = _active_bodies - 1;
        _mass[idx] = mass;
        _has_gravity[idx] = (mass != 0);
        _color[idx] = { 0, 255 };
        _index_map[_bodies_idx[idx]] = idx;

        return idx;
    }

    std::size_t ECS::_add_body(std::string name, int joints, int conns, TPE_Unit mass) NOEXCEPT {
        debug_assert(not _name_map.contains(name));
        const std::size_t idx = _add_body(joints, conns, mass);

        if(not name.empty()) {
            _name_map[name] = idx;
            _names[idx] = name;
        }

        return idx;
    }

    void ECS::_body_added(int joints, int conns, TPE_Unit mass) NOEXCEPT {
        TPE_bodyInit(&_bodies[_active_bodies],
                     &_joints[_assigned_joints], joints,
                     &_conns[_assigned_conns], conns, mass);

        ++_active_bodies;
        ++_game_world.bodyCount;
        _assigned_joints += joints;
        _assigned_conns += conns;
    }

    std::size_t ECS::_remove_body(std::size_t idx) NOEXCEPT {
        debug_assert(_skiplist[idx]);
        TPE_Unit body_idx = _bodies_idx[idx];
        std::size_t swapped_idx = _body_removed(body_idx);

        _skiplist[idx] = false;
        _bodies_idx[swapped_idx] = body_idx;
        _index_map[body_idx] = swapped_idx;

        if(not _names[idx].empty())
            _name_map.erase(_names[idx]);

        return swapped_idx;
    }

    std::size_t ECS::_body_removed(TPE_Unit idx) NOEXCEPT {
        debug_assert(idx < _active_bodies);
        std::swap(_bodies[idx], _bodies[_active_bodies - 1]);
        --_active_bodies;
        --_game_world.bodyCount;
        return _active_bodies;
    }
}


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
        debug_assert(not (x < 0 or y < 0 or x > TRES_X or y > TRES_Y), "Invalid screen coordinates.");

        uint8_t c;
        if(color == 0) {
            long offset = double(luminance / 255.0) * sizeof(world_color);
            c = world_color[offset];
        }
        else {
            luminance = luminance / sizeof(render::ColorGrade);
            luminance = (luminance > 15) ? 15 : luminance;
            c = render::color_grades[color - 1][luminance];
        }
        internal_buffer<char>->get_active_buffer()->set_value({ x, y }, c);
    }
}