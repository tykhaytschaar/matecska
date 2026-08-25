/* MATECSKA - kozos konstansok es tipusok */
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* --- kepernyo layout (pixel ill. tile-sor, lasd CLAUDE.md) -------------- */
#define SCR_W          160
#define SCR_H          144

#define PROGRESS_ROW     2      /* szett-allas ("4/5") jobbra zarva */
#define QUESTION_ROW     3      /* feladat szovege (tile-sor)      */
#define REVEAL_ROW       8      /* "A JO VALASZ: ..."              */
#define CURSOR_ROW      11      /* kijelolo csillag a tal folott   */
#define BOWL_ROW        12      /* talak teteje (y=96)             */
#define ANSWER_ROW      15      /* valaszok a talak alatt (y=120)  */

#define CAT_START_X     72
#define CAT_START_Y     46
#define BOWL_Y          96

/* GBDK sprite offset: az OBJ koordinatak (8,16)-tol indulnak */
#define OBJ_OX           8
#define OBJ_OY          16

/* --- jatekszabalyok ------------------------------------------------------ */
#define START_LIVES      3
#define MAX_LIVES        5
#define SET_SIZE         5      /* ennyi feladat egy szett          */
#define STREAK_TARGET    3      /* ennyi hibatlan szett zsinorban = tema teljesitve;
                                   minden tema teljesitve = szintlepes */
#define MAX_LEVEL        9
#define POINTS_PER_LEVEL 5      /* jo valasz = szint * 5 pont       */

/* --- jatekallapotok ----------------------------------------------------- */
typedef enum {
    ST_TITLE,      /* belepo animacio: logo + setalo macska      */
    ST_MAINMENU,   /* UJ JATEK / FOLYTATAS                       */
    ST_TOPIC,      /* feladattipus-valaszto (mg_topics[])        */
    ST_QUESTION,   /* valasztas, kurzor mozgatasa                */
    ST_WALK,       /* macska a valasztott talhoz setal           */
    ST_EAT,        /* eves animacio                              */
    ST_REACT,      /* orul vagy fanyalog                         */
    ST_REVEAL,     /* rossz valasznal a helyes megmutatasa       */
    ST_MSG,        /* szett vegi uzenet (tema kesz / szintlepes), A-ra tovabb */
    ST_GAMEOVER
} GameState;

typedef enum { DIR_DOWN, DIR_DL, DIR_DR } Direction;

/* --- sprite indexek (sorrend: src/sprites.h) ---------------------------- */
#define SPR_DOWN_A      0
#define SPR_DOWN_B      1
#define SPR_DL_A        2
#define SPR_DL_B        3
#define SPR_EAT_UP      4
#define SPR_EAT_MID     5
#define SPR_EAT_DOWN    6
#define SPR_HAPPY_A     7
#define SPR_HAPPY_B     8
#define SPR_YUCK_A      9
#define SPR_YUCK_B     10
#define SPR_BOWL_FISH  11
#define SPR_BOWL_KIB   12
#define SPR_BOWL_VEG   13
#define SPR_FX         14   /* bal fele sziv, jobb fele csepp */
#define SPR_COUNT      15

/* 8x16 OBJ tile indexek az effektekhez */
#define FX_TILE_HEART  (SPR_FX * 4)
#define FX_TILE_DROP   (SPR_FX * 4 + 2)

/* --- Game Boy Color palettak (indexek; az adat a render.c-ben) ----------- */
/* A tile-rajz valtozatlan: a szin = attributum-terkep (VRAM 1. bank) +
 * paletta. DMG-n mindez inaktiv (g_color == 0), a kep bitre a regi.      */
#define PAL_TEXT       0   /* papir + tinta: minden sima szoveg           */
#define PAL_HUD        1   /* sotet sav, vilagos szoveg (invertalt font)  */
#define PAL_HUD_HEART  2   /* piros szivek a HUD-savon                    */
#define PAL_SEL        3   /* temavalaszto kijelolt sora (narancs sav)    */
#define PAL_ACCENT     4   /* logo, START, kijelolo keret                 */
#define PAL_BOWL0      5   /* talak: PAL_BOWL0 + which (hal/szaraz/zoldseg) */
#define OBJ_PAL_CAT    0
#define OBJ_PAL_HEART  1
#define OBJ_PAL_DROP   2

/* --- globalis jatekallapot ---------------------------------------------- */
extern uint8_t   g_color;       /* 1 = Game Boy Coloron futunk           */
extern GameState g_state;
extern uint8_t   g_lives;
extern uint16_t  g_score;
extern uint8_t   g_level;       /* 1..MAX_LEVEL */
extern uint8_t   g_cursor;      /* 0..2 - melyik tal van kijelolve */

/* a harom tal kozeppontjanak X koordinataja pixelben */
extern const uint8_t g_bowl_x[3];

#endif
