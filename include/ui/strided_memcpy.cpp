#include "strided_memcpy.hpp"
#include <algorithm>

#define DST_CLAMP(idx) ((dst_offset.idx >= 0) * dst_offset.idx)
#define NEGATIVE_CLAMP(idx) ((dst_offset.idx < 0) * (-dst_offset.idx))
#define OFFSCREEN(idx) (dst_offset.idx > dst_size.idx) or ((dst_offset.idx + src_size.idx) < 0)

#define LOCK_WIDTH(idx) LOCK_WIDTH_LOW(idx) + LOCK_WIDTH_MID(idx) + LOCK_WIDTH_HIGH(idx)
#define LOCK_WIDTH_LOW(idx) ((src_size.idx < 0) * (src_size.idx))
#define LOCK_WIDTH_MID(idx) ((src_size.idx < (dst_size.idx - dst_offset.idx)) * src_size.idx)
#define LOCK_WIDTH_HIGH(idx) ((src_size.idx >= (dst_size.idx - dst_offset.idx)) * (dst_size.idx - dst_offset.idx))

namespace ui {
    static int get_dst_offset(api::Coords dst_offset, api::Coords dst_size) {
        int raw_offset = DST_CLAMP(x) + (DST_CLAMP(y) * dst_size.x);
        return std::clamp(raw_offset, 0, dst_size.area());
    }

    void VECCALL strided_memcpy(void* RESTRICT dst, const void* RESTRICT src,
    api::Coords dst_size, api::Coords dst_offset, api::Coords src_size, int elem_size) NOEXCEPT {
        if(OFFSCREEN(x) or OFFSCREEN(y)) return;

        const int src_offset_x = NEGATIVE_CLAMP(x);
        const int src_offset_y = NEGATIVE_CLAMP(y);
        if(src_offset_x >= src_size.x or src_offset_y >= src_size.y) return;

        auto* pdst = static_cast<unsigned char*>(dst);
        pdst += get_dst_offset(dst_offset, dst_size) * elem_size;

        const auto* pdst_max = pdst + (dst_size.area() * elem_size);
        const auto* psrc = static_cast<const unsigned char*>(src);

        const int src_width = (LOCK_WIDTH(x) - src_offset_x) * elem_size;
        const int src_height = LOCK_WIDTH(y) - src_offset_y;
        const int src_stride_width = src_size.x;
        const int dst_stride_width = dst_size.x * elem_size;

        for(int idx = src_offset_y; idx < src_height and pdst < pdst_max; ++idx) {
            std::memcpy(pdst, psrc + (src_offset_x * elem_size), src_width);
            psrc += src_stride_width;
            pdst += dst_stride_width;
        }
    }
}