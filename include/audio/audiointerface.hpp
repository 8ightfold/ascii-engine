#ifndef PROJECT3_TEST_AUDIOINTERFACE_HPP
#define PROJECT3_TEST_AUDIOINTERFACE_HPP

#include <unordered_map>
#include <audio/audiochannel.hpp>

namespace audio {
    struct XAudioInterface {
    private:
        XAudioInterface();

    public:
        XAudioInterface(const XAudioInterface&) = delete;
        XAudioInterface(XAudioInterface&&) = default;

        ~XAudioInterface();
        static void initialize() NOEXCEPT;
        static XAudioInterface create() NOEXCEPT;

        XAudioChannel& register_source(const std::string& name, SourceType type = SourceType::eSingleInstance) NOEXCEPT;
        void start_source(const std::string& name) NOEXCEPT;
        void restart_source(const std::string& name) NOEXCEPT;
        void stop_source(const std::string& name) NOEXCEPT;
        void set_volume(const std::string& name, float volume) NOEXCEPT;
        bool fade_in(const std::string& name, float rate, float approach = 1.0f) NOEXCEPT;
        bool fade_out(const std::string& name, float rate) NOEXCEPT;

    private:
        static int& _get_count() NOEXCEPT;

    private:
        IXAudio2* _audio_interface = nullptr;
        std::unordered_map<std::string, XAudioChannel> _pipeline_sources;
        IXAudio2MasteringVoice* _output_device = nullptr;
    };
}

#endif //PROJECT3_TEST_AUDIOINTERFACE_HPP
