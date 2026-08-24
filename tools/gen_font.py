# src/font.h generalasa a fonts.py keszleteibol.
# Tile 0 = ures (szokoz). A 3x5 torzs a tile 2..6. soraban ul, az ekezet
# a 0..1. sorban. Az 5x7-es logo-betuk a keszlet vegen.
#
# print_at kodolas:
#   - ASCII 32..95: font_map[c - 32] adja a tile indexet
#   - >= 0x80: kozvetlen tile index (c & 0x7F) - az ekezetes makrok igy mennek
from fonts import F3, F3_ACC, F5, SPECIAL

# determinisztikus sorrend: szokoz az elso (tile 0)
F3_ORDER = [' '] + [c for c in
    "ABCDEFGHIJKLMNOPRSTUVYZ0123456789+-*=?!):." if c in F3]
ACC_ORDER = ['Á', 'É', 'Í', 'Ó', 'Ö', 'Ő', 'Ú', 'Ü', 'Ű']
ACC_MACRO = {'Á': '_AA', 'É': '_EE', 'Í': '_II', 'Ó': '_OO', 'Ö': '_OE',
             'Ő': '_OEE', 'Ú': '_UU', 'Ü': '_UE', 'Ű': '_UEE'}
LOGO = "MATECSKA"


def tile_2bpp(rows8):
    """8 db max 8 karakteres sor ('.'/'D') -> 16 bajt 2bpp (szin 3)."""
    out = []
    for y in range(8):
        r = rows8[y] if y < len(rows8) else ""
        b = 0
        for x in range(8):
            if x < len(r) and r[x] == 'D':
                b |= 0x80 >> x
        out += [b, b]          # lo es hi bit is 1 -> szin 3 (sotet)
    return out


def place(rows, w, top):
    """Glifa 8x8-ba: x offset 1, elso sor a `top` sorba kerul."""
    grid = ["........"] * 8
    for i, r in enumerate(rows):
        y = top + i
        if 0 <= y < 8:
            grid[y] = ("." + r + "." * 8)[:8]
    return grid


tiles = []       # 16 bajtos tile-ok listaja
charmap = {}     # karakter -> tile index

for ch in F3_ORDER:
    charmap[ch] = len(tiles)
    tiles.append(tile_2bpp(place(F3[ch], 3, 2)))

acc_index = {}
for ch in ACC_ORDER:
    acc_index[ch] = len(tiles)
    tiles.append(tile_2bpp(place(F3_ACC[ch], 3, 0)))

# kesobb potolt szimbolumok - az ekezetek UTAN, hogy a mar kiosztott
# tile-indexek (es a 0xAB.. ekezet-kodok) ne tolodjanak el
for ch in "/":
    charmap[ch] = len(tiles)
    tiles.append(tile_2bpp(place(F3[ch], 3, 2)))

# a szoveges keszlet vege - a render.c ebbol tudja, mennyit kell invertalnia
text_ntiles = len(tiles)

# inverz (sotet savos HUD) keszlet: vilagos betu sotet alapon
INV_CHARS = " 0123456789PONTSZ"
inv_index = {}
for ch in INV_CHARS:
    inv_index[ch] = len(tiles)
    tiles.append([b ^ 0xFF for b in tile_2bpp(place(F3[ch], 3, 2))])

special_index = {}
for name, rows in SPECIAL.items():
    special_index[name] = len(tiles)
    tiles.append(tile_2bpp(rows))
special_index['HEART_INV'] = len(tiles)
tiles.append([b ^ 0xFF for b in tile_2bpp(SPECIAL['HEART'])])

logo_base = len(tiles)
logo_glyphs = []
for ch in LOGO:
    if ch not in logo_glyphs:
        logo_glyphs.append(ch)
        tiles.append(tile_2bpp(place(F5[ch], 5, 0)))
logo_tiles = [logo_base + logo_glyphs.index(ch) for ch in LOGO]

assert len(tiles) < 128, "print_at 0x80-as kodolasa miatt max 127 tile"

# --- font_map: ASCII 32..95 -> tile ---------------------------------------
fmap = [0] * 64
for ch, idx in charmap.items():
    o = ord(ch)
    assert 32 <= o < 96, ch
    fmap[o - 32] = idx

# --- font.h: interfesz (makrok + extern deklaraciok) ----------------------
flat_len = len(tiles) * 16

L = ["/* MATECSKA - GENERALT font interfesz (tools/gen_font.py) */",
     "#ifndef MATECSKA_FONT_H", "#define MATECSKA_FONT_H",
     "#include <stdint.h>", "",
     "#define FONT_NTILES %d" % len(tiles),
     "#define FONT_TEXT_NTILES %d  /* a print_at-keszlet (invertalando) */"
     % text_ntiles,
     "#define LOGO_LEN    %d" % len(LOGO), ""]
for name, idx in special_index.items():
    L.append("#define TILE_%-10s %d" % (name, idx))
L.append("")

L.append("/* ekezetes betuk: string-literalba fuzve, pl. \"SZORZ\" _OO \"T\" _AA \"BLA\" */")
for ch in ACC_ORDER:
    L.append('#define %-4s "\\x%02x"  /* %s */'
             % (ACC_MACRO[ch], 0x80 | acc_index[ch], ch))
L += ["",
      "extern const uint8_t font_tiles[%d];" % flat_len,
      "extern const uint8_t font_map[64];      /* ASCII 32..95 -> tile */",
      "extern const uint8_t font_map_inv[64];  /* ua. sotet HUD-savra */",
      "extern const uint8_t logo_map[LOGO_LEN];",
      "", "#endif"]

open('../src/font.h', 'w').write("\n".join(L) + "\n")

# --- font_data.c: a tile-adat ----------------------------------------------
flat = [b for t in tiles for b in t]
L = ["/* MATECSKA - GENERALT font tile-adat (tools/gen_font.py) */",
     '#include "font.h"', ""]
L.append("const uint8_t font_tiles[%d] = {" % len(flat))
for i in range(0, len(flat), 12):
    L.append("    " + ",".join("0x%02X" % b for b in flat[i:i+12])
             + ("," if i + 12 < len(flat) else ""))
L.append("};\n")

L.append("/* ASCII 32..95 -> tile index (0 = nincs glifa) */")
L.append("const uint8_t font_map[64] = {")
for i in range(0, 64, 12):
    L.append("    " + ",".join("%3d" % v for v in fmap[i:i+12])
             + ("," if i + 12 < 64 else ""))
L.append("};\n")

imap = [inv_index[' ']] * 64
for ch, idx in inv_index.items():
    imap[ord(ch) - 32] = idx
L.append("/* inverz valtozat a sotet HUD-savhoz (ismeretlen -> sotet ures) */")
L.append("const uint8_t font_map_inv[64] = {")
for i in range(0, 64, 12):
    L.append("    " + ",".join("%3d" % v for v in imap[i:i+12])
             + ("," if i + 12 < 64 else ""))
L.append("};\n")

L.append('/* a "MATECSKA" logo tile indexei (5x7 font) */')
L.append("const uint8_t logo_map[LOGO_LEN] = {%s};"
         % ",".join(str(t) for t in logo_tiles))

open('../src/font_data.c', 'w').write("\n".join(L) + "\n")
print("font tiles:", len(tiles))
