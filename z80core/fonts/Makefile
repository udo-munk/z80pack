PROJECT = terminus-font
VERSION = 4.49.1
SFDIR = $(PROJECT)-$(basename $(VERSION))
SRCDIR = $(PROJECT)-$(VERSION)
SRCFILE = $(SRCDIR).tar.gz
URL = https://downloads.sourceforge.net/project/$(PROJECT)/$(SFDIR)/$(SRCFILE)

CFLAGS = -O -Wall -Wextra

all:
	@echo "If you really want to regenerate the fonts use \"make fonts\""

fonts: bdf2c $(SRCFILE)
	@echo "Generating font*.h and OFL.TXT files..."
	@set -e; tar -x -f $(SRCFILE); \
	cd $(SRCDIR); \
	patch -s -p1 -i alt/td1.diff; \
	../bdf2c ter-u12n.bdf > ../font12.h; \
	../bdf2c ter-u14b.bdf > ../font14.h; \
	../bdf2c ter-u16b.bdf > ../font16.h; \
	../bdf2c ter-u18b.bdf > ../font18.h; \
	../bdf2c ter-u20b.bdf > ../font20.h; \
	../bdf2c ter-u22b.bdf > ../font22.h; \
	../bdf2c ter-u24b.bdf > ../font24.h; \
	../bdf2c ter-u28b.bdf > ../font28.h; \
	../bdf2c ter-u32b.bdf > ../font32.h; \
	tr -d '\r' < OFL.TXT > ../OFL.TXT; \
	cd ..; \
	rm -rf $(SRCDIR)

bdf2c: bdf2c.c

$(SRCFILE):
	curl -LO $(URL)

clean:
	rm -f bdf2c $(SRCFILE)

.PHONY: all fonts clean
