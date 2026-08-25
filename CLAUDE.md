# MATECSKA — projekt brief

Game Boy (DMG) játék: fejszámolós kvíz, ahol egy macska három tál közül
választva válaszol. GBDK-2020 / C.

## A játék

**Címképernyő** (a `design/cimkepernyo.png` kompozíciója): felül a
„MATECSKA" logó **2× méretben** (futásidőben skálázva), alatta a „MATEK
MACSKÁKKAL" alcím (az Á a font ékezet-makrójával: `"MACSK" _AA "KKAL"`). Középen két tál (halas balra, zöldséges jobbra, tile
x=4 ill. x=14, 8–9. sor), köztük a macska sétál oda-vissza, a tálaknál
1-1 másodpercre megállva **enni** (evés-animáció + szív a feje fölött).
A sarkokban díszítő műveleti jelek: `+ −` a 6., `× =` a 11. tile-sorban.
Alul 2× méretű „START" (13–14. sor) és „C 2026" (16. sor). START-ra jön
a főmenü.

**Főmenü:** `UJ JATEK` / `FOLYTATAS`. Folytatásnál a mentésből töltjük
vissza a szintet, az életeket és a pontszámot; új játéknál minden alapra
áll (1. szint, 3 élet, 0 pont). FOLYTATAS csak akkor választható, ha van
érvényes mentés.

**Feladattípus-választó:** a játékos kiválasztja a témát (szorzótábla,
osztás, átváltások, idő… — lásd lejjebb). A kiválasztott témából **5
feladatból álló szett** jön; a szett végén — vagy az első hibánál, ami
azonnal megszakítja a szettet — vissza a típusválasztóra.

Egy feladat: random matekfeladat, három válaszlehetőséggel. A képernyő
alján három macskatál van, mindegyik alatt egy válasz. A játékos
balra/jobbra mozgatja a kiválasztást, A gombbal véglegesít.

- A macska odasétál a választott tálhoz (le / balra-le / jobbra-le irányban)
- Beleeszik (evés-animáció)
- Ha jó a válasz: **örül** (ugrik, szívek), pont jár
- Ha rossz: **fanyalog** (kilógó nyelv, fejrázás), és **megmutatjuk a jó
  választ** — A gombra a szett megszakad, vissza a típusválasztóra
  (élet csak bizonyos esetben vész el, lásd a pontozásnál)

### Pontozás, életek, szintek

- Jó válasz: **szint × 5 pont** (1. szinten 5, 2. szinten 10, …).
  Rossz válasz pontot nem von le.
- **Élet csak akkor vész el hibázásnál, ha az adott témában még mind a 3
  szett teljesítendő** (nincs megkezdett hibátlan sorozat és a téma sincs
  teljesítve). Megkezdett vagy teljesített témában a hiba „ingyenes" —
  életet nem visz.
- **Hibánál a szett azonnal megszakad**: a jó válasz megmutatása után
  vissza a típusválasztóra, és az adott téma hibátlan-sorozata nullázódik.
- **3 élettel** indulunk, a maximum **5 élet**.
- **Téma teljesítése:** egy téma akkor teljesített, ha a játékos **3
  hibátlan (5/5-ös) szettet** old meg belőle zsinórban. Hibás szett csak
  az adott téma sorozatát nullázza; a már teljesített téma teljesítve
  marad a szint végéig.
- **Szintlépés:** ha **minden téma teljesített**, a karakter
  szintet lép, +1 életet kap (max 5-ig), és a teljesítések törlődnek —
  az új szinten elölről indul a kör. A szint felső határa 9.
- A típusválasztóban minden még nem teljesített téma előtt szám mutatja,
  hány hibátlan szett kell még a teljesítéséhez (3→1); teljesített
  témánál nincs szám (0 nem jelenik meg).
- A téma teljesítéséről és a szintlépésről a szett végén **üzenetképernyő**
  jelenik meg (ST_MSG), amit A-val lehet továbbléptetni.
- A generátorok a szintből származtatott nehézséget kapják:
  `difficulty = min(szint − 1, 3)`.
- Ha elfogynak az életek: **szintvesztés** — egy szint vissza, a
  teljesítések és a sorozatok nullázódnak, az életek 3-ra állnak, a
  pontszám megmarad, és a játék üzenet után (A-ra) a típusválasztón
  folytatódik. **Game over csak 1-es szinten van** (onnan nincs lejjebb):
  vissza a főmenübe, a pontszám és a teljesítések nullázódnak, az életek
  3-ra állnak.

### Mentés

SRAM-ba (MBC5+RAM+BATTERY a fejlécben), v2-es layout: szint, életek,
pontszám, teljesített témák bitmaskja, témánkénti hibátlan-szett
sorozatok (`MG_MAX_TOPICS` bájt), a szorzótábla nem-ismétlő maszkja
(`mg_mul_seen`, 16 bájt), magic + verzió + checksum a validáláshoz.
Mentés minden lezárt kérdés után. A FOLYTATAS ebből tölt vissza.

## Platform és build

- **Cél:** Game Boy DMG, 160×144, 4 árnyalat, `.gb` ROM
- **Toolchain:** GBDK-2020 (`lcc`), C
- **Fejlesztői gép:** macOS (Apple Silicon) — a GBDK-2020 arm64 release-t használd
- **Emulátor:** **SameBoy az alap** (natív mac app, `make run` ezt indítja);
  Emulicious csak opcionális extra, lásd lejjebb
- `GBDK_HOME` env változót be kell állítani, a Makefile arra hivatkozik

```sh
make        # matecska.gb a build/ mappába
make run    # SameBoy-ban indítja
make web    # böngészős játszóoldal: web/index.html (lásd lejjebb)
make clean
```

### Webes játszóoldal

`make web` → `tools/gen_web.py` legenerálja a **web/index.html**-t:
egyetlen önálló fájl (generált, gitignore-olt — lokális kipróbáláshoz
`make && make web`), benne a vendorolt **wasmBoy** emulátor-mag
(`web/wasmboy.wasm.umd.js`, a wasm base64-ként beágyazva) és a friss ROM
base64-ként. Fut `file://`-ról és bármilyen statikus hostról (GitHub
Pages). Saját billentyű- (nyilak, A=A, B=B — Z/X is —, Enter=START) és
érintőgomb-kezelés (`WasmBoy.setJoypadState`), a SRAM-mentést 5
másodpercenként `saveLoadedCartridge()` írja a böngésző IndexedDB-jébe,
így a FOLYTATÁS ott is működik. Érintőgombok: D-pad balra, A jobbra-fent /
B balra-lent átlósan (Game Boy-elrendezés), START alul középen — csak
érintőkijelzőn jelennek meg. Debug URL-kapcsolók: `?autostart` (katt és
hang nélkül indul, konzol-log), `?pads` (asztali gépen is mutatja az
érintőgombokat). ROM-frissítés után `make web`-et újra kell futtatni,
mert a ROM bele van sütve az oldalba.

### Verziózás

Egyetlen forrás: a **git `v*` tag**. A `tools/version.sh` (`git describe`)
adja a szöveget: tagen állva `0.1.0`, a tag után N committal `0.1.0+N`,
piszkos working tree-nél `*` a végén, tag nélkül `0.0.0+<SHA>`. A Makefile
ebből generálja a `build/version.h`-t (`MATECSKA_VERSION`, csak akkor írja
újra, ha változott), a főmenü a logó alatt írja ki (`V0.1.0`, 4. sor), a
`gen_web.py` a játszóoldal hintjébe teszi. **Kiadás = `git tag vX.Y.Z` +
a tag pusholása** — csak ez deployol; sima push csak buildel/tesztel. (A
CI `fetch-depth: 0`-val checkoutol, hogy lássa a tageket.)

### Deploy — GitHub Pages + almos.me

Repo: `github.com/tykhaytschaar/matecska` (public). **Deploy = `v*` tag
pusholása** (`git tag v0.2.0 && git push origin v0.2.0`). A
`.github/workflows/pages.yml` Ubuntu runneren letölti a GBDK-2020 linux64
release-t, `make test` + `make` + `make web`; sima `main`-pushnál ez csak
ellenőrzés, tagnél a `web/` mappát (index.html + matecska.gb) publikálja
is GitHub Pages-re: **https://tykhaytschaar.github.io/matecska/**
Buktató: a repó `github-pages` environmentje csak a felsorolt refekről
enged deployt — a `main` mellé a `v*` **tag**-szabály is fel van véve
(Settings → Environments → github-pages), különben a tag-deploy job
lépés nélkül elbukik.

Az **almos.me** (Netlify) nem tárol semmit a játékból: a `_redirects`
fájlja 200-as proxy-rewrite-tal a Pages-ről szolgálja ki a
`/matecska/play.html` és `/matecska/matecska.gb` címeket (same-origin,
így a böngészős mentés is almos.me alatt marad). Az ottani
`matecska/index.html` allap iframe-ben ágyazza be a `play.html`-t, a
főoldalon kártya + nav-link mutat rá (i18n: `projects.matecska.*`,
`matecska.*`). ROM-változáskor tehát csak ide kell pusholni.

### Fejlesztői környezet és debug-stratégia

Mit és hogyan kell a gépre tenni (kötelező: Xcode CLT, GBDK-2020 arm64,
Python 3 + Pillow, SameBoy): **[SETUP.md](SETUP.md)**.

Debug, ebben a sorrendben:
1. **Logika (mathgen és minden platform-független rész):** `make test`
   natív binárist ad — `lldb` / printf, emulátor nélkül. Ez a leggyorsabb
   iteráció, ide kerüljön minden, ami nem hardverfüggő.
2. **GB-oldali hibák:** a SameBoy beépített debuggere — breakpoint,
   memória-, VRAM- és OAM-nézet (asm-szintű, nem C-forrás-szintű).
3. **Ha tényleg C-szintű breakpoint kell a GB-kódban:** Emulicious +
   VS Code „Emulicious Debugger" kiegészítő (telepítés: SETUP.md).
   Debug buildhez az `lcc`-nek `-debug` flag kell (`.cdb`/`.noi`
   szimbólumfájlok). Csak akkor telepítsd, ha a 2. pont már kevés.

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

**Ékezetes betűk kellenek** (Á É Í Ó Ö Ő Ú Ü Ű, csak nagybetű):
- A glifa a 3×5-ös törzs fölé rajzolt ékezettel **3×7** lesz — még mindig
  elfér egy 8×8-as tile-ban. Éles ékezet: 1 px vessző; Ö/Ü: két pont egy
  sorban (`X.X`); Ő/Ű: két döntött vessző két sorban — ez a legszűkebb,
  de 3 px szélességben is megkülönböztethető az Ö/Ü-től.
- **Kódolás:** a C forrás maradjon ASCII. A `fonts.py` az ékezetes
  betűknek a készlet végén ad kódot, és generál hozzá makrókat
  (pl. `src/font.h`-ba): `#define _O "\x2C"` stb. A C oldalon
  literál-összefűzéssel írjuk: `"SZORZ" _OO "T" _AA "BLA"`.
  A `print_at` így nem tud az ékezetekről, csak tile-indexet nyomtat.

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
y  16–21   szett-állás jobbra zárva, pl. "4/5" (2. tile-sor, x=17)
y  22–32   feladat, 2× méretű 3×5 font, középre zárva  ("36+9*2=")
y  46–62   macska (16×16), start x = 72
y  96–112  három tál, x = 10 / 72 / 134
y 118–123  válaszok, 3×5 font, tálanként középre  ("A 90")
```

A kijelölés a tál + válasz köré rajzolt keret.

### Menü layoutok

Főmenü:
```
y   8–20   logó vagy "MATECSKA" felirat
y  56      UJ JATEK
y  72      FOLYTATAS   (csak érvényes mentésnél)
```

Feladattípus-választó:
```
y   8–20   "MIT GYAKOROLJUNK?" felirat
y  32–     témalista, soronként egy téma (3×5 font, max 17 karakter —
           szám az 1-es, név a 3-as tile-oszloptól)
           a sor elején a hátralévő hibátlan szettek száma (teljesítettnél
           semmi); a kijelölt sor teljes szélességében invertált (nincs
           külön kurzorjel); fel/le lépked, A = indít, B = főmenü
```

A típusválasztó **a `mg_topics[]` táblából rajzolja magát** (lásd
lejjebb), nem kézzel felsorolt feliratokból — új téma felvételekor a
menühöz nem kell nyúlni. A képernyőre ~8 sor fér el; ha több téma lesz,
lapozás kell. A menüfeliratok a 3×5 font karakterkészletét használhatják
(A–Z + ékezetes nagybetűk, számok, `: ? ! . + - * =` — lásd a Font
szakaszt az ékezetek kódolásáról).

## Kódstruktúra

```
src/
  main.c        állapotgép + fő ciklus + SRAM mentés
  game.h        közös típusok, konstansok, layout-sorok
  sprites.h     GENERÁLT (tools/gen_assets.py) — csak a render.c includolja!
  font.h        GENERÁLT (tools/gen_font.py) — makrók + externek, bárhonnan
  font_data.c   GENERÁLT — a font tile-adat definíciói
  mathgen.c/h   feladatgenerátor + témaregiszter (natívan tesztelhető)
  render.c/h    print_at, HUD, tálak, logó — a tile-adatok egyetlen gazdája
```

A `sprites.h` tile-tömböket *definiál*, ezért csak a `render.c`-be szabad
includolni — az animációs sorrendeket a `render.h` exportálja externként.

### Állapotgép
```
TITLE → MAINMENU → TOPIC → QUESTION → WALK → EAT → REACT ─┬→ QUESTION
                     ↑                        (rossznál)  ├→ REVEAL → TOPIC
                     ├──── (szett vége: 5 kérdés után) ───┤
                     └── MSG ←─ (téma kész / szintlépés / ┤
                                 szintvesztés)            │
        MAINMENU ←── (életek elfogytak 1-es szinten) ─────┴→ GAMEOVER
```

Az MSG a szett végi üzenetképernyő (téma teljesítve / szintlépés /
szintvesztés), A-ra lép vissza a TOPIC-ra. Az életek elfogyása 2-es
szinttől MSG-be visz (szintvesztés), csak 1-es szinten GAMEOVER-be.

**Gombszerepek:** **A** = akció (menüpont, tál kiválasztása, üzenet
továbbléptetése), **B** = visszalépés: QUESTION → TOPIC (a szett megszakad,
a téma sorozata nullázódik, élet nem vész el), TOPIC → MAINMENU → TITLE.
START csak a címképernyőn indít; a B sehol nem akció.

## Feladatgenerátor és feladattípusok

`src/mathgen.c`-ben a kétműveletes (műveleti sorrend) generátor már kész és
gcc-vel tesztelhető (`make test`). Ezt kell **témaregiszterré** általánosítani.

### A közös alapelv — ez a játék lényege

A rossz válaszok **tipikus tanulói hibákból** származnak, nem
véletlenszerűek. Minden témánál ez az első kérdés: mit ront el egy gyerek
jellemzően? Ha nincs elég hihető hiba, csak akkor jöhet ±1/±10 elcsúszás,
végső menedékként random szám. A három válasz mindig különböző.

### Témaregiszter — így bővíthető

Minden feladattípus egy generátorfüggvény, ami ugyanazt a `MathQuestion`
struct-ot tölti ki. A típusok egy konstans táblában vannak regisztrálva:

```c
typedef void (*MgGenFn)(MathQuestion *q, uint8_t difficulty);

typedef struct {
    const char *name;   /* menüfelirat, 3×5 font készlete, max 17 kar. */
    MgGenFn     gen;
} MgTopic;

extern const MgTopic mg_topics[];   /* mathgen.c-ben */
extern const uint8_t mg_topic_count;
```

**Új téma felvétele = egy `gen_xxx()` függvény megírása + egy sor a
`mg_topics[]` táblába.** Semmi máshoz (menü, main.c, render) nem kell
hozzányúlni. A közös helperek (`mg_rand`, `rnd`, `put_num`, `try_add`,
válasz-keverés) a mathgen belső eszközkészlete, minden generátor ezekből
építkezik.

A `MathQuestion`-t ehhez bővíteni kell: `text[16]` (elfér: `"2 KG = ? G"`),
`answer_text[3][6]` (elfér: `"2000"`, `"12:45"`). A megjelenítés mindig az
`answer_text`-ből megy — az `answers[]` numerikus érték csak a generátor
belső ügye (időnél pl. percben számol, de `Ó:PP`-t ír ki).

### Tervezett témák

| név (menü) | feladat | tipikus rossz válaszok |
|---|---|---|
| `ÖSSZEADÁS-KIVONÁS` | összeadás/kivonás, nehézséggel nő a számkör (20→1000) | kölcsön/átvitel csak ott, ahol tényleg van; kivonásnál „nagyobból a kisebbet" (10−2 → 12); ±1 csak pótlásnak |
| `SZORZÓTÁBLA` | szorzás 10×10-ig (nehezebben 12-ig); **nem ismétel**: amíg az aktuális számkör összes (a,b) párja ki nem jött, ugyanaz nem jöhet újra (3×6 ≠ 6×3) — 121 bites maszk (16 bájt statikus RAM), kimerüléskor újraindul, szettek közt is él | szomszédos sor/oszlop eredménye (7×8 → 49 v. 63), ±a v. ±b |
| `OSZTÁS` | maradék nélküli osztás, a szorzótábla inverze | hányados ±1, szomszédos tábla-eredményből számolt hányados |
| `SORREND` | kétműveletes kifejezés, a magas precedenciájú művelet mindig a második tag (`a ± b×c`, nehezebben `a ± b:c`) — így minden szinten van valódi sorrend-csapda | balról jobbra számolás, előjelhiba az első műveletnél |
| `ÁTVÁLTÁS` | mértékegység-átváltás: tömeg (g/dkg/kg/t), hossz (mm/cm/dm/m/km), űrmérték (ml/cl/dl/l/hl); pl. `"2 KG = ? DKG"` | rossz tízes hatvány (×10 a ×100 helyett), fordított irányú váltás |
| `IDŐ` | óra+perc: `"3:40 + 35 P = ?"`, időtartam két időpont közt | 60-as váltás elrontva (100-zal számol: 3:75), óraátlépés elfelejtve |

A menünevek a forrásban a font-fejléc ékezet-makróival íródnak
(pl. `"SZORZ" _OO "T" _AA "BLA"`), a kérdés-szövegek (mértékegységek)
maradnak sima ASCII nagybetűk.

Számhatárok: aritmetikai témáknál marad a 0–999; átváltásnál max 4 számjegy
(pl. `2000`), időnél `Ó:PP` formátum. Az `int16_t` mindenhova elég.

Nehézségi szintek a `difficulty` mezőn keresztül (0–3), minden téma a maga
módján értelmezi: nagyobb számkör, több egység-ugrás (kg→g két lépcső),
áthúzódó percek stb.

## Hang

Pipeline: **hUGETracker** (szerkesztés) + **hUGEDriver** (lejátszás a ROM-ban).

- A zene `.uge` fájlként készül a hUGETrackerben (1.0.4 óta van Mac-verzió),
  a forrásfájlok az `assets/music/` mappába kerülnek.
- A `.uge`-t a hUGETracker mellé csomagolt `uge2source` konvertálja C
  forrássá, ez kerül a build-be. A hUGEDriver public domain, GBDK-ból
  dokumentáltan hívható: `hUGE_init(&song)` egyszer, `hUGE_dosound()`
  képkockánként a fő ciklusból. A drivert vendorolva vesszük be
  (pl. `lib/hugedriver/`), ne submodule legyen.
- **SFX** (rövid „nyam", öröm-arpeggio, hibahang): kézzel írt
  regiszter-írások (NRxx), nem tracker-anyag. Effekt közben az érintett
  csatornát `hUGE_mute_channel()`-lel vesszük el a zenétől, utána vissza.
- Zene csak akkor szól, ha már van: a játék hang nélkül is teljes értékű
  legyen (némítás-opció úgyis kell majd).

## Asset-generálás

A sprite-ok Pythonból generálódnak, nem képszerkesztőből. A tools/
szkripteknek Python 3 + **Pillow** kell. Ha módosítani kell egy sprite-ot:

1. szerkeszd a pixel-térképet `tools/sprites.py`-ban (`.` = átlátszó,
   `L` = világos, `M` = közép, `D` = sötét)
2. `make assets` → újragenerálja a `src/sprites.h`-t és az `assets/*.png`-t
3. `python3 tools/gen_design.py` → frissíti a `design/` látványterveket

Alternatíva: a `assets/matecska_sprites_1x.png` közvetlenül betölthető
Asepritebe, vagy futtatható rajta a GBDK `png2asset`.

## Kész van

- font (3×5 + ékezetesek + 5×7 logó) tile-ként, `print_at()`, ékezet-makrók
- témaregiszter (`mg_topics[]`) + mind a 7 generátor, natív teszttel
- teljes állapotgép: cím-animáció, főmenü (új játék/folytatás),
  típusválasztó, 5-ös szettek, pontozás/életek/szintlépés, reveal
- SRAM mentés (MBC5+RAM+BATTERY), folytatás
- tálak háttér-tile-ként; sötét HUD-sáv inverz fonttal + elválasztó vonal
- futásidőben invertált teljes 3×5 készlet (`render_init`, `INV_BASE`
  = 203-tól a VRAM-ban; `print_at_inv`) — ebből van a típusválasztó
  inverz kijelölése
- 2× szöveg általánosítva (`print_big`, 18 slot a 128..202 régióban):
  a kérdés a 0–9, a címképernyő logója a 0–7, a START a 8–12 slotokat
  használja; `draw_logo_big`, `draw_bowl_at` helperek
- kijelölő keret a tál körül; 2× méretű feladatszöveg (futásidőben
  skálázott tile-ok a 0x8800-as VRAM-régióban; >10 karakternél 1×)
- szív/csepp effekt-sprite a reakció alatt (OBJ 2, `SPR_FX`)
- headless teszt PyBoy-jal a repo `.venv`-jéből
  (`.venv/bin/python` — a venv nincs verziókezelve, létrehozás:
  `python3 -m venv .venv && .venv/bin/pip install pyboy pillow`)
- `make e2e` — teljes végigjátszó teszt (`tools/test_e2e.py`): egy szint
  végigvitele savestate-próbás helyes válaszokkal; a WRAM-címeket a
  `build/matecska.noi`-ból olvassa (linker `-Wl-j`)

## Amit még nem csináltam meg

- hang (lásd a Hang szekciót: hUGEDriver beépítése, SFX, zene)
- lapozás a típusválasztóban, ha 8-nál több téma lesz
- IDŐ témába az időtartam-kérdések (két időpont közt), most csak összeadás
- game over képernyő csinosítása (fanyalgó macska, statisztika)

## Megjegyzés a kódhoz

- SDCC-t használ a GBDK, ami **C89-közeli** és nagyon korlátozott:
  nincs `long long`, a `float` kerülendő, a struct-visszaadás drága.
  Használj `uint8_t`/`int16_t`-t mindenhol, ahol lehet.
- Globális változók olcsóbbak, mint a mély hívási lánc (nincs igazi stack frame).
- `banked` függvények csak akkor, ha tényleg elfogy a 32 KB.
