// See LICENSE for license details.

#ifndef _ISS_PLATFORM_H
#define _ISS_PLATFORM_H

// Some things missing from the official encoding.h
#define MCAUSE_INT         0x80000000
#define MCAUSE_CAUSE       0x7FFFFFFF

#include "rtl/const.h"
/****************************************************************************
 * Platform definitions
 *****************************************************************************/

#define MEM_BASE_ADDR _AC(0x00000000,UL)

// Helper functions
#define _REG32(p, i) (*(volatile uint32_t *) ((p) + (i)))
#define _REG32P(p, i) ((volatile uint32_t *) ((p) + (i)))

// Misc

#include <stdint.h>

void init_pll(void);
unsigned long get_cpu_freq(void);
unsigned long get_timer_freq(void);
uint64_t get_timer_value(void);

#endif /* _ISS_PLATFORM_H */
