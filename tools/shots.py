#!/usr/bin/env python3
"""Kepernyokepek a jatekbol PyBoy-jal (emulator-ablak nelkul) - a szinek
(CGB-palettak) megitelesehez es a DMG/CGB osszehasonlitashoz.

    .venv/bin/python tools/shots.py          # CGB-mod (a ROM fejlece szerint)
    .venv/bin/python tools/shots.py --dmg    # kenyszeritett DMG-mod (A/B)

Kimenet: build/shots/<mod>_<kepernyo>.png (3x nagyitva). Elotte: make.
A helyes valaszokat a test_e2e savestate-probaja adja."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import test_e2e as T

def main():
    from pyboy import PyBoy
    import pyboy as _pyboy
    dmg = "--dmg" in sys.argv
    out = os.path.join(T.ROOT, "build", "shots"); os.makedirs(out, exist_ok=True)
    tag = "dmg" if dmg else "cgb"
    ram = T.ROM.replace(".gb", ".ram")
    if os.path.exists(ram): os.remove(ram)
    if dmg:
        boot = os.path.join(os.path.dirname(_pyboy.__file__), "core", "bootrom_dmg.bin")
        py = PyBoy(T.ROM, window="null", cgb=False, bootrom=boot)
    else:
        py = PyBoy(T.ROM, window="null")
    py.set_emulation_speed(0)
    g = T.Game(py, T.load_symbols())

    def shot(name):
        g.tick(6)
        img = py.screen.image.convert("RGB")
        img = img.resize((img.width * 3, img.height * 3), 0)   # NEAREST
        img.save(os.path.join(out, "%s_%s.png" % (tag, name)))
        print("  ", tag, name)

    g.tick(140); shot("01_cim")
    while g.state() != T.ST["MAINMENU"]: g.press("start")
    shot("02_fomenu")
    g.press("a"); g.wait_state(lambda s: s == T.ST["TOPIC"]); shot("03_temak")
    g.select_topic(0); shot("04_kerdes")
    # jo valasz reakcioja (sziv) + rossz valasz (csepp, reveal)
    g.answer_correctly(); shot("05_kerdes2")
    for guess in range(3):
        import io
        st = io.BytesIO(); py.save_state(st)
        end = g.try_bowl(guess)
        if end == T.ST["REVEAL"]:
            shot("06_reveal"); break
        st.seek(0); py.load_state(st); g.tick(4)
    g.press("a"); g.wait_state(lambda s: s == T.ST["TOPIC"])
    # tema teljesitese -> uzenet
    T.complete_topic(g, 1); shot("07_uzenet")
    py.stop(save=False)
    print("kesz:", out)

if __name__ == "__main__":
    main()
