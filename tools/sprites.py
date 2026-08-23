from PIL import Image

BG   = (0x9B, 0xBC, 0x0F)
C_L  = (0x8B, 0xAC, 0x0F)
C_M  = (0x30, 0x62, 0x30)
C_D  = (0x0F, 0x38, 0x0F)
PAL = {'L': C_L, 'M': C_M, 'D': C_D}

def render(rows, scale=1):
    h, w = len(rows), max(len(r) for r in rows)
    img = Image.new('RGBA', (w, h), (0, 0, 0, 0))
    px = img.load()
    for y, row in enumerate(rows):
        for x, ch in enumerate(row):
            if ch in PAL:
                px[x, y] = PAL[ch] + (255,)
    return img.resize((w*scale, h*scale), Image.NEAREST) if scale > 1 else img

# ============================================================ LE (front walk)
CAT_DOWN_A = [
    ".D..........D...",
    ".DD........DD...",
    ".DMD......DMD...",
    ".DMMDDDDDDMMD...",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDLLLLDDLD...",
    ".DLDDLLLLDDLD..D",
    ".DLLLLDDLLLLD.DD",
    ".DLLLDLLDLLLD.DD",
    "..DDLLLLLLDD..DD",
    "...DLLLLLLD...DD",
    "...DLLLLLLD..DD.",
    "...DLLLLLLDDDD..",
    "...DDDDDDDD.....",
    "..DDDD...DD.....",
]
CAT_DOWN_B = [
    ".D..........D...",
    ".DD........DD...",
    ".DMD......DMD...",
    ".DMMDDDDDDMMD...",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDLLLLDDLD...",
    ".DLDDLLLLDDLD.D.",
    ".DLLLLDDLLLLDDD.",
    ".DLLLDLLDLLLDDD.",
    "..DDLLLLLLDDDD..",
    "...DLLLLLLDDD...",
    "...DLLLLLLD.....",
    "...DLLLLLLD.....",
    "...DDDDDDDD.....",
    "...DD...DDDD....",
]

# ======================================================= BALRA-LE (3/4 view)
CAT_DL_A = [
    "D........D......",
    "DD......DMD.....",
    "DMD....DMMD.....",
    "DMMDDDDDMMD.....",
    "DMLLLLLLLMD.....",
    "DLLLLLLLLLD.....",
    "DLDDLLLDDLD.....",
    "DLDDLLLDDLD...D.",
    "DLLLDDLLLLD..DD.",
    "DLLLDLLLLLD.DDD.",
    ".DDLLLLLLLDDDDD.",
    "..DLLLLLLLLLLLD.",
    "..DLLLLLLLLLLLD.",
    "..DLLLLLLLLLLDD.",
    "..DDDDDDDDDDD...",
    "..DDD.DD..DDD...",
]
CAT_DL_B = [
    "D........D......",
    "DD......DMD.....",
    "DMD....DMMD.....",
    "DMMDDDDDMMD.....",
    "DMLLLLLLLMD.....",
    "DLLLLLLLLLD.....",
    "DLDDLLLDDLD.....",
    "DLDDLLLDDLD.DDD.",
    "DLLLDDLLLLDDDDD.",
    "DLLLDLLLLLDDDD..",
    ".DDLLLLLLLDDD...",
    "..DLLLLLLLLLLD..",
    "..DLLLLLLLLLLD..",
    "..DLLLLLLLLLDD..",
    "..DDDDDDDDDDD...",
    "..DD..DDD..DD...",
]

# ================================================================ REAKCIOK
CAT_HAPPY = [
    ".D..........D...",
    ".DD........DD...",
    ".DMD......DMD...",
    ".DMMDDDDDDMMD...",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD.DD",
    ".DLLDLLLLDLLD.DD",
    ".DLDLDLLDLDLD.DD",
    ".DLLLLDDLLLLD.DD",
    ".DLLLDMMDLLLD.DD",
    "..DDLLDDLLDD..DD",
    "...DLLLLLLD..DD.",
    "...DLLLLLLD.DD..",
    "...DLLLLLLDDD...",
    "...DDDDDDDD.....",
    "..DDDD...DDD....",
]
CAT_YUCK = [
    "................",
    "................",
    ".DD........DD...",
    ".DMMDDDDDDMMD...",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDDLLDDDLD...",
    ".DLLLLLLLLLLD...",
    ".DLLLLDDLLLLD...",
    ".DLDDDDDDDDLD..D",
    "..DDLMMMMLDD..DD",
    "...DLMMMMLD..DD.",
    "...DLLLLLLD.DD..",
    "...DLLLLLLDDD...",
    "...DDDDDDDD.....",
    "..DDD.....DDD...",
]

# =============================================================== TALAK 16x16
BOWL_FISH = [
    "................",
    "...DDDDD....DD..",
    "..DLLLLLDD.DMMD.",
    ".DLDLLLLLLDMMMD.",
    "..DLLLLLLLDDMMD.",
    "...DDDDDDD..DD..",
    "DDDDDDDDDDDD....",
    "DMMMMMMMMMMD....",
    "DLMMMMMMMMLD....",
    "DLLMMMMMMLLD....",
    ".DLLLMMLLLD.....",
    ".DDLLLLLLDD.....",
    "..DDDDDDDD......",
    "................",
    "................",
    "................",
]
BOWL_KIBBLE = [
    "................",
    "................",
    "....DD..DD......",
    "...DLLDDLLD.....",
    "..DDLLLLLLDD....",
    "..DLLDLLDLLD....",
    "DDDDDDDDDDDD....",
    "DMMMMMMMMMMD....",
    "DLMMMMMMMMLD....",
    "DLLMMMMMMLLD....",
    ".DLLLMMLLLD.....",
    ".DDLLLLLLDD.....",
    "..DDDDDDDD......",
    "................",
    "................",
    "................",
]
BOWL_VEG = [
    "................",
    ".....D..D.......",
    "....DMDDMD......",
    ".....DMMD.......",
    "....DLLLLD......",
    "....DLLLLD......",
    "DDDDDLLLLDDD....",
    "DMMMMDLLDMMD....",
    "DLMMMMDDMMLD....",
    "DLLMMMMMMLLD....",
    ".DLLLMMLLLD.....",
    ".DDLLLLLLDD.....",
    "..DDDDDDDD......",
    "................",
    "................",
    "................",
]

# ================================================================ IKONOK 8x8
HEART = [
    ".DD..DD.",
    "DMMDDMMD",
    "DMMMMMMD",
    "DMMMMMMD",
    ".DMMMMD.",
    "..DMMD..",
    "...DD...",
    "........",
]
DROP = [
    "...DD...",
    "..DMMD..",
    "..DMMD..",
    ".DMMMMD.",
    ".DMLMMD.",
    ".DMLMMD.",
    "..DMMD..",
    "...DD...",
]

# =============================================================== HELPERS
def shift(rows, dx=0, dy=0, w=16, h=16):
    out = []
    for y in range(h):
        sy = y - dy
        line = ""
        for x in range(w):
            sx = x - dx
            if 0 <= sy < len(rows) and 0 <= sx < len(rows[sy]):
                line += rows[sy][sx]
            else:
                line += "."
        out.append(line)
    return out

def mirror(rows):
    return [r[::-1].ljust(16, '.') for r in [r.ljust(16, '.') for r in rows]]

# ============================================================ EVES (3 frame)
CAT_EAT_UP = [
    ".D..........D.DD",
    ".DD........DD.DD",
    ".DMD......DMD.DD",
    ".DMMDDDDDDMMDDD.",
    ".DMMLLLLLLMMDDD.",
    ".DLLLLLLLLLLD...",
    ".DLDDLLLLDDLD...",
    ".DLDDLLLLDDLD...",
    ".DLLLLDDLLLLD...",
    ".DLLLDLLDLLLD...",
    "..DDLLLLLLDD....",
    "...DLLLLLLD.....",
    "...DLLLLLLD.....",
    "...DLLLLLLD.....",
    "...DDDDDDDD.....",
    "..DDD.....DDD...",
]
CAT_EAT_MID = [
    "..............DD",
    ".D..........D.DD",
    ".DD........DD.DD",
    ".DMD......DMDDD.",
    ".DMMDDDDDDMMDDD.",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDLLLLDDLD...",
    ".DLDDLLLLDDLD...",
    ".DLLLLDDLLLLD...",
    ".DLLLDMMDLLLD...",
    "..DDLLMMLLDD....",
    "...DLLLLLLD.....",
    "...DLLLLLLD.....",
    "...DDDDDDDD.....",
    "..DDD.....DDD...",
]
CAT_EAT_DOWN = [
    "..............DD",
    "..............DD",
    ".D..........D.DD",
    ".DD........DD.DD",
    ".DMD......DMDDD.",
    ".DMMDDDDDDMMDDD.",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDLLLLDDLD...",
    ".DLLLLLLLLLLD...",
    ".DLLDMMMMDLLD...",
    "..DDLMMMMLDD....",
    "...DDLMMLDD.....",
    "...DDLLLLDD.....",
    "...DDDDDDDD.....",
    "..DDD.....DDD...",
]

# ========================================================== ORUL (3 frame)
CAT_HAPPY_B = [
    "................",
    ".D..........D.DD",
    ".DD........DD.DD",
    ".DMD......DMD.DD",
    ".DMMDDDDDDMMD.DD",
    ".DMMLLLLLLMMDDD.",
    ".DLLDLLLLDLLDDD.",
    ".DLDLDLLDLDLD...",
    ".DLLLLDDLLLLD...",
    ".DLLDMMMMDLLD...",
    "..DDLMMMMLDD....",
    "...DLLMMLLD.....",
    "..DDLLLLLLDD....",
    "..DLLDDDDLLD....",
    "..DDD....DDD....",
    "................",
]

# ====================================================== FANYALOG (3 frame)
CAT_YUCK_B = [
    "................",
    "................",
    ".DD........DD...",
    ".DMMDDDDDDMMD...",
    ".DMMLLLLLLMMD...",
    ".DLLLLLLLLLLD...",
    ".DLDDDLLDDDLD...",
    ".DLLLLLLLLLLD...",
    ".DLLLLDDLLLLD...",
    ".DLDDDDDDDDLD..D",
    "..DDLMMMMLDD..DD",
    "...DLMMMMLD..DD.",
    "...DLLMMLLD.DD..",
    "...DLLMMLLDDD...",
    "...DDDMMDDD.....",
    "..DDD.DD..DDD...",
]
