/* MATECSKA - fo ciklus es allapotgep
 *
 * VAZLAT. Ez a fajl a strukturat rogziti, nem teljes implementacio.
 * A TODO-val jelolt reszek varnak megirasra - lasd CLAUDE.md.
 */

#include <gb/gb.h>
#include <stdint.h>

#include "game.h"
#include "mathgen.h"
#include "sprites.h"

/* ----------------------------------------------------------- globalisok */
GameState g_state      = ST_TITLE;
uint8_t   g_lives      = START_LIVES;
uint16_t  g_score      = 0;
uint8_t   g_difficulty = 0;
uint8_t   g_cursor     = 1;

const uint8_t g_bowl_x[3] = { 10, 72, 134 };

static MathQuestion g_q;
static uint8_t  g_frame;        /* animacios szamlalo   */
static uint8_t  g_anim_step;
static uint8_t  g_cat_x, g_cat_y;
static Direction g_dir;
static uint8_t  g_prev_keys;

/* --------------------------------------------------------- sprite rajz */

/* Egy 16x16 metasprite = 2 db 8x16 OBJ. Az OBJ 0 es 1 a macskae. */
static void cat_draw(uint8_t spr, uint8_t x, uint8_t y, uint8_t flip)
{
    uint8_t tl = (uint8_t)(spr * 4);      /* 4 tile / sprite, 8x16 modban */
    uint8_t lx = flip ? (uint8_t)(x + 8) : x;
    uint8_t rx = flip ? x : (uint8_t)(x + 8);

    set_sprite_tile(0, tl);
    move_sprite(0, (uint8_t)(lx + OBJ_OX), (uint8_t)(y + OBJ_OY));
    set_sprite_prop(0, flip ? S_FLIPX : 0);

    set_sprite_tile(1, (uint8_t)(tl + 2));
    move_sprite(1, (uint8_t)(rx + OBJ_OX), (uint8_t)(y + OBJ_OY));
    set_sprite_prop(1, flip ? S_FLIPX : 0);
}

/* --------------------------------------------------------------- TITLE */
static void title_enter(void)
{
    /* TODO: cimkepernyo hattere, logo tile-ok, "START" villogas */
}

static void title_update(uint8_t keys, uint8_t pressed)
{
    (void)keys;
    if (pressed & (J_START | J_A)) {
        g_lives      = START_LIVES;
        g_score      = 0;
        g_difficulty = 0;
        g_state      = ST_QUESTION;
        /* seed: a gombnyomas idozitesebol - eleg jo entropia GB-n */
        mg_seed((uint16_t)(g_frame * 257u + 13u));
        mg_generate(&g_q, g_difficulty);
        /* TODO: kerdes es valaszok kiirasa */
    }
}

/* ------------------------------------------------------------ QUESTION */
static void question_update(uint8_t keys, uint8_t pressed)
{
    (void)keys;
    if ((pressed & J_LEFT)  && g_cursor > 0) g_cursor--;
    if ((pressed & J_RIGHT) && g_cursor < 2) g_cursor++;
    /* TODO: kijelolo keret athelyezese */

    if (pressed & J_A) {
        g_dir   = (g_cursor == 0) ? DIR_DL : (g_cursor == 2 ? DIR_DR : DIR_DOWN);
        g_state = ST_WALK;
    }
}

/* ---------------------------------------------------------------- WALK */
static void walk_update(void)
{
    uint8_t target_x = g_bowl_x[g_cursor];
    uint8_t target_y = BOWL_Y - 16;
    uint8_t spr, flip = 0;

    if (g_cat_x < target_x) g_cat_x++;
    if (g_cat_x > target_x) g_cat_x--;
    if (g_cat_y < target_y) g_cat_y++;

    if (g_dir == DIR_DOWN) spr = (g_frame & 8) ? SPR_DOWN_B : SPR_DOWN_A;
    else { spr = (g_frame & 8) ? SPR_DL_B : SPR_DL_A; flip = (g_dir == DIR_DR); }

    cat_draw(spr, g_cat_x, g_cat_y, flip);

    if (g_cat_x == target_x && g_cat_y == target_y) {
        g_state = ST_EAT;
        g_anim_step = 0;
        g_frame = 0;
    }
}

/* ----------------------------------------------------------------- EAT */
static void eat_update(void)
{
    /* anim_eat[] = {EAT_UP, EAT_MID, EAT_DOWN, EAT_MID}, 6 kepkocka/fazis */
    cat_draw(anim_eat[g_anim_step & 3], g_cat_x, g_cat_y, 0);

    if (++g_frame >= 6) {
        g_frame = 0;
        if (++g_anim_step >= 12) {          /* 3 teljes kor */
            g_state = ST_REACT;
            g_anim_step = 0;
        }
    }
}

/* --------------------------------------------------------------- REACT */
static void react_update(void)
{
    uint8_t ok = (g_cursor == g_q.correct_index);
    uint8_t step = (uint8_t)(g_anim_step & 3);
    uint8_t yoff = 0, xoff = 0, spr;

    if (ok) {
        spr = anim_happy[step];
        if (step == 1) yoff = 2;            /* ugras: -2 px */
        if (step == 2) yoff = 4;            /* ugras: -4 px */
        cat_draw(spr, g_cat_x, (uint8_t)(g_cat_y - yoff), 0);
        /* TODO: sziv ikonok az OBJ 2-3 helyen */
    } else {
        spr = anim_yuck[step];
        xoff = (step == 1) ? 1 : 0;         /* fejrazas */
        cat_draw(spr, (step == 3) ? (uint8_t)(g_cat_x + 1)
                                  : (uint8_t)(g_cat_x - xoff), g_cat_y, 0);
        /* TODO: csepp ikon */
    }

    if (++g_frame >= 8) {
        g_frame = 0;
        if (++g_anim_step >= 8) {
            if (ok) {
                g_score += POINTS_CORRECT;
                if (g_score > 100 && g_difficulty < 3) g_difficulty++;
            } else if (--g_lives == 0) {
                g_state = ST_GAMEOVER;
                return;
            }
            /* uj kerdes */
            g_cat_x = CAT_START_X;
            g_cat_y = CAT_START_Y;
            g_cursor = 1;
            mg_generate(&g_q, g_difficulty);
            /* TODO: kerdes es valaszok kiirasa, HUD frissites */
            g_state = ST_QUESTION;
        }
    }
}

/* ------------------------------------------------------------------ fo */
void main(void)
{
    uint8_t keys, pressed;

    SPRITES_8x16;
    set_sprite_data(0, SPR_COUNT * 4, (const uint8_t *)cat_down_a);
    /* megj.: a sprites.h kulon tombokben tarolja a sprite-okat.
       TODO: vagy egy folytonos tombbe kell osszefuzni oket, vagy
       sprite-onkent kulon set_sprite_data() hivas kell. */

    DISPLAY_ON;
    SHOW_SPRITES;
    SHOW_BKG;

    g_cat_x = CAT_START_X;
    g_cat_y = CAT_START_Y;
    title_enter();

    for (;;) {
        keys    = joypad();
        pressed = (uint8_t)(keys & ~g_prev_keys);
        g_prev_keys = keys;
        g_frame++;

        switch (g_state) {
            case ST_TITLE:    title_update(keys, pressed); break;
            case ST_QUESTION: question_update(keys, pressed); break;
            case ST_WALK:     walk_update(); break;
            case ST_EAT:      eat_update(); break;
            case ST_REACT:    react_update(); break;
            case ST_GAMEOVER:
                /* TODO: game over kepernyo, majd vissza a cimre */
                if (pressed & J_START) { g_state = ST_TITLE; title_enter(); }
                break;
        }

        wait_vbl_done();
    }
}
