#include "audiointerface.hpp"

namespace audio {
    XAudioInterface::~XAudioInterface() {
        for(auto&& [name, source] : _pipeline_sources) source.release();
        if(_output_device) _output_device->DestroyVoice();
        if(_audio_interface) _audio_interface->Release();

        --_get_count();
    }

    XAudioInterface XAudioInterface::create_interface() NOEXCEPT {
        static int count = 0;
        if(count) assert(!"FATAL: Only one XAudioInterface instance may exist at a time!");

        if(FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
            assert(!"Failed to initialize COM!");
        }

        XAudioInterface interface {};

        ++_get_count();
        return interface;
    }

    XAudioChannel& XAudioInterface::register_source(const std::string& name, SourceType type) NOEXCEPT {
        if(not _pipeline_sources.contains(name)) {
            auto& source = _pipeline_sources[name];
            if(not source) {
                source.set_type(type);
                source.bind(_audio_interface, name);
            }
            return source;
        }

        return _pipeline_sources[name];
    }

    void XAudioInterface::start_source(const std::string& name) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            _pipeline_sources[name].play();
        }
    }

    void XAudioInterface::restart_source(const std::string& name) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            _pipeline_sources[name].pause();
            _pipeline_sources[name].clear();
            _pipeline_sources[name].play();
        }
    }

    void XAudioInterface::stop_source(const std::string& name) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            auto& source = _pipeline_sources[name];
            _pipeline_sources[name].pause();
        }
    }

    int& XAudioInterface::_get_count() NOEXCEPT {
        static int count = 0;
        return count;
    }
}
