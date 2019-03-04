/* crc32_acle.c -- compute the CRC-32 of a data stream
 * Copyright (C) 1995-2006, 2010, 2011, 2012 Mark Adler
 * Copyright (C) 2016 Yang Zhang
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
*/

#if __ARM_FEATURE_CRC32
# include <arm_acle.h>
# ifdef ZLIB_COMPAT
#  include <zconf.h>
# else
#  include <zconf-ng.h>
# endif
# ifdef __linux__
#  include <stddef.h>
# endif
# include "zutil.h"

# define do_crc32h(val, buf, c) do {            \
    MEMCPY(&val, buf, sizeof(val));             \
    c = __crc32h(c, val);                       \
    buf += sizeof(val);                         \
  } while (0)

# define do_crc32w(val, buf, c) do {            \
    MEMCPY(&val, buf, sizeof(val));             \
    c = __crc32w(c, val);                       \
    buf += sizeof(val);                         \
  } while (0)

# define do_crc32d(val, buf, c) do {            \
    MEMCPY(&val, buf, sizeof(val));             \
    c = __crc32d(c, val);                       \
    buf += sizeof(val);                         \
  } while (0)


uint32_t crc32_acle(uint32_t crc, const unsigned char *buf, uint64_t len) {
    uint32_t c = ~crc;

    if (len && ((ptrdiff_t)buf & 1)) {
        c = __crc32b(c, *buf++);
        len--;
    }

    if ((len > sizeof(uint16_t)) && ((ptrdiff_t)buf & 2)) {
        uint16_t val;
        do_crc32h(val, buf, c);
        len -= sizeof(val);
    }

# if defined(__aarch64__)
    if ((len > sizeof(uint32_t)) && ((ptrdiff_t)buf & 4)) {
        uint32_t val;
        do_crc32w(val, buf, c);
        len -= sizeof(val);
    }

#  if defined(UNROLL_MORE)
    while (len >= 4 * sizeof(uint64_t)) {
        uint64_t val;
        do_crc32d(val, buf, c);
        do_crc32d(val, buf, c);
        do_crc32d(val, buf, c);
        do_crc32d(val, buf, c);
        len -= 4 * sizeof(val);
    }
#  endif

    while (len >= sizeof(uint64_t)) {
        uint64_t val;
        do_crc32d(val, buf, c);
        len -= sizeof(val);
    }

    if (len >= sizeof(uint32_t)) {
        uint32_t val;
        do_crc32w(val, buf, c);
        len -= sizeof(val);
    }
# else /* __aarch64__ */

#  if defined(UNROLL_MORE)
    while (len >= 8 * sizeof(uint32_t)) {
        uint32_t val;
        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);

        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);
        do_crc32w(val, buf, c);
        len -= 8 * sizeof(val);
    }
#  endif

    while (len >= sizeof(uint32_t)) {
        uint32_t val;
        MEMCPY(&val, buf, sizeof(val));
        c = __crc32w(c, val);
        len -= sizeof(val);
    }
# endif /* __aarch64__ */

    if (len >= sizeof(uint16_t)) {
        uint16_t val;
        do_crc32h(val, buf, c);
        len -= sizeof(val);
    }

    if (len) {
        c = __crc32b(c, *buf);
    }

    c = ~c;
    return c;
}
#endif /* __ARM_FEATURE_CRC32 */