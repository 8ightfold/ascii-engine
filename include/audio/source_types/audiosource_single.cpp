#include "audiosource_single.hpp"

namespace audio {
    void AudioSourceSingle::bind(IXAudio2* interface, const std::string& name) {
        _name = name;
        _resource = new AudioResource(name, "wav");
        _play_source.bind(interface, _resource);
    }

    void AudioSourceSingle::start(int operation_set) {
        if(_play_source) LIKELY {
            if(_playing) this->clear();
            _play_source.submit_buffer();
            _play_source->Start(0, operation_set);
            _playing = true;
        }
    }

    void AudioSourceSingle::stop(int operation_set) {
        if(_play_source) LIKELY {
            _play_source->Stop(0, operation_set);
            _play_source->FlushSourceBuffers();
            _playing = false;
        }
    }

    void AudioSourceSingle::pause() {
        if(_play_source) LIKELY {
            _play_source->Stop();
            _playing = false;
        }
    }

    void AudioSourceSingle::set_volume(float f) {
        if(_play_source) LIKELY {
            _play_source->Stop();
            _play_source->SetVolume(f);
            _play_source->Start();
        }
    }

    SourceType AudioSourceSingle::type() CNOEXCEPT {
        return SourceType::eSingleInstance;
    }
}