#ifndef _RTL_CONST_H
#define _RTL_CONST_H

#ifdef __ASSEMBLER__
#define _AC(X,Y)        X
#define _AT(T,X)        X
#else
#define _AC(X,Y)        (X##Y)
#define _AT(T,X)        ((T)(X))
#endif /* !__ASSEMBLER__*/

#define _BITUL(x)       (_AC(1,UL) << (x))
#define _BITULL(x)      (_AC(1,ULL) << (x))

#define UART0_BASE_ADDR 0xffff0000ULL

#define UART_REG_TXFIFO         0x00
#define UART_REG_RXFIFO         0x04
#define UART_REG_TXCTRL         0x08
#define UART_REG_RXCTRL         0x0c
#define UART_REG_IE             0x10
#define UART_REG_IP             0x14
#define UART_REG_DIV            0x18
#define UART_TXEN               0x1

#define UART0_REG(ADDR) *((volatile uint32_t*) (UART0_BASE_ADDR + ADDR))
#endif /* _RTL_CONST_H */
