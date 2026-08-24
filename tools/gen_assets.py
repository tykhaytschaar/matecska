from sprites import *
from PIL import Image

# effekt-ikonok egy 16x16-os frame-ben: bal oszlop = sziv, jobb = csepp
# (8x16 OBJ modban a bal fel onmagaban hasznalhato sziv-sprite-nak stb.)
FX_ICONS = [HEART[y] + DROP[y] for y in range(8)] + ["." * 16] * 8

FRAMES = [
    ("cat_down_a", CAT_DOWN_A), ("cat_down_b", CAT_DOWN_B),
    ("cat_dl_a", CAT_DL_A),     ("cat_dl_b", CAT_DL_B),
    ("cat_eat_up", CAT_EAT_UP), ("cat_eat_mid", CAT_EAT_MID), ("cat_eat_down", CAT_EAT_DOWN),
    ("cat_happy_a", CAT_HAPPY), ("cat_happy_b", CAT_HAPPY_B),
    ("cat_yuck_a", CAT_YUCK),   ("cat_yuck_b", CAT_YUCK_B),
    ("bowl_fish", BOWL_FISH),   ("bowl_kibble", BOWL_KIBBLE), ("bowl_veg", BOWL_VEG),
    ("fx_icons", FX_ICONS),
]
IDX = {'.': 0, 'L': 1, 'M': 2, 'D': 3}

sheet = Image.new('RGBA', (16 * len(FRAMES), 16), (0, 0, 0, 0))
for i, (_, rows) in enumerate(FRAMES):
    s = render(rows)
    sheet.paste(s, (i * 16, 0), s)
sheet.save('../assets/matecska_sprites_1x.png')

def tile_bytes(rows, ox, oy):
    out = []
    for y in range(8):
        lo = hi = 0
        for x in range(8):
            r = rows[oy + y] if oy + y < len(rows) else ""
            ch = r[ox + x] if ox + x < len(r) else '.'
            v = IDX.get(ch, 0)
            lo = (lo << 1) | (v & 1)
            hi = (hi << 1) | ((v >> 1) & 1)
        out += [lo, hi]
    return out

L = ["/* MATECSKA - sprite tile data",
     "   GB 2bpp, 16x16 metasprite = 2 db 8x16 OBJ (bal, jobb)",
     "   szin 0 = atlatszo, 1 = vilagos, 2 = kozep, 3 = sotet",
     "   Jobbra-le irany: cat_dl_* + OBJ X-flip (S_FLIPX) */",
     "#ifndef MATECSKA_TILES_H", "#define MATECSKA_TILES_H",
     "#include <stdint.h>", ""]
for name, rows in FRAMES:
    data = []
    for cx in (0, 8):
        data += tile_bytes(rows, cx, 0) + tile_bytes(rows, cx, 8)
    L.append("const uint8_t %s[64] = {" % name)
    for i in range(0, len(data), 12):
        chunk = ",".join("0x%02X" % b for b in data[i:i+12])
        L.append("    " + chunk + ("," if i + 12 < len(data) else ""))
    L.append("};\n")
L += ["/* animacios sorrend */",
      "const uint8_t anim_eat[4]   = {4,5,6,5};   /* 6 kepkocka/frame, 3x */",
      "const uint8_t anim_happy[4] = {7,8,8,7};   /* f2-f3: -2/-4 px Y offset */",
      "const uint8_t anim_yuck[4]  = {9,10,9,10}; /* f2/f4: -1/+1 px X offset */",
      "", "#endif"]
open('../src/sprites.h', 'w').write("\n".join(L))
print("frames:", len(FRAMES))
