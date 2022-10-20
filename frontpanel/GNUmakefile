LIB = libfrontpanel.a

SRCS = jpeg.cpp lpanel.cpp lp_gfx.cpp lp_main.cpp lp_utils.cpp lp_window.cpp \
	lp_switch.cpp lp_font.cpp lp_materials.cpp

CORE_DIR = ../z80core

###
### START O/S PLATFORM DEPENDENT VARIABLES
###
include $(CORE_DIR)/Makefile.in-os

CXX = g++
ifeq ($(TARGET_OS),BSD)
CXX = c++
PLAT_INCS = -I/usr/local/include
endif
ifeq ($(TARGET_OS),WIN32)
endif
ifeq ($(TARGET_OS),LINUX)
endif
ifeq ($(TARGET_OS),OSX)
PLAT_INCS = -I/opt/X11/include
endif
###
### END O/S DEPENDENT VARIABLES
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
	@rm -f $@
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

makerules:
	@echo "# AUTOMATICALLY GENERATED DO NOT EDIT" >tmp.rules
	@echo >>tmp.rules;
	@for i in *.d; do \
		cat $$i >>tmp.rules; \
		f=`sed 's/.*: \([^ ]*\).*/\1/;2,$$d' $$i`; \
		if grep -q '\.cpp' $$i; then \
			echo '	$$(CXX) $$(CXXFLAGS) -c -o $$@' $$f >>tmp.rules; \
		else \
			echo '	$$(CC) $$(CFLAGS) -c -o $$@' $$f >>tmp.rules; \
		fi ; \
		echo >>tmp.rules; \
	done
	@echo '# DO NOT PUT ANYTHING AFTER THIS LINE' >>tmp.rules;
	@for i in Makefile.*; do \
		sed '/^# AUTOMATICALLY GENERATED DO NOT EDIT/,$$d' <$$i >tmp.make; \
		cat tmp.rules >>tmp.make; \
		mv -f tmp.make $$i; \
	done
	@rm -f tmp.rules tmp.make

.PHONY: all build clean allclean makerules
