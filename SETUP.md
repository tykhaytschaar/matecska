# Fejlesztői környezet telepítése (macOS, Apple Silicon)

A kötelező rész négy tétel: Xcode CLT, GBDK-2020, Python+Pillow, SameBoy.
Minden más opcionális, csak akkor kell, ha odaérsz.

## 1. Xcode Command Line Tools

`make` és `gcc`(=clang) — a `make test` ezzel fordítja a feladatgenerátort.

```sh
xcode-select --install
```

## 2. GBDK-2020 (a GB toolchain)

Az arm64 release-t használd. A binárisok aláíratlanok, a quarantine-t
le kell szedni, különben a Gatekeeper blokkolja őket.

```sh
cd ~
curl -LO https://github.com/gbdk-2020/gbdk-2020/releases/latest/download/gbdk-macos-arm64.tar.gz
tar xzf gbdk-macos-arm64.tar.gz          # ~/gbdk mappa jön létre
xattr -dr com.apple.quarantine ~/gbdk
echo 'export GBDK_HOME=$HOME/gbdk' >> ~/.zshrc
source ~/.zshrc
```

## 3. Python 3 + Pillow (asset-generátorok)

```sh
python3 -m pip install Pillow
```

Ha „externally managed environment" hibát kapsz:
`brew install pillow`, vagy csinálj venv-et a repo mellé.

## 4. SameBoy (emulátor — ez az alap)

```sh
brew install --cask sameboy
```

## Ellenőrzés

```sh
make test    # feladatgenerátor natívan, GBDK nélkül is mennie kell
make         # build/matecska.gb
make run     # SameBoy-ban elindul
```

---

## Opcionális: PyBoy (headless játék-teszt)

A repo tesztszkriptjei PyBoy-jal, emulátor-ablak nélkül játsszák végig a
ROM-ot. Dedikált venv-be megy, nem a rendszer-Pythonba:

```sh
python3 -m venv .venv
.venv/bin/pip install pyboy pillow
```

A ROM CGB-kompatibilis, a PyBoy a fejléc alapján Game Boy Color módban
bootol. A monokróm (DMG) utat kényszerített módban lehet nézni:
`PyBoy(rom, cgb=False, bootrom=<site-packages>/pyboy/core/bootrom_dmg.bin)`
— a `tools/test_e2e.py` és a `tools/shots.py --dmg` ezt csinálja.

## Opcionális: hUGETracker (zeneszerkesztés)

Csak ha zenét írsz. Töltsd le a Mac buildet a
[releases oldalról](https://github.com/SuperDisk/hUGETracker/releases)
(1.0.4 óta van), és itt is kell a quarantine-feloldás:

```sh
xattr -dr com.apple.quarantine /Applications/hUGETracker.app
```

A lejátszó driver (hUGEDriver) nem telepítendő — vendorolva kerül majd
a repóba (`lib/hugedriver/`), lásd a CLAUDE.md Hang szekcióját.

## Opcionális: Emulicious (C-forrásszintű debug)

Csak akkor, ha a SameBoy asm-szintű debuggere már kevés. Java kell hozzá,
de **ne** rendszerszintű: hordozható JRE egy mappában.

```sh
mkdir -p ~/tools/jre
# adoptium.net → Temurin JRE (aarch64/mac, .tar.gz), kicsomagolva ide
# emulicious.net → Emulicious.zip, kicsomagolva pl. ~/tools/emulicious-ba
~/tools/jre/bin/java -jar ~/tools/emulicious/Emulicious.jar
```

Hozzá: VS Code-ban az „Emulicious Debugger" kiegészítő, és a ROM-ot
`-debug` flaggel kell buildelni (`.cdb`/`.noi` szimbólumfájlok).
