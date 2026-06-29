#ifndef __WZ_ETHERCAT_OS_DEFINITIONS_H
#define __WZ_ETHERCAT_OS_DEFINITIONS_H

#if defined(__GNUC__)
#define PACKED_BEGIN
#define PACKED_BEGIN_4
#define PACKED          __attribute__((__packed__))
#define PACKED_4        __attribute__((aligned(4), packed))
#define PACKED_END
#else
#define PACKED_BEGIN    __pragma(pack(push, 1))
#define PACKED_BEGIN_4  __pragma(pack(push, 4))
#define PACKED
#define PACKED_4
#define PACKED_END      __pragma(pack(pop))
#endif

#endif
