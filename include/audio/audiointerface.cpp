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

    XAudioChannel& XAudioInterface::register_source(const std::string& name) NOEXCEPT {
        if(not _pipeline_sources.contains(name)) {
            auto& source = _pipeline_sources[name];
            if(not source) source.bind(_audio_interface, name);
            return source;
        }

        return _pipeline_sources[name];
    }

    void XAudioInterface::start_source(const std::string& name) NOEXCEPT {
        auto& source = register_source(name);
        source.play();
    }

    void XAudioInterface::restart_source(const std::string& name) NOEXCEPT {
        auto& source = register_source(name);
        source.pause();
        source._play_source->FlushSourceBuffers();
        source.play();
    }

    void XAudioInterface::stop_source(const std::string& name) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            auto& source = _pipeline_sources[name];
            source._play_source->Stop(0);
        }
    }

    int& XAudioInterface::_get_count() NOEXCEPT {
        static int count = 0;
        return count;
    }
}
