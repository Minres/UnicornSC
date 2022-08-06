
TARGET  = hello
C_SRCS  = $(wildcard *.c) 
HEADERS = $(wildcard *.h)
CFLAGS += -O0 -g 

BOARD=iss
LINK_TARGET=link
RISCV_ARCH:=rv32i
RISCV_ABI:=ilp32
LDFLAGS := -march=$(RISCV_ARCH) -mabi=$(RISCV_ABI)
LDFLAGS += -Wl,--wrap=printf

compiler := $(shell which riscv32-unknown-elf-gcc)
TOOL_DIR=$(dir $(compiler))

TRIPLET=riscv32-unknown-elf
BSP_BASE = ../bsp
include $(BSP_BASE)/env/common-gcc.mk
