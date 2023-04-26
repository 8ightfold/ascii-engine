#include "audiosource_looping.hpp"

namespace audio {
    void AudioSourceLooping::bind(IXAudio2* interface, const std::string& name) {
        _name = name;
        _resource = new AudioResource(name, "wav");
        _play_source.bind(interface, _resource);
        _play_source.get_buffer().LoopCount = XAUDIO2_LOOP_INFINITE;
    }

    void AudioSourceLooping::start(int operation_set) {
        if(_play_source) LIKELY {
            if(_playing) this->clear();
            _play_source.submit_buffer();
            _play_source->Start(0, operation_set);
            _playing = true;
        }
    }

    void AudioSourceLooping::stop(int operation_set) {
        if(_play_source) LIKELY {
            _play_source->Stop(0, operation_set);
            _play_source->FlushSourceBuffers();
            _playing = false;
        }
    }

    void AudioSourceLooping::pause() {
        if(_play_source) LIKELY {
            _play_source->Stop();
            _playing = false;
        }
    }

    void AudioSourceLooping::set_volume(float f) {
        if(_play_source) LIKELY {
            _play_source->Stop();
            _play_source->SetVolume(f);
            _play_source->Start();
        }
    }

    SourceType AudioSourceLooping::type() CNOEXCEPT {
        return SourceType::eLoopingInstance;
    }
}