#!/usr/bin/env python3
"""MATECSKA headless E2E teszt (PyBoy, emulator-ablak nelkul).

Vegigjatszik egy teljes szintet es ellenorzi a temankenti szintlepes-logikat:
  - uj jateknal a valasztoban minden temanal 3-as szamlalo (ennyi hibatlan
    szett kell meg), a kijelolt sor invertalt
  - a helyes valaszt savestate-visszatoltessel talalja meg (rossz utan
    rollback), igy minden szett hibatlan; a szamlalo szettenkent csokken
  - 3 hibatlan szett -> MSG allapot (TELJESITVE), A-ra tovabb, a szamlalo
    eltunik (teljesitett temanal nincs szam)
  - mentes/folytatas: ujrainditas utan FOLYTATAS visszahozza a szamlalokat
    es a szorzotabla-maszkot (SRAM)
  - mind a 6 tema utan szintlepes: g_level 1->2, g_lives 3->4, szamlalok vissza

Futtatas a repo gyokerebol (elotte: make):
    .venv/bin/python tools/test_e2e.py

A WRAM-cimeket a build/matecska.noi szimbolumfajlbol olvassa (Makefile:
-Wl-j), igy kodvaltozas utan sem kell kezzel frissiteni.
"""
import io, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ROM = os.path.join(ROOT, "build", "matecska.gb")
NOI = os.path.join(ROOT, "build", "matecska.noi")
ST = dict(TITLE=0, MAINMENU=1, TOPIC=2, QUESTION=3, WALK=4, EAT=5, REACT=6,
          REVEAL=7, MSG=8, GAMEOVER=9)
TILE_DIGIT0 = 24          # font_map['0' - 32]
INV_BASE = 203            # render.c: futasidoben invertalt keszlet
TOPICS = 6

def die(msg):
    print("FAIL:", msg); sys.exit(1)

def load_symbols():
    syms = {}
    for line in open(NOI):
        m = re.match(r"DEF _(\w+) 0x([0-9A-Fa-f]+)", line)
        if m: syms[m.group(1)] = int(m.group(2), 16)
    for need in ("g_state", "g_lives", "g_level", "g_cursor", "mg_mul_seen"):
        if need not in syms: die("hianyzo szimbolum a .noi-ban: " + need)
    return syms

class Game:
    def __init__(self, py, sym):
        self.py, self.sym = py, sym
    def mem(self, name): return self.py.memory[self.sym[name]]
    def state(self): return self.mem("g_state")
    def tick(self, n=1):
        for _ in range(n): self.py.tick()
    def press(self, btn, hold=3, settle=3):
        self.py.button_press(btn); self.tick(hold)
        self.py.button_release(btn); self.tick(settle)
    def wait_state(self, pred, timeout=4000):
        for _ in range(timeout):
            if pred(self.state()): return self.state()
            self.tick()
        die("timeout: state=%d" % self.state())
    def mul_mask(self):
        a = self.sym["mg_mul_seen"]
        return bytes(self.py.memory[a:a + 16])
    def remaining(self):
        """a valaszto szamlaloi: {temaindex: hatralevo hibatlan szettek};
        teljesitett tema (nincs szam) nem szerepel"""
        out = {}
        for i in range(TOPICS):
            t = self.py.memory[0x9800 + (4 + 2 * i) * 32 + 1]
            if t >= INV_BASE: t -= INV_BASE          # kijelolt (invertalt) sor
            if TILE_DIGIT0 < t <= TILE_DIGIT0 + 9:
                out[i] = t - TILE_DIGIT0
        return out

    def row_inverted(self, i):
        return self.py.memory[0x9800 + (4 + 2 * i) * 32 + 3] >= INV_BASE

    def to_topic_from_boot(self, cont=False):
        self.tick(120)
        while self.state() != ST["MAINMENU"]:
            self.press("start")
        if cont:
            self.press("down")
        self.press("a")
        self.wait_state(lambda s: s == ST["TOPIC"])

    def select_topic(self, t):
        for _ in range(TOPICS): self.press("up", settle=2)
        for _ in range(t): self.press("down", settle=2)
        self.press("a")
        self.wait_state(lambda s: s == ST["QUESTION"])

    def try_bowl(self, guess):
        """megeteti a macskat a guess talbol; a lezaro allapotot adja"""
        while self.mem("g_cursor") > guess: self.press("left", settle=2)
        while self.mem("g_cursor") < guess: self.press("right", settle=2)
        self.press("a")
        self.wait_state(lambda s: s != ST["QUESTION"])
        return self.wait_state(lambda s: s not in
                               (ST["WALK"], ST["EAT"], ST["REACT"]))

    def answer_correctly(self):
        """az elso olyan tal, ami nem REVEAL-be (rossz valasz) visz"""
        for guess in range(3):
            st = io.BytesIO(); self.py.save_state(st)
            end = self.try_bowl(guess)
            if end != ST["REVEAL"]:
                return end                    # QUESTION / TOPIC / MSG
            st.seek(0); self.py.load_state(st); self.tick(4)
        die("mindharom valasz rossznak bizonyult")

    def answer_wrong(self):
        """az elso REVEAL-be vivo tal; A-val tovabb a reveal-rol"""
        for guess in range(3):
            st = io.BytesIO(); self.py.save_state(st)
            end = self.try_bowl(guess)
            if end == ST["REVEAL"]:
                self.press("a")
                return self.wait_state(lambda s: s != ST["REVEAL"])
            st.seek(0); self.py.load_state(st); self.tick(4)
        die("mindharom valasz jonak bizonyult")

    def check_progress(self, q):
        """a kerdeskepernyo jobb felso '<q+1>/5' kijelzese (2. tile-sor)"""
        row = 0x9800 + 2 * 32
        if self.py.memory[row + 17] != TILE_DIGIT0 + 1 + q \
           or self.py.memory[row + 19] != TILE_DIGIT0 + 5:
            die("szett-allas: %d/5-ot vartam a kijelzon" % (q + 1))

    def play_set(self):
        end, q = ST["QUESTION"], 0
        while end == ST["QUESTION"]:
            self.check_progress(q)
            end = self.answer_correctly()
            q += 1
        return end

def complete_topic(g, t, sets_needed=3):
    for s in range(sets_needed):
        g.select_topic(t)
        end = g.play_set()
        want = ST["TOPIC"] if s < sets_needed - 1 else ST["MSG"]
        if end != want:
            die("t%d s%d: %d allapotot vartam, %d jott" % (t, s, want, end))
        if end == ST["TOPIC"]:
            got = g.remaining().get(t)
            if got != sets_needed - 1 - s:
                die("t%d: %d hatralevot vartam, a valaszto %s-t mutat"
                    % (t, sets_needed - 1 - s, got))
    return end

def main():
    from pyboy import PyBoy
    sym = load_symbols()
    ram = ROM.replace(".gb", ".ram")
    if os.path.exists(ram): os.remove(ram)

    py = PyBoy(ROM, window="null"); py.set_emulation_speed(0)
    g = Game(py, sym)
    g.to_topic_from_boot()
    if g.remaining() != {i: 3 for i in range(TOPICS)}:
        die("uj jateknal minden temanal 3-nak kene allnia: %s" % g.remaining())
    if not g.row_inverted(0):
        die("a kijelolt (0.) sornak invertaltnak kene lennie")
    if g.mem("g_level") != 1 or g.mem("g_lives") != 3: die("kezdoertekek")
    print("OK: uj jatek, 6x3-as szamlalo, kijeloles invertalva")

    complete_topic(g, 0)
    g.press("a"); g.wait_state(lambda s: s == ST["TOPIC"])
    if g.remaining() != {i: 3 for i in range(1, TOPICS)}:
        die("a 0. tema szamlalojanak el kene tunnie: %s" % g.remaining())
    print("OK: 0. tema teljesitve, szamlaloja eltunt")

    # teljesitett temaban a hiba nem visz eletet, es a hiba utan a szett
    # azonnal megszakad (vissza a valasztora)
    g.select_topic(0)
    end = g.answer_wrong()
    if end != ST["TOPIC"]:
        die("hiba utan azonnal TOPIC-ot vartam, %d jott" % end)
    if g.mem("g_lives") != 3:
        die("kesz temaban a hibanak nem szabadna eletet vinnie: lives=%d"
            % g.mem("g_lives"))
    print("OK: hiba utan a szett megszakad; kesz temaban eletet sem visz")

    # B a kerdesnel: visszalepes a valasztora, elet nem vesz el, a tema
    # szamlaloja (friss temanal) 3 marad
    g.select_topic(1)
    g.press("b")
    g.wait_state(lambda s: s == ST["TOPIC"])
    if g.mem("g_lives") != 3 or g.remaining().get(1) != 3:
        die("B-visszalepes utan lives=%d, rem=%s" % (g.mem("g_lives"), g.remaining()))
    print("OK: B a kerdesnel -> vissza a valasztora")

    mul_before = g.mul_mask()
    py.stop()                                 # battery .ram kiirasa
    py = PyBoy(ROM, window="null"); py.set_emulation_speed(0)
    g = Game(py, sym)
    g.to_topic_from_boot(cont=True)
    if g.remaining() != {i: 3 for i in range(1, TOPICS)}:
        die("FOLYTATAS utan rossz szamlalok: %s" % g.remaining())
    if g.mul_mask() != mul_before:
        die("mg_mul_seen nem jott vissza a mentesbol")
    if g.mem("g_level") != 1: die("FOLYTATAS: szint")
    print("OK: mentes/folytatas (jelolesek + szorzotabla-maszk vissza)")

    for t in range(1, TOPICS):
        complete_topic(g, t)
        g.press("a"); g.wait_state(lambda s: s == ST["TOPIC"])
        if t < TOPICS - 1:
            want = {i: 3 for i in range(t + 1, TOPICS)}
            if g.remaining() != want:
                die("t%d utan szamlalok: %s != %s" % (t, g.remaining(), want))
            print("OK: %d. tema teljesitve (%d van hatra)" % (t, len(want)))

    if g.mem("g_level") != 2: die("szintlepes elmaradt: level=%d" % g.mem("g_level"))
    if g.mem("g_lives") != 4: die("+1 elet elmaradt: lives=%d" % g.mem("g_lives"))
    if g.remaining() != {i: 3 for i in range(TOPICS)}:
        die("szintlepes utan minden szamlalonak 3-ra kene allnia")
    print("OK: minden tema kesz -> szintlepes (level=2, lives=4), szamlalok vissza")

    # szintvesztes: 2-es szinten az osszes elet (4) elvesztese nem game
    # over, hanem MSG + egy szint vissza, teljesitesek es eletek vissza
    # (minden hiba megszakitja a szettet, ezert ujra kell inditani)
    for _ in range(4):
        g.select_topic(0)
        end = g.answer_wrong()
    if end != ST["MSG"]: die("szintvesztesnel MSG-t vartam, %d jott" % end)
    g.press("a"); g.wait_state(lambda s: s == ST["TOPIC"])
    if g.mem("g_level") != 1: die("szintvesztes: level=%d" % g.mem("g_level"))
    if g.mem("g_lives") != 3: die("szintvesztes: lives=%d" % g.mem("g_lives"))
    if g.remaining() != {i: 3 for i in range(TOPICS)}:
        die("szintvesztes utan minden szamlalonak 3-nak kene lennie")
    print("OK: eletvesztes 2-es szinten -> szintvesztes (level=1, lives=3)")

    # 1-es szinten viszont game over
    for _ in range(3):
        g.select_topic(0)
        end = g.answer_wrong()
    if end != ST["GAMEOVER"]: die("1-es szinten GAMEOVER-t vartam, %d jott" % end)
    g.press("a"); g.wait_state(lambda s: s == ST["MAINMENU"])
    print("OK: eletvesztes 1-es szinten -> game over -> fomenu")

    py.stop(save=False)
    print("\nE2E OK")

if __name__ == "__main__":
    main()
