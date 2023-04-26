#include "audiosource_circular.hpp"

namespace audio {
    void AudioSourceCircular::bind(IXAudio2* interface, const std::string& name) {
        _name = name;
        _resource = new AudioResource(name, "wav");
        _play_sources.apply([&](AudioVoiceSource& src){ src.bind(interface, _resource); });
        _bound = true;
    }

    void AudioSourceCircular::start(int operation_set) {
        if(_bound) LIKELY {
            auto& src = _play_sources.next();
            src.submit_buffer();
            src->Start(0, operation_set);
        }
    }

    void AudioSourceCircular::stop(int operation_set) {
        if(_bound) LIKELY {
            _play_sources.apply([&](AudioVoiceSource& src){
                src->Stop(0, operation_set);
                src->FlushSourceBuffers();
            });
        }
    }

    void AudioSourceCircular::pause() {}

    void AudioSourceCircular::set_volume(float f) {
        if(_bound) LIKELY {
            auto& src = _play_sources.peek();
            src->SetVolume(f, 0);
        }
    }

    SourceType AudioSourceCircular::type() CNOEXCEPT {
        return SourceType::eCircularInstance;
    }
}