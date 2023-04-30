#include "audiointerface.hpp"

namespace audio {
    XAudioInterface::XAudioInterface() {
        if(FAILED( XAudio2Create(&_audio_interface, 0, XAUDIO2_DEFAULT_PROCESSOR) )) {
            FATAL("Failed to create XAudio2 instance.");
        }

        if(FAILED( _audio_interface->CreateMasteringVoice(&_output_device, XAUDIO2_DEFAULT_CHANNELS,
        XAUDIO2_DEFAULT_SAMPLERATE, 0, nullptr, nullptr, AudioCategory_GameEffects) )) {
            FATAL("Failed to create XAudio2 mastering voice.");
        }
    }

    XAudioInterface::~XAudioInterface() {
        for(auto&& [name, source] : _pipeline_sources) source.release();
        if(_output_device) _output_device->DestroyVoice();
        if(_audio_interface) _audio_interface->Release();

        --_get_count();
    }

    void XAudioInterface::initialize() NOEXCEPT {
        if(FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
            FATAL("Failed to initialize COM.");
        }
    }

    XAudioInterface XAudioInterface::create() NOEXCEPT {
        int& count = _get_count();
        if(count) FATAL("Only one XAudioInterface instance may exist at a time.");

        XAudioInterface interface {};

        ++count;
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
            auto& source = _pipeline_sources[name];
            source.pause();
            source.clear();
            source.play();
        }
    }

    void XAudioInterface::stop_source(const std::string& name) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            _pipeline_sources[name].pause();
        }
    }

    void XAudioInterface::stop_all() NOEXCEPT {
        for(auto& name_source : _pipeline_sources) {
            name_source.second.pause();
        }
    }

    void XAudioInterface::set_volume(const std::string& name, float volume) NOEXCEPT {
        if(_pipeline_sources.contains(name)) {
            auto& source = _pipeline_sources[name];
            source.set_volume(volume);
        }
    }

    bool XAudioInterface::fade_in(const std::string& name, float rate, float approach) NOEXCEPT {
        if(_pipeline_sources.contains(name)) LIKELY {
            auto& source = _pipeline_sources[name];
            return source.linear_fade(approach, rate);
        }
        return false;
    }

    bool XAudioInterface::fade_out(const std::string& name, float rate) NOEXCEPT {
        if(_pipeline_sources.contains(name)) LIKELY {
            auto& source = _pipeline_sources[name];
            return source.linear_fade(0.0f, rate);
        }
        return false;
    }

    int& XAudioInterface::_get_count() NOEXCEPT {
        static int count = 0;
        return count;
    }
}
