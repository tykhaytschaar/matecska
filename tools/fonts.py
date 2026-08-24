F3 = {
    'A': ["DDD", "D.D", "DDD", "D.D", "D.D"],
    'B': ["DD.", "D.D", "DD.", "D.D", "DD."],
    'C': ["DDD", "D..", "D..", "D..", "DDD"],
    'D': ["DD.", "D.D", "D.D", "D.D", "DD."],
    'E': ["DDD", "D..", "DDD", "D..", "DDD"],
    'F': ["DDD", "D..", "DDD", "D..", "D.."],
    'G': ["DDD", "D..", "D.D", "D.D", "DDD"],
    'H': ["D.D", "D.D", "DDD", "D.D", "D.D"],
    'I': ["DDD", ".D.", ".D.", ".D.", "DDD"],
    'J': ["..D", "..D", "..D", "D.D", "DDD"],
    'K': ["D.D", "D.D", "DD.", "D.D", "D.D"],
    'L': ["D..", "D..", "D..", "D..", "DDD"],
    'M': ["D.D", "DDD", "DDD", "D.D", "D.D"],
    'N': ["D.D", "DDD", "DDD", "DDD", "D.D"],
    'O': ["DDD", "D.D", "D.D", "D.D", "DDD"],
    'P': ["DDD", "D.D", "DDD", "D..", "D.."],
    'R': ["DDD", "D.D", "DDD", "DD.", "D.D"],
    'S': ["DDD", "D..", "DDD", "..D", "DDD"],
    'T': ["DDD", ".D.", ".D.", ".D.", ".D."],
    'U': ["D.D", "D.D", "D.D", "D.D", "DDD"],
    'V': ["D.D", "D.D", "D.D", "D.D", ".D."],
    'Y': ["D.D", "D.D", "DDD", ".D.", ".D."],
    'Z': ["DDD", "..D", ".D.", "D..", "DDD"],
    '0': ["DDD", "D.D", "D.D", "D.D", "DDD"],
    '1': [".D.", "DD.", ".D.", ".D.", "DDD"],
    '2': ["DDD", "..D", "DDD", "D..", "DDD"],
    '3': ["DDD", "..D", "DDD", "..D", "DDD"],
    '4': ["D.D", "D.D", "DDD", "..D", "..D"],
    '5': ["DDD", "D..", "DDD", "..D", "DDD"],
    '6': ["DDD", "D..", "DDD", "D.D", "DDD"],
    '7': ["DDD", "..D", "..D", "..D", "..D"],
    '8': ["DDD", "D.D", "DDD", "D.D", "DDD"],
    '9': ["DDD", "D.D", "DDD", "..D", "DDD"],
    '+': ["...", ".D.", "DDD", ".D.", "..."],
    '-': ["...", "...", "DDD", "...", "..."],
    '*': ["...", "D.D", ".D.", "D.D", "..."],
    '=': ["...", "DDD", "...", "DDD", "..."],
    '?': ["DDD", "..D", ".DD", "...", ".D."],
    '!': [".D.", ".D.", ".D.", "...", ".D."],
    ')': [".D.", "..D", "..D", "..D", ".D."],
    ':': ["...", ".D.", "...", ".D.", "..."],
    '.': ["...", "...", "...", "...", ".D."],
    '/': ["..D", "..D", ".D.", "D..", "D.."],
    ' ': ["...", "...", "...", "...", "..."],
}

# 3x7 ekezetes nagybetuk: 2 sor ekezet + a 3x5-os torzs.
# Az ekezet tulnyulhat a 3 px-es torzson (a tile 8 szeles): a dupla ekezet
# ket parhuzamos, dontott vonas 4 px-en - igy ranezesre mas, mint az umlaut.
def _acc(accent2rows, base):
    return accent2rows + F3[base]

F3_ACC = {}          # kulcs: unicode betu, ertek: 7 soros pixel-terkep
def _mk_acc():
    acute  = ["..D", ".D."]
    uml    = ["...", "D.D"]
    dacute = [".D.D", "D.D."]
    for ch, acc, base in [
        ('Á', acute, 'A'), ('É', acute, 'E'), ('Í', acute, 'I'),
        ('Ó', acute, 'O'), ('Ö', uml, 'O'), ('Ő', dacute, 'O'),
        ('Ú', acute, 'U'), ('Ü', uml, 'U'), ('Ű', dacute, 'U'),
    ]:
        F3_ACC[ch] = _acc(acc, base)
_mk_acc()

# teljes 8x8-as specialis tile-ok (HUD ikonok, keret, vonal)
_E = "........"
_F = "DDDDDDDD"
_L = "D......."
_R = ".......D"

SPECIAL = {
    'HEART': [
        "........",
        ".DD.DD..",
        "DDDDDDD.",
        "DDDDDDD.",
        ".DDDDD..",
        "..DDD...",
        "...D....",
        "........",
    ],
    'RULE':     [_F, _E, _E, _E, _E, _E, _E, _E],   # HUD alatti vonal
    'FRAME_TL': [_F, _L, _L, _L, _L, _L, _L, _L],
    'FRAME_T':  [_F, _E, _E, _E, _E, _E, _E, _E],
    'FRAME_TR': [_F, _R, _R, _R, _R, _R, _R, _R],
    'FRAME_L':  [_L, _L, _L, _L, _L, _L, _L, _L],
    'FRAME_R':  [_R, _R, _R, _R, _R, _R, _R, _R],
    'FRAME_BL': [_L, _L, _L, _L, _L, _L, _L, _F],
    'FRAME_B':  [_E, _E, _E, _E, _E, _E, _E, _F],
    'FRAME_BR': [_R, _R, _R, _R, _R, _R, _R, _F],
}

# 5x7 title font
F5 = {
    'M': ["D...D", "DD.DD", "D.D.D", "D...D", "D...D", "D...D", "D...D"],
    'A': [".DDD.", "D...D", "D...D", "DDDDD", "D...D", "D...D", "D...D"],
    'T': ["DDDDD", "..D..", "..D..", "..D..", "..D..", "..D..", "..D.."],
    'E': ["DDDDD", "D....", "D....", "DDDD.", "D....", "D....", "DDDDD"],
    'C': [".DDD.", "D...D", "D....", "D....", "D....", "D...D", ".DDD."],
    'S': [".DDDD", "D....", "D....", ".DDD.", "....D", "....D", "DDDD."],
    'K': ["D...D", "D..D.", "D.D..", "DD...", "D.D..", "D..D.", "D...D"],
}
