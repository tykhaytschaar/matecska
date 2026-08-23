from PIL import Image, ImageDraw, ImageFont
from sprites import *
from fonts import F3, F5

DEF = ImageFont.load_default()


def txt(img, s, x, y, col=C_D, sc=1, font=F3, gw=3, gh=5, adv=4):
    px = img.load()
    for ch in s.upper():
        g = font.get(ch, font.get(' ', ["." * gw] * gh))
        for gy, row in enumerate(g):
            for gx, c in enumerate(row):
                if c == 'D':
                    for a in range(sc):
                        for b in range(sc):
                            X, Y = x + gx * sc + a, y + gy * sc + b
                            if 0 <= X < img.width and 0 <= Y < img.height:
                                px[X, Y] = col
        x += adv * sc
    return x


def tw(s, adv=4, sc=1):
    return len(s) * adv * sc


def paste(img, rows, x, y):
    s = render(rows)
    img.paste(s, (x, y), s)


def hud(img, score="0120", lives=3):
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 159, 13], fill=C_M)
    d.line([0, 14, 159, 14], fill=C_D)
    txt(img, "SCORE " + score, 4, 4, col=BG)
    for i in range(lives):
        h = render(HEART)
        img.paste(h, (148 - i * 10, 3), h)


def screen():
    return Image.new('RGB', (160, 144), BG)


# =========================================================== 1. CIMKEPERNYO
t = screen()
d = ImageDraw.Draw(t)
d.rectangle([0, 0, 159, 143], outline=C_M)
title = "MATECSKA"
W5 = len(title) * 6 * 2
x0 = (160 - W5) // 2
txt(t, title, x0 + 2, 24, col=C_M, sc=2, font=F5, gw=5, gh=7, adv=6)   # shadow
txt(t, title, x0, 22, col=C_D, sc=2, font=F5, gw=5, gh=7, adv=6)
sub = "MATEK MACSKAKKAL"
txt(t, sub, (160 - tw(sub)) // 2, 46, col=C_M)
for sx, sy, sym in [(8, 60, "+"), (144, 60, "-"), (8, 96, "*"), (144, 96, "=")]:
    txt(t, sym, sx, sy, col=C_M, sc=2)
paste(t, CAT_HAPPY, 72, 68)
paste(t, BOWL_FISH, 46, 74)
paste(t, BOWL_KIBBLE, 100, 74)
h = render(HEART)
t.paste(h, (66, 60), h)
st = "START"
txt(t, st, (160 - tw(st, sc=2)) // 2, 110, col=C_D, sc=2)
cp = "(C) 2026"
txt(t, "C 2026", (160 - tw("C 2026")) // 2, 132, col=C_M)

# ====================================================== 2. KERDES KEPERNYO
BX = [10, 72, 134]
BY = 96
ANS = [("A", "90"), ("B", "54"), ("C", "47")]


def question_screen(q, sel=None, score="0120", lives=3):
    s = screen()
    hud(s, score, lives)
    txt(s, q, (160 - tw(q, sc=2)) // 2, 22, col=C_D, sc=2)
    dd = ImageDraw.Draw(s)
    for i, bx in enumerate(BX):
        paste(s, [BOWL_FISH, BOWL_VEG, BOWL_KIBBLE][i], bx, BY)
        lab = ANS[i][0] + " " + ANS[i][1]
        txt(s, lab, bx + 8 - tw(lab) // 2, 118, col=C_D)
        if sel == i:
            dd.rectangle([bx - 3, BY + 5, bx + 14, 126], outline=C_D)
    return s


s_q = question_screen("36+9*2=")
for i, bx in enumerate(BX):
    d2 = ImageDraw.Draw(s_q)
    for k in range(1, 7):
        tt = k / 7
        X = round((72 + 8) + (bx + 6 - 80) * tt)
        Y = round(62 + (BY - 62) * tt)
        d2.rectangle([X, Y, X + 1, Y + 1], fill=C_M)
paste(s_q, CAT_DOWN_A, 72, 46)

# jo valasz: C) 47
s_ok = question_screen("36+9*2=", sel=2, score="0140")
paste(s_ok, CAT_HAPPY, 134, 80)
s_ok.paste(h, (126, 64), h)
s_ok.paste(h, (152, 60), h)
txt(s_ok, "NYAMI!", 118, 48, col=C_D)

# rossz valasz: A) 90
s_bad = question_screen("36+9*2=", sel=0, lives=2)
paste(s_bad, CAT_YUCK, 10, 80)
dr = render(DROP)
s_bad.paste(dr, (28, 64), dr)
txt(s_bad, "FUJ!", 8, 48, col=C_D)

# ============================================================= 3. ANIMACIOK
EAT = [("1", CAT_EAT_UP), ("2", CAT_EAT_MID), ("3", CAT_EAT_DOWN), ("2", CAT_EAT_MID)]
HAP = [("1", CAT_HAPPY), ("2", CAT_HAPPY_B), ("3", CAT_HAPPY_B), ("2", CAT_HAPPY)]
YUK = [("1", CAT_YUCK), ("2", shift(CAT_YUCK_B, -1, 0)), ("3", CAT_YUCK), ("2", shift(CAT_YUCK_B, 1, 0))]
WLK = [("1", CAT_DOWN_A), ("2", CAT_DOWN_B), ("1", CAT_DL_A), ("2", CAT_DL_B)]

strips = [
    ("JARAS  (le / balra-le)   8 frame/mp", WLK),
    ("EVES   loop 3x, 6 frame/kock", EAT),
    ("ORUL  f2-f3 kodbol -2/-4px Y offset (ugras)", HAP),
    ("FANYALOG  fejrazas 2x", YUK),
]

SC = 5
CELL = 16 * SC
GAPX = 12
ROWH = CELL + 46
AW = 24 + 4 * CELL + 3 * GAPX
AH = 30 + len(strips) * ROWH
anim = Image.new('RGB', (AW, AH), C_M)
ad = ImageDraw.Draw(anim)
ad.text((12, 8), "MATECSKA - ANIMACIOK (16x16, 2 db 8x16 OBJ)", fill=BG, font=DEF)
y = 30
for name, frames in strips:
    ad.text((12, y), name, fill=BG, font=DEF)
    yy = y + 16
    x = 12
    for i, (lab, rows) in enumerate(frames):
        ad.rectangle([x - 1, yy - 1, x + CELL, yy + CELL], fill=BG, outline=C_D)
        sp = render(rows, SC)
        anim.paste(sp, (x, yy), sp)
        ad.text((x, yy + CELL + 4), "f%s" % lab, fill=BG, font=DEF)
        x += CELL + GAPX
    y += ROWH

# ================================================================= COMPOSE
def board(screens, labels, title, path, sc=2):
    W = 160 * sc + 24
    H = 34 + len(screens) * (144 * sc + 30)
    b = Image.new('RGB', (W, H), C_M)
    bd = ImageDraw.Draw(b)
    bd.text((12, 10), title, fill=BG, font=DEF)
    yy = 34
    for s, lab in zip(screens, labels):
        bd.text((12, yy), lab, fill=BG, font=DEF)
        yy += 14
        b.paste(s.resize((160 * sc, 144 * sc), Image.NEAREST), (12, yy))
        bd.rectangle([11, yy - 1, 12 + 160 * sc, yy + 144 * sc], outline=BG)
        yy += 144 * sc + 16
    b.save(path)
    return b


board([t], ["160x144"], "MATECSKA - CIMKEPERNYO", '../design/cimkepernyo.png', sc=3)
board([s_q, s_ok, s_bad],
      ["1. KERDES - a macska meg a start pozicioban",
       "2. JO valasz (C-47) - odaer, eszik, orul",
       "3. ROSSZ valasz (A-90) - fanyalog, -1 elet"],
      "MATECSKA - JATEKMENET", '../design/jatekmenet.png', sc=2)
anim.save('../design/animaciok.png')
print("ok")
