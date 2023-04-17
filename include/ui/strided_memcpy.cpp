#include "strided_memcpy.hpp"

namespace ui {
    void VECCALL strided_memcpy(void* RESTRICT dst, const void* RESTRICT src,
                                api::Coords dst_size, api::Coords dst_offset, api::Coords src_size) NOEXCEPT {
        auto* pdst = static_cast<unsigned char*>(dst);
        const auto* psrc = static_cast<const unsigned char*>(src);

        const int src_width = (src_size.x > dst_size.x - dst_offset.x) ? (dst_size.x - dst_offset.x) : src_size.x;
        const int src_height = (src_size.y > dst_size.y - dst_offset.y) ? (dst_size.y - dst_offset.y) : src_size.y;
        const int src_stride_width = src_size.x;
        const int dst_stride_width = dst_size.x;

        for(int idx = src_width; idx < src_height; ++idx) {
            std::memcpy(pdst, psrc, src_width);
            psrc += src_stride_width;
            pdst += dst_stride_width;
        }
    }
}