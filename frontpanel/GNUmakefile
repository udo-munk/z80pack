LIB = libfrontpanel.a

SRCS = jpeg.cpp lpanel.cpp lp_gfx.cpp lp_main.cpp lp_utils.cpp lp_window.cpp \
	lp_switch.cpp lp_font.cpp lp_materials.cpp

CORE_DIR = ../z80core

CXX = g++

###
### START O/S PLATFORM DEPENDANT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

ifeq ($(TARGET_OS),BSD)
CXX = c++
PLAT_INCS = -I/usr/local/include
endif
ifeq ($(TARGET_OS),WIN32)
EXEC := $(EXEC:=.exe)
.LIBPATTERNS += lib%.dll.a
endif
ifeq ($(TARGET_OS),LINUX)
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include
.LIBPATTERNS += lib%.dylib
endif
###
### END O/S DEPENDANT VARIABLES
###

CXXWARNS = -Wall -Wextra

# Production - the default
CXXFLAGS = -O3 $(CXXWARNS) $(PLAT_CXXFLAGS) -U_FORTIFY_SOURCE $(PLAT_INCS)

# Development - use `MODE=DEV make build`
ifeq ($(MODE),DEV)
CXXFLAGS = -O3 $(CXXWARNS) $(PLAT_CXXFLAGS) -fstack-protector-all -D_FORTIFY_SOURCE=2 $(PLAT_INCS)
endif

# Debug - use `DEBUG=1 make build`
ifneq ($(DEBUG),)
CXXFLAGS = -O -g $(PLAT_CXXFLAGS) $(PLAT_INCS)
endif

OBJS = $(SRCS:.cpp=.o)

all: $(LIB)
	@echo
	@echo "Done."
	@echo

$(LIB): $(OBJS)
	rm -f $@
	ar cq $@ $(OBJS)

%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@

-include $(SRCS:.cpp=.d)

build: _rm_obj all

clean: _rm_obj _rm_deps

_rm_obj:
	rm -f *.o

_rm_deps:
	rm -f *.d

allclean: clean
	rm -f $(LIB)

.PHONY: all build clean allclean
