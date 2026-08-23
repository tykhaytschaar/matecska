# MATECSKA — projekt brief

Game Boy (DMG) játék: fejszámolós kvíz, ahol egy macska három tál közül
választva válaszol. GBDK-2020 / C.

## A játék

Random matekfeladat jelenik meg, három válaszlehetőséggel. A képernyő alján
három macskatál van, mindegyik alatt egy válasz. A játékos balra/jobbra
mozgatja a kiválasztást, A gombbal véglegesít.

- A macska odasétál a választott tálhoz (le / balra-le / jobbra-le irányban)
- Beleeszik (evés-animáció)
- Ha jó a válasz: **örül** (ugrik, szívek), pont jár
- Ha rossz: **fanyalog** (kilógó nyelv, fejrázás), −1 élet

3 élet, a pontszám nő. Game over → cím képernyő.

## Platform és build

- **Cél:** Game Boy DMG, 160×144, 4 árnyalat, `.gb` ROM
- **Toolchain:** GBDK-2020 (`lcc`), C
- **Fejlesztői gép:** macOS (Apple Silicon) — a GBDK-2020 arm64 release-t használd
- **Emulátor:** Emulicious vagy SameBoy (mindkettő fut macOS-en)
- `GBDK_HOME` env változót be kell állítani, a Makefile arra hivatkozik

```sh
make        # matecska.gb a build/ mappába
make run    # emulátorban indítja
make clean
```

## Hardveres korlátok — ezeket TARTSD BE

- 4 szín összesen. Sprite-nál a 0-s index **átlátszó**, tehát sprite-onként
  csak 3 látható árnyalat van.
- Egyszerre max **40 OBJ**, és **10 OBJ per scanline**. Egy 16×16-os macska
  már 2 OBJ (8×16 módban). Ezért:
  - a **tálak háttér-tile-ként** rajzolódnak, nem sprite-ként
  - a szöveg is háttér-tile
  - sprite csak: macska (2 OBJ) + max 2-3 effekt ikon (szív, csepp)
- VRAM tile-ok: `set_sprite_data` / `set_bkg_data`, tile index 0-255
- `SPRITES_8x16` módot használunk (`SPRITES_8x16;` a `main()` elején)
- VRAM írás csak VBlank alatt biztonságos, vagy használd a GBDK
  `set_*_tiles` hívásait `wait_vbl_done()` után

## Grafika

### Paletta (DMG)
| index | szerep | referencia szín |
|---|---|---|
| 0 | háttér / sprite-nál átlátszó | `#9BBC0F` |
| 1 | világos | `#8BAC0F` |
| 2 | közép | `#306230` |
| 3 | sötét (körvonal) | `#0F380F` |

### Sprite-ok — `src/sprites.h`
Kész 2bpp tile-adat, minden sprite 16×16 = **64 bájt** = 2 db 8×16 OBJ
(bal fél, jobb fél). 14 sprite:

| index | név | szerep |
|---|---|---|
| 0,1 | `cat_down_a/b` | járás lefelé, 2 fázis |
| 2,3 | `cat_dl_a/b` | járás balra-le, 2 fázis |
| 4,5,6 | `cat_eat_up/mid/down` | evés, 3 fázis |
| 7,8 | `cat_happy_a/b` | örül, 2 fázis |
| 9,10 | `cat_yuck_a/b` | fanyalog, 2 fázis |
| 11,12,13 | `bowl_fish/kibble/veg` | tálak |

**Jobbra-le irány nincs külön rajzolva** — a `cat_dl_*` sprite X-flippelve
(`S_FLIPX` az OBJ attribútumban). Így fele annyi tile kell.

### Animációs sorrendek
A `src/sprites.h` végén megvannak tömbként:

```c
anim_eat[4]   = {4,5,6,5};   // 6 kepkocka/frame, 3x lejátszva
anim_happy[4] = {7,8,8,7};   // f2-f3 alatt a rajzolási Y −2 / −4 px (ugrás)
anim_yuck[4]  = {9,10,9,10}; // f2/f4 alatt a rajzolási X −1 / +1 px (fejrázás)
```

Az ugrás és a fejrázás **kódból jön offsetként**, nincs beégetve a sprite-ba.

### Font
Nincs még tile-ként exportálva. A `tools/fonts.py`-ban megvan a 3×5-ös
(szöveg, számok, `+ − × =`) és az 5×7-es (címlogó) készlet Python dict-ként.
**Első feladatok egyike:** exportáld ezeket 8×8-as háttér-tile-okká és írj
egy `print_at(x, y, str)` helper-t.

### Látványterv
A `design/` mappa PNG-i mutatják, hogy néznie kell kinéznie:
- `cimkepernyo.png` — cím, logó, START
- `jatekmenet.png` — kérdés / jó válasz / rossz válasz képernyő
- `animaciok.png` — az összes animációs fázis nagyítva
- `karakterlap.png` — sprite-lap

## Képernyő-layout (160×144, pixelben)

```
y   0–13   HUD sáv (sötét háttér): "SCORE 0120" balra, 1–3 szív jobbra
y  14      elválasztó vonal
y  22–32   feladat, 2× méretű 3×5 font, középre zárva  ("36+9*2=")
y  46–62   macska (16×16), start x = 72
y  96–112  három tál, x = 10 / 72 / 134
y 118–123  válaszok, 3×5 font, tálanként középre  ("A 90")
```

A kijelölés a tál + válasz köré rajzolt keret.

## Kódstruktúra (javasolt)

```
src/
  main.c        állapotgép + fő ciklus
  game.h        közös típusok, konstansok
  sprites.h     GENERÁLT — ne szerkeszd kézzel, lásd tools/
  mathgen.c/h   feladatgenerátor (platform-független, gcc-vel tesztelhető)
  render.c/h    HUD, szöveg, tálak kirajzolása
  anim.c/h      sprite animációs állapotgép
```

### Állapotgép
`TITLE → QUESTION → WALK → EAT → REACT → (QUESTION | GAMEOVER) → TITLE`

## Feladatgenerátor

`src/mathgen.c` már kész és gcc-vel tesztelhető (`make test`).

Kétműveletes kifejezéseket generál helyes műveleti sorrenddel, és a rossz
válaszokat **tipikus tanulói hibákból** származtatja, nem véletlenszerűen:

- balról jobbra számolás (`36+9*2` → `90` a `54` helyett)
- előjelhiba
- ±1 / ±10 elcsúszás
- operandusok felcserélése kivonásnál

Ez a lényeg: a rossz válaszok legyenek hihetőek, különben triviális a játék.

Nehézségi szintek a `difficulty` mezőn keresztül (0–3): egyre nagyobb
számok, majd zárójel és osztás. Minden válasz 0–999 közé essen és a három
válasz legyen különböző.

## Asset-generálás

A sprite-ok Pythonból generálódnak, nem képszerkesztőből. Ha módosítani
kell egy sprite-ot:

1. szerkeszd a pixel-térképet `tools/sprites.py`-ban (`.` = átlátszó,
   `L` = világos, `M` = közép, `D` = sötét)
2. `make assets` → újragenerálja a `src/sprites.h`-t és az `assets/*.png`-t
3. `python3 tools/gen_design.py` → frissíti a `design/` látványterveket

Alternatíva: a `assets/matecska_sprites_1x.png` közvetlenül betölthető
Asepritebe, vagy futtatható rajta a GBDK `png2asset`.

## Amit még nem csináltam meg

- font tile-ok exportja + `print_at()`
- háttér-tile-ok (tál, HUD sáv, keret)
- hang (a GB 4 csatornája: rövid „nyam", öröm-arpeggio, hibahang)
- mentés (SRAM high score) — kell hozzá MBC3+RAM+BATTERY a fejlécben
- a `Makefile` `GBDK_HOME`-ot feltételez, nincs kipróbálva; az első
  buildnél valószínűleg igazítani kell rajta

## Megjegyzés a kódhoz

- SDCC-t használ a GBDK, ami **C89-közeli** és nagyon korlátozott:
  nincs `long long`, a `float` kerülendő, a struct-visszaadás drága.
  Használj `uint8_t`/`int16_t`-t mindenhol, ahol lehet.
- Globális változók olcsóbbak, mint a mély hívási lánc (nincs igazi stack frame).
- `banked` függvények csak akkor, ha tényleg elfogy a 32 KB.
