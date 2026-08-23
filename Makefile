# MATECSKA - GBDK-2020 build
# GBDK_HOME-ot allitsd be, pl.:
#   export GBDK_HOME=$HOME/gbdk
GBDK_HOME ?= $(HOME)/gbdk
LCC        = $(GBDK_HOME)/bin/lcc

ROM        = build/matecska.gb
SRC        = src/main.c src/mathgen.c
OBJ        = $(SRC:src/%.c=build/%.o)

# ROM fejlec: nev, MBC (mentes majd MBC3+RAM+BATTERY kell)
LCCFLAGS   = -Wl-yt0x00 -Wl-yo4 -Wl-ya0 -Wm-yn"MATECSKA"

EMU       ?= open -a Emulicious

all: $(ROM)

build:
	mkdir -p build

build/%.o: src/%.c | build
	$(LCC) $(LCCFLAGS) -c -o $@ $<

$(ROM): $(OBJ)
	$(LCC) $(LCCFLAGS) -o $@ $(OBJ)

run: $(ROM)
	$(EMU) $(ROM)

# sprite tile-adat es PNG-k ujragenerálasa a tools/ pixel-terkepekbol
assets:
	cd tools && python3 gen_assets.py

design:
	cd tools && python3 gen_design.py

# a feladatgenerator gcc-vel, GBDK nelkul
test: 
	mkdir -p build
	gcc -Wall -Wextra -O2 -o build/test_mathgen src/test_mathgen.c src/mathgen.c
	./build/test_mathgen

clean:
	rm -rf build

.PHONY: all run assets design test clean
