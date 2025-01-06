# use SDL2 instead of X11
WANT_SDL ?= NO

LIB = libfrontpanel.a

SRCS = jpeg.c lpanel.c lp_gfx.c lp_main.c lp_utils.c lp_window.c \
	lp_switch.c lp_font.c lp_materials.c

CORE_DIR = ../z80core

include $(CORE_DIR)/Makefile.in-os

###
### SDL2/X11 PLATFORM VARIABLES
###
ifeq ($(WANT_SDL),YES)
PLAT_DEFS = -DWANT_SDL
ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include/SDL2
else ifeq ($(TARGET_OS),LINUX)
PLAT_INCS = -I/usr/include/SDL2
else ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -F/Library/Frameworks -I/Library/Frameworks/SDL2.framework/Headers \
	-I/Library/Frameworks/SDL2_image.framework/Headers
endif
endif
###
### END SDL2 VARIABLES
###

CPPFLAGS = $(PLAT_DEFS) $(PLAT_INCS)

CSTDS = -std=c99 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CWARNS = -Wall -Wextra -Wwrite-strings

# Production - the default
COPTS = -O3 -U_FORTIFY_SOURCE

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
COPTS = -O3 -fstack-protector-all -D_FORTIFY_SOURCE=2
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
COPTS = -O -g
endif

CFLAGS = $(CSTDS) $(COPTS) $(CWARNS)

OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

all: $(LIB)

$(LIB): $(OBJS)
	@rm -f $@
	ar cq $@ $(OBJS)

%.d: %.c
	@$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

-include $(DEPS)

build: _rm_obj all

install:

uninstall:

clean: _rm_obj _rm_deps
	rm -f $(LIB)

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(DEPS)

distclean: clean

.PHONY: all build install uninstall clean distclean
