#include "core.hpp"

namespace audio {
    GlobalResource::GlobalResource(const std::string& name, const std::string& extension) {
        _resource = $invoke_winapi(FindResourceA, NULL)(nullptr, name.c_str(), extension.c_str());
        _resource_handle = $invoke_winapi(LoadResource, NULL)(nullptr, _resource);
        _resource_data = LockResource(_resource_handle);
        _resource_size = SizeofResource(nullptr, _resource);
    }

    std::size_t GlobalResource::size() CNOEXCEPT {
        return _resource_size;
    }


    PADPCMWAVEFORMAT AudioResource::create_wav() NOEXCEPT {
        // see https://learn.microsoft.com/en-us/previous-versions/ee415686(v=vs.85)
        constexpr ADPCMCOEFSET coeff_data[] = {{256, 0}, {512, -256}, {0,0}, {192,64}, {240,0}, {460, -208}, {392,-232}};
        auto* adpcm = typed_alloc<ADPCMWAVEFORMAT*>(sizeof(ADPCMWAVEFORMAT) + sizeof(coeff_data));

        adpcm->wSamplesPerBlock = 512;
        adpcm->wNumCoef = ARRAY_SIZE(coeff_data);
        std::memcpy(adpcm->aCoef, coeff_data, sizeof(coeff_data));

        return adpcm;
    }

    void AudioResource::load_wfx() NOEXCEPT {
        DWORD wfx_size = 0;
        auto* fmt = (BYTE*)memmem(_resource_data, _resource_size, "fmt ", 4);

        if(fmt == nullptr) {
            std::string err = "fmt chunk not found for '" + _filename + "'.";
            FATAL(err);
        }

        std::memcpy(&wfx_size, fmt + sizeof(DWORD), sizeof(DWORD));
        std::memcpy(&_wav_fmt->wfx, fmt + sizeof(DWORD) + sizeof(DWORD), wfx_size);
    }

    void AudioResource::load_data() NOEXCEPT {
        DWORD data_size = 0;
        auto* data = (BYTE*)memmem(_resource_data, _resource_size, "data", 4);

        if(data == nullptr) {
            std::string err = "data chunk not found for '" + _filename + "'.";
            FATAL(err);
        }

        std::memcpy(&data_size, data + sizeof(DWORD), sizeof(DWORD));
        _data_buf.resize(data_size);
        std::memcpy(_data_buf.data(), data + sizeof(DWORD) + sizeof(DWORD), data_size);
    }
}