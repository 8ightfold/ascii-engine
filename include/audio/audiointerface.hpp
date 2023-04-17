#ifndef PROJECT3_TEST_AUDIOINTERFACE_HPP
#define PROJECT3_TEST_AUDIOINTERFACE_HPP

#include <unordered_map>
#include <audio/audiochannel.hpp>

namespace audio {
    struct XAudioInterface {
    private:
        XAudioInterface() {
            if(FAILED( XAudio2Create(&_audio_interface, 0, XAUDIO2_DEFAULT_PROCESSOR) ))
                assert(!"Failed to create XAudio2 instance!");

            if(FAILED( _audio_interface->CreateMasteringVoice(&_output_device, XAUDIO2_DEFAULT_CHANNELS,
                                                              XAUDIO2_DEFAULT_SAMPLERATE, 0, nullptr, nullptr, AudioCategory_GameEffects) ))
                assert(!"Failed to create XAudio2 mastering voice!");
        }

    public:
        XAudioInterface(const XAudioInterface&) = delete;
        XAudioInterface(XAudioInterface&&) = default;

        ~XAudioInterface();
        static XAudioInterface create_interface() NOEXCEPT;

        XAudioChannel& register_source(const std::string& name, SourceType type = SourceType::eSingleInstance) NOEXCEPT;
        void start_source(const std::string& name) NOEXCEPT;
        void restart_source(const std::string& name) NOEXCEPT;
        void stop_source(const std::string& name) NOEXCEPT;

    private:
        static int& _get_count() NOEXCEPT;

    private:
        IXAudio2* _audio_interface = nullptr;
        std::unordered_map<std::string, XAudioChannel> _pipeline_sources;
        IXAudio2MasteringVoice* _output_device = nullptr;
    };
}

#endif //PROJECT3_TEST_AUDIOINTERFACE_HPP
