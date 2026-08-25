# MATECSKA

Game Boy (DMG) fejszamolos jatek. Random matekfeladat, harom valasz, es egy
macska, aki a valasztott talbol eszik - ha jol tippeltel, orul, ha nem,
fanyalog.

![cimkepernyo](design/cimkepernyo.png)

## Allapot

Jatszahato: cim-animacio, fomenu (uj jatek / folytatas SRAM-mentesbol),
7 feladattipus 5-os szettekben, pontozas/eletek/szintlepes.

| resz | allapot |
|---|---|
| sprite-ok + animaciok | kesz (`src/sprites.h`) |
| latvanyterv | kesz (`design/`) |
| feladatgenerator, 7 tema + regiszter | kesz, tesztelt (`make test`) |
| menuk (fomenu, temavalaszto) | kesz |
| allapotgep, szett/pont/szint logika | kesz (`src/main.c`) |
| font (ekezetes) + szovegkiiras | kesz (`src/font.h`, `render.c`) |
| SRAM mentes (folytatas) | kesz |
| hattergrafika, HUD | alap kesz; keret/sotet sav hianyzik |
| hang | hianyzik |

## Gyorsindulas

Kornyezet telepitese lepesrol lepesre: **[SETUP.md](SETUP.md)**

```sh
export GBDK_HOME=$HOME/gbdk
make test     # feladatgenerator gcc-vel, GBDK nelkul is megy
make          # build/matecska.gb
make run      # emulatorban
make web      # bongeszos jatszooldal (web/index.html, onallo fajl)
```

## Mappak

```
src/      C forras (sprites.h GENERALT - ne szerkeszd kezzel)
assets/   1x sprite sheet PNG (Aseprite / png2asset bemenet)
design/   latvanytervek: cimkepernyo, jatekmenet, animaciok, karakterlap
tools/    Python sprite- es latvanyterv-generatorok
```

Reszletes brief: **[CLAUDE.md](CLAUDE.md)**
