#include "iaudiosource.hpp"

namespace audio {
    AudioVoiceSource::AudioVoiceSource(AudioVoiceSource&& rhs) NOEXCEPT
    : _play_source(rhs._play_source), _buffer(rhs._buffer) {
        rhs._play_source = nullptr;
        rhs._buffer = {};
    }

    AudioVoiceSource::~AudioVoiceSource() {
        if(_play_source and _initialized) LIKELY {
            _play_source->Stop(0);
            _play_source->DestroyVoice();
        }
    }

    void AudioVoiceSource::bind(IXAudio2* interface, AudioResource* resource) NOEXCEPT {
        if(not resource) return;
        auto* wfx = &resource->get_fmt()->wfx;

        if(FAILED( interface->CreateSourceVoice(&_play_source, wfx, 0,
                                                XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, nullptr, nullptr) )) UNLIKELY {
            auto name = resource->get_filename();
            std::cerr << "Could not create source voice '" << name << "'" << std::endl;
            std::exit(-1);
        }

        if(_play_source) LIKELY {
            const auto& data_buf = resource->get_data();
            _buffer.AudioBytes = data_buf.size();
            _buffer.pAudioData = data_buf.data();
            _buffer.Flags = XAUDIO2_END_OF_STREAM;
            _initialized = true;
        }
    }

    void AudioVoiceSource::release() NOEXCEPT {
        if(_play_source and _initialized) LIKELY {
            _play_source->Stop(0);
            _play_source->DestroyVoice();
            _play_source = nullptr;
            _initialized = false;
        }
    }

    void AudioVoiceSource::submit_buffer() NOEXCEPT {
        if(FAILED( _play_source->SubmitSourceBuffer(&_buffer) )) UNLIKELY {
            std::cerr << "Could not create source buffer." << std::endl;
            std::exit(-1);
        }
    }

    IXAudio2SourceVoice* AudioVoiceSource::operator->() NOEXCEPT {
        if(not _initialized) UNLIKELY return nullptr;
        return _play_source;
    }

    XAUDIO2_BUFFER& AudioVoiceSource::get_buffer() NOEXCEPT {
        return _buffer;
    }

    AudioVoiceSource::operator bool() CNOEXCEPT {
        if(not _initialized) UNLIKELY return false;
        return _play_source;
    }
}
