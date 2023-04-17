#ifndef PROJECT3_TEST_AUDIO_CORE_HPP
#define PROJECT3_TEST_AUDIO_CORE_HPP

/*
    cxaudio2 and memmem are NOT made by me
    Both were made by Pascal Gloor
    You can find the repo here: https://github.com/deadcast2/xaudio2-mingw-w64
 */

#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include <api/core.hpp>
#include <audio/cxaudio2.h>
#include <audio/memmem.h>

#undef interface

namespace audio {
    struct WavFormatDeleter {
        void operator()(ADPCMWAVEFORMAT* ptr) CNOEXCEPT { free(ptr); }
    };
    using WavFormatPtr = std::unique_ptr<ADPCMWAVEFORMAT, WavFormatDeleter>;

    // A wrapper for resources created by windres
    struct GlobalResource {
        GlobalResource(const std::string& name, const std::string& extension);
        NODISCARD std::size_t size() CNOEXCEPT;

    protected:
        HRSRC _resource;
        HGLOBAL _resource_handle;
        LPVOID _resource_data;
        DWORD _resource_size;
    };

    struct AudioResource : GlobalResource {
        AudioResource(const std::string& name, const std::string& extension)
        : GlobalResource(name, extension), _wav_fmt(create_wav()) {
            _filename = name + '.' + extension;
            load_wfx();
            load_data();
        }

        NODISCARD PADPCMWAVEFORMAT get_fmt() CNOEXCEPT {
            return _wav_fmt.get();
        }

        NODISCARD const std::vector<BYTE>& get_data() CNOEXCEPT {
            return _data_buf;
        }

        NODISCARD std::string get_filename() CNOEXCEPT {
            return _filename;
        }

    protected:
        static PADPCMWAVEFORMAT create_wav() NOEXCEPT;
        void load_wfx() NOEXCEPT;
        void load_data() NOEXCEPT;

    private:
        std::string _filename;
        WavFormatPtr _wav_fmt;
        std::vector<BYTE> _data_buf;
    };
}

#endif //PROJECT3_TEST_AUDIO_CORE_HPP
