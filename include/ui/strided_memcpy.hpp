#ifndef PROJECT3_TEST_STRIDED_MEMCPY_HPP
#define PROJECT3_TEST_STRIDED_MEMCPY_HPP

#include <cstring>
#include <api/core.hpp>

namespace ui {
    // dst:         start of destination buffer
    // src:         start of source buffer
    // dst_size:    size of destination buffer (origin at 0,0)
    // dst_offset:  offset into destination buffer
    // src_size:    size of source buffer (origin at 0,0)
    void VECCALL strided_memcpy(void* RESTRICT dst, const void* RESTRICT src,
    api::Coords dst_size, api::Coords dst_offset, api::Coords src_size) NOEXCEPT;
}

#endif //PROJECT3_TEST_STRIDED_MEMCPY_HPP
