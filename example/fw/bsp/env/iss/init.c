#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "platform.h"
#include "encoding.h"

extern int main(int argc, char** argv);
extern void trap_entry();

static unsigned long mtime_lo(void)
{
    unsigned long ret;
    __asm volatile("rdtime %0":"=r"(ret));
    return ret;
}


#ifdef __riscv32

static uint32_t mtime_hi(void)
{
    unsigned long ret;
    __asm volatile("rdtimeh %0":"=r"(ret));
    return ret;
}

uint64_t get_timer_value()
{
  while (1) {
    uint32_t hi = mtime_hi();
    uint32_t lo = mtime_lo();
    if (hi == mtime_hi())
      return ((uint64_t)hi << 32) | lo;
  }
}

#else /* __riscv32 */

uint64_t get_timer_value()
{
  return mtime_lo();
}

#endif

unsigned long get_timer_freq()
{
  return 32768;
}

unsigned long get_cpu_freq()
{
  return 10000000;
}

void init_pll(void){

}

#ifdef USE_PLIC
extern void handle_m_ext_interrupt();
#endif

#ifdef USE_M_TIME
extern void handle_m_time_interrupt();
#endif

uintptr_t handle_trap(uintptr_t mcause, uintptr_t epc)
{
  if (0){
#ifdef USE_PLIC
    // External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) == IRQ_M_EXT)) {
    handle_m_ext_interrupt();
#endif
#ifdef USE_M_TIME
    // External Machine-Level interrupt from PLIC
  } else if ((mcause & MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) == IRQ_M_TIMER)){
    handle_m_time_interrupt();
#endif
  }
  else {
    write(1, "trap\n", 5);
    _exit(1 + mcause);
  }
  return epc;
}

void _init()
{
  
#ifndef NO_INIT
  init_pll();
  printf("core freq at %d Hz\n", get_cpu_freq());
  write_csr(mtvec, &trap_entry);
  if (read_csr(misa) & (1 << ('F' - 'A'))) { // if F extension is present
    write_csr(mstatus, MSTATUS_FS); // allow FPU instructions without trapping
    write_csr(fcsr, 0); // initialize rounding mode, undefined at reset
  }
#endif
  
}

void _fini()
{
}
