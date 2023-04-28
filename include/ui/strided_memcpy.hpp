#ifndef PROJECT3_TEST_STRIDED_MEMCPY_HPP
#define PROJECT3_TEST_STRIDED_MEMCPY_HPP

#include <cstring>
#include <api/core.hpp>

namespace ui {
    /**
     * Copies one strided 2D array into another.
     * @param dst         start of destination buffer
     * @param src         start of source buffer
     * @param dst_size    size of destination buffer (origin at 0,0)
     * @param dst_offset  offset into destination buffer
     * @param src_size    size of source buffer (origin at 0,0)
     * @param elem_size   size of each element (sizeof(T))
     */
    void VECCALL strided_memcpy(void* RESTRICT dst, const void* RESTRICT src,
    api::Coords dst_size, api::Coords dst_offset, api::Coords src_size, int elem_size) NOEXCEPT;
}

#endif //PROJECT3_TEST_STRIDED_MEMCPY_HPP
