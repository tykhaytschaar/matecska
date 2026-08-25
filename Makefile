# MATECSKA - GBDK-2020 build
# GBDK_HOME-ot allitsd be, pl.:
#   export GBDK_HOME=$HOME/gbdk
GBDK_HOME ?= $(HOME)/gbdk
LCC        = $(GBDK_HOME)/bin/lcc

# verzio: egyetlen forras a git v* tag (tools/version.sh -> build/version.h;
# a gen_web.py is ezt a szkriptet hivja)
VERSION   := $(strip $(shell sh tools/version.sh))

ROM        = build/matecska.gb
SRC        = src/main.c src/mathgen.c src/render.c src/font_data.c
HDRS       = $(wildcard src/*.h)
OBJ        = $(SRC:src/%.c=build/%.o)

# ROM fejlec: nev, MBC5+RAM+BATTERY (SRAM mentes a FOLYTATAS-hoz)
# -Wl-j: .noi szimbolumfajl a headless teszthez (tools/test_e2e.py)
LCCFLAGS   = -Wl-yt0x1B -Wl-yo4 -Wl-ya1 -Wl-j -Wm-yn"MATECSKA"

EMU       ?= open -a SameBoy

all: $(ROM)

build:
	mkdir -p build

# mindig kiertekelodik, de csak akkor irodik ujra, ha valtozott (ne forduljon
# ujra minden objektum feleslegesen)
build/version.h: FORCE | build
	@printf '/* GENERALT a git tagbol (tools/version.sh) - ne szerkeszd */\n#define MATECSKA_VERSION "%s"\n' "$(VERSION)" > $@.tmp
	@cmp -s $@.tmp $@ || { mv $@.tmp $@; echo "version.h: $(VERSION)"; }
	@rm -f $@.tmp
FORCE:

build/%.o: src/%.c $(HDRS) build/version.h | build
	$(LCC) $(LCCFLAGS) -Ibuild -c -o $@ $<

$(ROM): $(OBJ)
	$(LCC) $(LCCFLAGS) -o $@ $(OBJ)

run: $(ROM)
	$(EMU) $(ROM)

# sprite/font tile-adat es PNG-k ujrageneralasa a tools/ pixel-terkepekbol
assets:
	cd tools && python3 gen_assets.py && python3 gen_font.py

design:
	cd tools && python3 gen_design.py

# bongeszos jatszooldal: web/index.html (wasmBoy + ROM egyetlen fajlban)
web: $(ROM)
	python3 tools/gen_web.py

# headless vegigjatszas PyBoy-jal (elotte: make; venv: lasd SETUP.md)
e2e: $(ROM)
	.venv/bin/python tools/test_e2e.py

# a feladatgenerator gcc-vel, GBDK nelkul
test:
	mkdir -p build
	gcc -Wall -Wextra -O2 -o build/test_mathgen src/test_mathgen.c src/mathgen.c
	./build/test_mathgen

clean:
	rm -rf build

.PHONY: all run assets design test e2e web clean FORCE
