#include "core.hpp"
#include <exception>

namespace TPE {
    ObjectModel::ObjectModel(const fs::path& filepath) {
        load_model(filepath);
    }

    void ObjectModel::load_model(const fs::path& filepath) {

    }


    ECS::ECSentry::ECSentry(ECS* e, std::size_t idx) : _bound_ecs(e), _entity(idx) {
        DEBUG_ONLY( assert(e->_skiplist[idx]); )
    }

    ECS::ECSentry::ECSentry(ECSentry&& rhs) NOEXCEPT
    : _bound_ecs(rhs._bound_ecs), _entity(rhs._entity) {
        rhs._bound_ecs = nullptr;
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

    ECSentry ECS::bind_index(std::size_t idx) NOEXCEPT {
        return { this, idx };
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
        _index_map[_bodies_idx[idx]] = idx;

        return idx;
    }

    std::size_t ECS::_add_body(std::string name, int joints, int conns, TPE_Unit mass) NOEXCEPT {
        DEBUG_ONLY( assert(not _name_map.contains(name)); )
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
        DEBUG_ONLY( assert(_skiplist[idx]); )
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
        DEBUG_ONLY( assert(idx < _active_bodies); )
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
        DEBUG_ONLY(
            if(x > screen_coords.x - 1 or y > (screen_coords.y / 2) - 1)
                throw std::runtime_error("Invalid screen coords!"); )

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
}