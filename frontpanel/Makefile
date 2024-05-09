LIB = libfrontpanel.a

SRCS = jpeg.cpp lpanel.cpp lp_gfx.cpp lp_main.cpp lp_utils.cpp lp_window.cpp \
	lp_switch.cpp lp_font.cpp lp_materials.cpp

CORE_DIR = ../z80core

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
PLAT_INCS = -I/usr/local/include
endif
ifeq ($(TARGET_OS),LINUX)
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include
endif
###
### END O/S DEPENDENT VARIABLES
###

CXXSTDS = -std=c++03 -D_DEFAULT_SOURCE # -D_XOPEN_SOURCE=700L
CXXWARNS = -Wall -Wextra

# Production - the default
CXXFLAGS = -O3 $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) -U_FORTIFY_SOURCE $(PLAT_INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CXXFLAGS = -O3 $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(PLAT_INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CXXFLAGS = -O -g $(CXXSTDS) $(CXXWARNS) $(PLAT_CXXFLAGS) $(PLAT_INCS)
endif

OBJS = $(SRCS:.cpp=.o)

all: $(LIB)
	@echo
	@echo "Done."
	@echo

$(LIB): $(OBJS)
	@rm -f $@
	ar cq $@ $(OBJS)

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@

-include $(SRCS:.cpp=.d)

build: _rm_obj all

clean: _rm_obj _rm_deps
	rm -f $(LIB)

_rm_obj:
	rm -f $(OBJS)

_rm_deps:
	rm -f $(SRCS:.cpp=.d)

allclean: clean

.PHONY: all build clean allclean
