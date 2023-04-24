#include "globalcontext.hpp"

#define META_REGEXP R"r(#(?:(callback)|(enable)|(\w+))\s+([^\n\r]+))r"
#define CALLBACK_STRIP_REGEXP R"r((\w+)\(\s*(.*)\s*\))r"
#define CALLBACK_VALUE_REGEXP R"r((\w+)(?:\s*:\s*([\w()*&<>]+))?\s*,?)r"

namespace parser {
    static std::string get_module_type(std::string_view name) {
        return static_cast<std::string>(name) + "*";
    }
}

namespace parser {
    TypeContext::TypeContext(std::string_view type_name) : _type_name(type_name) {
        _fundamental = is_fundamental(type_name);
    }

    TypeContext::_map_type* TypeContext::operator->() { return &_type_values; }
    std::string_view TypeContext::type() const { return _type_name; }
    const TypeContext::_map_type& TypeContext::values() const { return _type_values; }


    SlimTypeContext::SlimTypeContext(std::string_view type) : _type(type) {}

    bool SlimTypeContext::is_type() const {
        return (not _type.empty());
    }


    void GlobalContext::set_dirs(const fs::path& exe_dir, const fs::path& core_dir) {
        _exe_dir = exe_dir;
        _core_dir = core_dir;
    }

    void GlobalContext::set_package_name(std::string_view name) {
        _package_name = name;
    }

    void GlobalContext::set_debug_level(std::string_view value_str) {
        std::size_t value = 0;
        auto conversion_result = std::from_chars(value_str.begin(), value_str.end(), value);

        if (conversion_result.ec == std::errc::invalid_argument) {
            std::cout << "Unable to set debug level to '" << value_str << "'." << std::endl;
            return;
        }

        if(value > 3) value = 3;
        _debug_level = value;
    }

    void GlobalContext::set_output_filename(std::string_view name) {
        _output_filename = name;
    }

    void GlobalContext::set_stacktrace_depth(std::string_view depth_str) {
        std::size_t depth = 0;
        auto conversion_result = std::from_chars(depth_str.begin(), depth_str.end(), depth);

        if (conversion_result.ec == std::errc::invalid_argument) {
            std::cout << "Unable to set stacktrace depth to '" << depth_str << "'." << std::endl;
            return;
        }

        _stacktrace_depth = depth;
    }


    void GlobalContext::enable_timer() {
        _do_timing = true;
    }

    void GlobalContext::start_timer() {
        _time_start = steady_clock::now();
    }

    void GlobalContext::stop_timer() {
        auto time_end = steady_clock::now();
        if(_do_timing) {
            std::chrono::duration<double, std::milli> duration = time_end - _time_start;
            double length = duration.count();
            if(length < 100.0) {
                std::cout << "Codegen took " << length << " milliseconds." << std::endl;
            }
            else {
                std::cout << "Codegen took " << (length / 1000.0) << " seconds." << std::endl;
            }
        }
    }


    void GlobalContext::parse_meta(std::string_view meta) {
        constexpr auto matcher = ctre::match<META_REGEXP>;
        auto&& [_, callback, enable, directive, value] = matcher(meta);

        if(callback) _parse_callback(value);
        else if(enable) _parse_enable(value);
        else if(directive) _parse_directive(directive, value);
        else throw InvalidMetaValue(meta);
    }

    void GlobalContext::verify_core() const {
        if(not fs::exists(_core_dir / "core.uxi")) {
            std::cerr << "No core file found at " << _core_dir << "." << std::endl;
            std::exit(-1);
        }
    }

    bool GlobalContext::check_debug_level(std::size_t value) const {
        return (_debug_level >= value);
    }

    fs::path GlobalContext::get_exe() const { return _exe_dir; }
    fs::path GlobalContext::get_core() const { return _core_dir; }
    std::string_view GlobalContext::get_package_name() const { return _package_name; }
    std::size_t GlobalContext::get_stacktrace_depth() const { return _stacktrace_depth; }

    void GlobalContext::_parse_directive(std::string_view directive, std::string_view value) {
        if(directive == "colortype") {
            if(_color_type != ColorType::eDefault)
                throw InvalidMetaValue(value);

            if(value == "ascii") _color_type = ColorType::eASCII;
            else if(value == "rgba") _color_type = ColorType::eRGBA;
            else throw InvalidMetaValue(value);
        }
        else if(directive == "entry") {
            if(not _entry_point.empty()) throw InvalidMetaValue(value);
            _entry_point = value;
        }
        else throw InvalidMetaValue(directive);
    }

    void GlobalContext::_parse_callback(std::string_view callback) {
        constexpr auto callback_strip = ctre::match<CALLBACK_STRIP_REGEXP>;
        auto&& [_, name, types] = callback_strip(callback);
        if(not (name && types)) throw InvalidMetaValue(callback);

        auto [pcallback_value, inserted] = _callbacks.emplace(name.to_view(), name.to_view());
        if(not inserted) throw std::runtime_error("Element count not be inserted into '_callbacks'.");
        auto&& [key, callback_value] = *pcallback_value;

        constexpr auto callback_types = ctre::range<CALLBACK_VALUE_REGEXP>;
        auto matches = callback_types(types);
        const std::string package_type = get_module_type(_package_name);

        for(auto&& [match, type_name, type] : matches) {
            if(not type_name) {
                if(not match) throw InvalidMetaValue(callback);
                else throw InvalidMetaValue(match);
            }

            if(not type) callback_value.add_type(type_name, package_type);
            else callback_value.add_type(type_name, type);
        }
    }

    void GlobalContext::_parse_enable(std::string_view to_enable) {
        if(to_enable == "device")  _enabled_device = true;
        else if(to_enable == "intluminance") _enabled_intluminance = true;
        else throw InvalidMetaValue(to_enable);
    }
}