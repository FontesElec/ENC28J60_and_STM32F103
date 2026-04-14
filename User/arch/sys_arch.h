#ifndef SYS_ARCH_H
#define SYS_ARCH_H


#include <stdint.h>
#include <stm32f10x.h>
#include "core_cm3.h"

typedef uint32_t sys_prot_t;

#define SYS_ARCH_DECL_PROTECT(lev) sys_prot_t lev

#define SYS_ARCH_PROTECT(lev)        \
  do {                               \
    lev = __get_PRIMASK();           \
    __disable_irq();                 \
  } while(0)

#define SYS_ARCH_UNPROTECT(lev)      \
  do {                               \
    __set_PRIMASK(lev);              \
  } while(0)

#endif
