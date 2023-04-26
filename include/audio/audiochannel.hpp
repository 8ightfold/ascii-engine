#ifndef PROJECT3_TEST_AUDIOCHANNEL_HPP
#define PROJECT3_TEST_AUDIOCHANNEL_HPP

#include <audio/core.hpp>
#include <audio/audiosource.hpp>

namespace audio {
    struct XAudioChannel {
        XAudioChannel() = default;
        void release() NOEXCEPT;
        ~XAudioChannel();

        NODISCARD operator bool() CNOEXCEPT;
        NODISCARD std::string filename() CNOEXCEPT;

    private:
        void bind(IXAudio2* interface, const std::string& name) NOEXCEPT;
        void set_type(SourceType type) NOEXCEPT;
        NODISCARD SourceType get_type() CNOEXCEPT;
        void play() NOEXCEPT;
        void clear() NOEXCEPT;
        void pause() NOEXCEPT;
        void set_volume(float f) NOEXCEPT;

    private:
        IAudioSource* _play_source = nullptr;
        float volume = 1.0f;
        SourceType _type = SourceType::eSingleInstance;
        friend struct XAudioInterface;
    };
}

#endif //PROJECT3_TEST_AUDIOCHANNEL_HPP
