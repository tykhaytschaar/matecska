# MATECSKA

Game Boy (DMG) fejszamolos jatek. Random matekfeladat, harom valasz, es egy
macska, aki a valasztott talbol eszik - ha jol tippeltel, orul, ha nem,
fanyalog.

![cimkepernyo](design/cimkepernyo.png)

**Jatszd a bongeszoben:** https://tykhaytschaar.github.io/matecska/ (a `.gb` is onnan toltheto le). Deploy: `vX.Y.Z` tag pusholasa -> GitHub Actions -> Pages (sima push csak buildel/tesztel).
Verzio: a git `vX.Y.Z` tagbol (`tools/version.sh`), a fomenuben is latszik; kiadas = tag + push.

## Allapot

Jatszahato: cim-animacio, fomenu (uj jatek / folytatas SRAM-mentesbol),
6 feladattipus 5-os szettekben, temankenti teljesites, szintlepes/szintvesztes.

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
