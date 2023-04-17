#include "audiochannel.hpp"

namespace audio {
    void XAudioChannel::release() NOEXCEPT {
        delete _play_source;
        _play_source = nullptr;
    }

    XAudioChannel::~XAudioChannel() {
        release();
    }

    XAudioChannel::operator bool() CNOEXCEPT {
        return _play_source;
    }

    std::string XAudioChannel::filename() CNOEXCEPT {
        if(_play_source) return _play_source->name();
        else return "null";
    }

    // Private
    static IAudioSource* create_instance(SourceType type) NOEXCEPT {
        switch(type) {
            case SourceType::eSingleInstance: {
                return new AudioSourceSingle();
            }
            case SourceType::eLoopingInstance: {
                return new AudioSourceLooping();
            }
            case SourceType::eCircularInstance: {
                return new AudioSourceCircular();
            }
            default: return nullptr;
        }
    }

    void XAudioChannel::bind(IXAudio2* interface, const std::string& name) NOEXCEPT {
        if(not _play_source) {
            _play_source = create_instance(_type);
            _play_source->bind(interface, name);
        }
    }

    void XAudioChannel::set_type(SourceType type) NOEXCEPT {
        if(not _play_source) _type = type;
    }

    SourceType XAudioChannel::get_type() CNOEXCEPT {
        return _type;
    }

    void XAudioChannel::play() NOEXCEPT {
        if(_play_source) LIKELY {
            _play_source->play();
        }
    }

    void XAudioChannel::clear() NOEXCEPT {
        if(_play_source) LIKELY {
            _play_source->clear();
        }
    }

    void XAudioChannel::pause() NOEXCEPT {
        if(_play_source) LIKELY {
            _play_source->pause();
        }
    }
}
