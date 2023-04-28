#ifndef PROJECT3_TEST_IAUDIOSOURCE_HPP
#define PROJECT3_TEST_IAUDIOSOURCE_HPP

#include <audio/core.hpp>

namespace audio {
    enum SourceType {
        eSingleInstance,
        eCircularInstance,
        eLoopingInstance,
    };

    struct IAudioSource {
        virtual ~IAudioSource() {
            delete _resource;
        }

        virtual void bind(IXAudio2* interface, const std::string& name) PURE;
        virtual void pause() PURE;
        virtual void set_volume(float f) PURE;
        NODISCARD virtual SourceType type() CNOEXCEPT PURE;

        void play(int operation_set = 0) {
            this->start(operation_set);
        }

        void clear(int operation_set = 0) {
            this->stop(operation_set);
        }

        virtual std::string name() CNOEXCEPT {
            return _name;
        }

    protected:
        virtual void start(int operation_set) PURE;
        virtual void stop(int operation_set) PURE;

    protected:
        std::string _name = "null";
        AudioResource* _resource = nullptr;
    };


    struct AudioVoiceSource {
        AudioVoiceSource() = default;

        AudioVoiceSource(const AudioVoiceSource&) = delete;
        AudioVoiceSource(AudioVoiceSource&& rhs) NOEXCEPT;

        ~AudioVoiceSource();

        void bind(IXAudio2* interface, AudioResource* resource) NOEXCEPT;
        void release() NOEXCEPT;
        void submit_buffer() NOEXCEPT;
        IXAudio2SourceVoice* operator->() NOEXCEPT;
        XAUDIO2_BUFFER& get_buffer() NOEXCEPT;
        operator bool() CNOEXCEPT;

    private:
        IXAudio2SourceVoice* _play_source = nullptr;
        XAUDIO2_BUFFER _buffer = {};
        bool _initialized = false;
    };
}

#endif //PROJECT3_TEST_IAUDIOSOURCE_HPP
