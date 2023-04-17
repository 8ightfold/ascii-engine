#include "audiochannel.hpp"

namespace audio {
    void XAudioChannel::release() NOEXCEPT {
        if(_play_source) {
            _play_source->Stop(0);
            _play_source->DestroyVoice();
            _play_source = nullptr;
        }
    }

    XAudioChannel::~XAudioChannel() {
        release();
        delete _resource;
    }

    XAudioChannel::operator bool() CNOEXCEPT {
        return (_play_source && _resource);
    }

    std::string XAudioChannel::filename() CNOEXCEPT {
        if(_resource) return _resource->get_filename();
        else return "null";
    }

    void XAudioChannel::bind(IXAudio2* interface, const std::string& name) NOEXCEPT {
        _resource = new AudioResource(name, "wav");
        auto* wfx = &_resource->get_fmt()->wfx;

        if(FAILED( interface->CreateSourceVoice(&_play_source, wfx, 0,
                                                XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr) )) {
            std::cerr << "Could not create source voice '" << name << "'" << std::endl;
            std::exit(-1);
        }

        const auto& data_buf = _resource->get_data();
        _buffer.AudioBytes = data_buf.size();
        _buffer.pAudioData = data_buf.data();
        _buffer.Flags = XAUDIO2_END_OF_STREAM;
    }

    void XAudioChannel::play() NOEXCEPT {
        if(_play_source) LIKELY {
            if(FAILED( _play_source->SubmitSourceBuffer(&_buffer) )) UNLIKELY {
                std::cerr << "Could not create source buffer for '" << _resource->get_filename() << "'" << std::endl;
                std::exit(-1);
            }
            _play_source->Start(0);
        }
    }

    void XAudioChannel::pause() NOEXCEPT {
        if(_play_source) LIKELY {
            _play_source->Stop(0);
        }
    }
}
