/* MATECSKA - kozos konstansok es tipusok */
#ifndef GAME_H
#define GAME_H

#include <stdint.h>

/* --- kepernyo layout (pixel, lasd CLAUDE.md) --------------------------- */
#define SCR_W          160
#define SCR_H          144

#define HUD_H           14
#define QUESTION_Y      22
#define CAT_START_X     72
#define CAT_START_Y     46
#define BOWL_Y          96
#define ANSWER_Y       118

/* GBDK sprite offset: az OBJ koordinatak (8,16)-tol indulnak */
#define OBJ_OX           8
#define OBJ_OY          16

/* --- jatekallapotok ----------------------------------------------------- */
typedef enum {
    ST_TITLE,
    ST_QUESTION,   /* valasztas, kurzor mozgatasa                */
    ST_WALK,       /* macska a valasztott talhoz setal           */
    ST_EAT,        /* eves animacio                              */
    ST_REACT,      /* orul vagy fanyalog                         */
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
#define SPR_COUNT      14

#define START_LIVES     3
#define POINTS_CORRECT 20

/* --- globalis jatekallapot ---------------------------------------------- */
extern GameState g_state;
extern uint8_t   g_lives;
extern uint16_t  g_score;
extern uint8_t   g_difficulty;
extern uint8_t   g_cursor;      /* 0..2 - melyik tal van kijelolve */

/* a harom tal kozeppontjanak X koordinataja pixelben */
extern const uint8_t g_bowl_x[3];

#endif
