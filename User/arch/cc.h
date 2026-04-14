#include "arch/sys_arch.h"

#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

#include <stdint.h>

typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;

#define BYTE_ORDER LITTLE_ENDIAN

#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

#endif