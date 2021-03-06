ifndef _MK_LIBWRAP
_MK_LIBWRAP := # defined

LIBWRAP_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
LIBWRAP_DIR := $(LIBWRAP_DIR:/=)

LIBWRAP_SRCS := \
	stdlib/malloc.c \
	sys/open.c \
	sys/lseek.c \
	sys/read.c \
	sys/write.c \
	sys/fstat.c \
	sys/stat.c \
	sys/close.c \
	sys/link.c \
	sys/unlink.c \
	sys/execve.c \
	sys/fork.c \
	sys/getpid.c \
	sys/kill.c \
	sys/wait.c \
	sys/isatty.c \
	sys/times.c \
	sys/sbrk.c \
	sys/_exit.c \
	misc/write_hex.c \
	sys/printf.c

LIBWRAP_SRCS := $(foreach f,$(LIBWRAP_SRCS),$(LIBWRAP_DIR)/$(f))
LIBWRAP_OBJS := $(LIBWRAP_SRCS:.c=.o)

LIBWRAP_SYMS := malloc free \
	open lseek read write fstat stat close link unlink \
	execve fork getpid kill wait \
	isatty times sbrk _exit

LIBWRAP := libwrap.a

LINK_DEPS += $(LIBWRAP)

LDFLAGS += $(foreach s,$(LIBWRAP_SYMS),-Wl,--wrap=$(s))
LDFLAGS += -L. -Wl,--start-group -lwrap -lc -Wl,--end-group

CLEAN_OBJS += $(LIBWRAP_OBJS)

$(LIBWRAP_OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(LIBWRAP): $(LIBWRAP_OBJS)
	$(AR) rcs $@ $^

endif # _MK_LIBWRAP
