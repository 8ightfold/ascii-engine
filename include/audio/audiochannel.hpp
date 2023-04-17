#ifndef PROJECT3_TEST_AUDIOCHANNEL_HPP
#define PROJECT3_TEST_AUDIOCHANNEL_HPP

#include <audio/core.hpp>

namespace audio {
    struct XAudioChannel {
        XAudioChannel() = default;
        void release() NOEXCEPT;
        ~XAudioChannel();

        NODISCARD operator bool() CNOEXCEPT;
        NODISCARD std::string filename() CNOEXCEPT;

    private:
        void bind(IXAudio2* interface, const std::string& name) NOEXCEPT;
        void play() NOEXCEPT;
        void pause() NOEXCEPT;

    private:
        IXAudio2SourceVoice* _play_source = nullptr;
        AudioResource* _resource = nullptr;
        XAUDIO2_BUFFER _buffer = {};

        friend struct XAudioInterface;
    };
}

#endif //PROJECT3_TEST_AUDIOCHANNEL_HPP
