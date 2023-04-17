#ifndef PROJECT3_TEST_AUDIOSOURCE_CIRCULAR_HPP
#define PROJECT3_TEST_AUDIOSOURCE_CIRCULAR_HPP

#include <audio/source_types/iaudiosource.hpp>

namespace audio {
    struct AudioSourceCircular final : IAudioSource {
        AudioSourceCircular() : _play_sources(AUDIO_CIRCULAR_QUEUE_MAX) {}
        ~AudioSourceCircular() override = default;

        void bind(IXAudio2* interface, const std::string& name) override;
        void start(int operation_set) override;
        void stop(int operation_set) override;
        void pause() override;
        NODISCARD SourceType type() CNOEXCEPT override;

    private:
        api::CircularQueue<AudioVoiceSource> _play_sources;
        bool _bound = false;
    };
}

#endif //PROJECT3_TEST_AUDIOSOURCE_CIRCULAR_HPP
