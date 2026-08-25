/* MATECSKA - hatter-rajzolas
 *
 * A sprites.h es a font.h tile-adatot DEFINIAL, ezert csak itt
 * includoljuk - mas fajl a render.h-n keresztul er el mindent.
 */

#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>

#include "game.h"
#include "render.h"
#include "sprites.h"
#include "font.h"

/* a tal-tile-ok a font utan kerulnek a VRAM-ba */
#define BOWL_BASE   FONT_NTILES        /* 3 tal x 4 tile */

/* futasidoben invertalt 3x5 keszlet (a print_at-keszlet parja) a menu-
 * kijeloleshez - a VRAM tetejen, a 2x regio (BIG_BASE, lasd lejjebb)
 * folott */
#define INV_BASE    203
#define INV_NTILES  FONT_TEXT_NTILES

static const uint8_t *const spr_ptrs[SPR_COUNT] = {
    cat_down_a, cat_down_b, cat_dl_a, cat_dl_b,
    cat_eat_up, cat_eat_mid, cat_eat_down,
    cat_happy_a, cat_happy_b, cat_yuck_a, cat_yuck_b,
    bowl_fish, bowl_kibble, bowl_veg, fx_icons,
};

/* --- Game Boy Color: palettak + attributum-terkep ------------------------ */
/* A tile-adat valtozatlan (DMG-n bitre a regi kep); Coloron a VRAM 1. bankja
 * (attributum-terkep) mondja meg tile-onkent, melyik paletta szinezi.
 * Index-szerepek: font = 0 papir / 3 tinta (az invertalt font es a HUD
 * forditva: 0 szoveg / 3 sav); talak es sprite-ok mind a 4 indexet hasznaljak. */
uint8_t g_color;

#define PAPER RGB8(0xFD, 0xF6, 0xE3)
#define INK   RGB8(0x2B, 0x2D, 0x42)
#define BAR   RGB8(0x1F, 0x2A, 0x44)

static const palette_color_t bg_pals[8 * 4] = {
    /* PAL_TEXT      */ PAPER, RGB8(0xB8,0xBF,0xCC), RGB8(0x6B,0x74,0x8A), INK,
    /* PAL_HUD       */ RGB8(0xFF,0xFF,0xFF), RGB8(0xB0,0xB8,0xC8), RGB8(0x5A,0x66,0x80), BAR,
    /* PAL_HUD_HEART */ RGB8(0xFF,0x5A,0x5F), RGB8(0xB0,0xB8,0xC8), RGB8(0x5A,0x66,0x80), BAR,
    /* PAL_SEL       */ RGB8(0xFF,0xFF,0xFF), RGB8(0xFF,0xD6,0xA0), RGB8(0xE0,0x8A,0x2E), RGB8(0xF2,0x8C,0x28),
    /* PAL_ACCENT    */ PAPER, RGB8(0xF6,0xC0,0x4A), RGB8(0xE0,0x7A,0x1F), RGB8(0xC0,0x39,0x2B),
    /* PAL_BOWL0 hal */ PAPER, RGB8(0xA8,0xDA,0xDC), RGB8(0x45,0x7B,0x9D), RGB8(0x1D,0x35,0x57),
    /* +1 szaraz     */ PAPER, RGB8(0xE9,0xC4,0x6A), RGB8(0xB5,0x65,0x1D), RGB8(0x4A,0x2C,0x0B),
    /* +2 zoldseg    */ PAPER, RGB8(0xB7,0xE4,0xA0), RGB8(0x52,0xA4,0x47), RGB8(0x1E,0x56,0x31),
};

#define CAT_PAL  0, RGB8(0xFF,0xB3,0x47), RGB8(0xC6,0x5D,0x1E), RGB8(0x1A,0x1A,0x1A)
static const palette_color_t obj_pals[8 * 4] = {
    /* OBJ_PAL_CAT   */ CAT_PAL,
    /* OBJ_PAL_HEART */ 0, RGB8(0xFF,0xB3,0xC1), RGB8(0xE6,0x39,0x46), RGB8(0x7A,0x10,0x20),
    /* OBJ_PAL_DROP  */ 0, RGB8(0xBD,0xE0,0xFE), RGB8(0x4A,0x90,0xE2), RGB8(0x1F,0x4E,0x9A),
    /* a tobbi: macska, hogy kobor bit se mutasson szemetet */
    CAT_PAL, CAT_PAL, CAT_PAL, CAT_PAL, CAT_PAL,
};

/* Az EGYETLEN hely, ami a VBK_REG-hez nyul: attributum-teglalap irasa,
 * majd azonnal vissza a tile-bankra. DMG-n (g_color == 0) nem csinal semmit. */
static void attr_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t pal)
{
    if (!g_color) return;
    VBK_REG = VBK_ATTRIBUTES;
    fill_bkg_rect(x, y, w, h, pal);
    VBK_REG = VBK_TILES;
}

void render_init(void)
{
    uint8_t i, j, buf[16];
    const uint8_t *src;

    if (g_color) {                      /* kijelzo ki: biztonsagos */
        set_bkg_palette(0, 8, bg_pals);
        set_sprite_palette(0, 8, obj_pals);
    }

    for (i = 0; i < SPR_COUNT; i++)
        set_sprite_data((uint8_t)(i * 4), 4, spr_ptrs[i]);

    set_bkg_data(0, FONT_NTILES, font_tiles);
    /* a harom tal (16x16 = 4 tile) a hatter tile-keszlet vegere */
    set_bkg_data(BOWL_BASE,     4, bowl_fish);
    set_bkg_data(BOWL_BASE + 4, 4, bowl_kibble);
    set_bkg_data(BOWL_BASE + 8, 4, bowl_veg);

    /* invertalt keszlet: 2bpp-ben az inverz = bajtonkenti komplemens */
    for (i = 0; i < INV_NTILES; i++) {
        src = font_tiles + (uint16_t)i * 16;
        for (j = 0; j < 16; j++) buf[j] = (uint8_t)~src[j];
        set_bkg_data((uint8_t)(INV_BASE + i), 1, buf);
    }
}

void cls(void)
{
    fill_bkg_rect(0, 0, 20, 18, 0);
    attr_rect(0, 0, 20, 18, PAL_TEXT);
}

static void print_map(uint8_t x, uint8_t y, const char *s,
                      const uint8_t *map, uint8_t ofs, uint8_t pal)
{
    uint8_t buf[20];
    uint8_t n = 0;
    uint8_t c;

    while ((c = (uint8_t)s[n]) != 0 && n < 20) {
        if (c >= 0x80)      buf[n] = (uint8_t)((c & 0x7F) + ofs);
        else if (c < 32)    buf[n] = (uint8_t)(map[0] + ofs);
        else                buf[n] = (uint8_t)(map[c - 32] + ofs);
        n++;
    }
    if (n) { set_bkg_tiles(x, y, n, 1, buf); attr_rect(x, y, n, 1, pal); }
}

void print_at(uint8_t x, uint8_t y, const char *s)
{
    print_map(x, y, s, font_map, 0, PAL_TEXT);
}

/* mint a print_at, csak az invertalt keszlettel (sotet alapon vilagos) */
void print_at_inv(uint8_t x, uint8_t y, const char *s)
{
    print_map(x, y, s, font_map, INV_BASE, PAL_SEL);
}

static uint8_t str_len(const char *s)
{
    uint8_t n = 0;
    while (s[n]) n++;
    return n;
}

void print_center(uint8_t y, const char *s)
{
    uint8_t len = str_len(s);
    print_at((uint8_t)((len < 20) ? (20 - len) / 2 : 0), y, s);
}

static void u16_str(char *b, uint16_t v)
{
    uint8_t n = 0, i = 0;
    char tmp[5];
    do { tmp[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n) b[i++] = tmp[--n];
    b[i] = '\0';
}

void print_u16(uint8_t x, uint8_t y, uint16_t v)
{
    char b[6];
    u16_str(b, v);
    print_at(x, y, b);
}

void draw_logo(uint8_t y)
{
    set_bkg_tiles((uint8_t)((20 - LOGO_LEN) / 2), y, LOGO_LEN, 1, logo_map);
    attr_rect((uint8_t)((20 - LOGO_LEN) / 2), y, LOGO_LEN, 1, PAL_ACCENT);
}

void draw_hud(void)
{
    uint8_t buf[6];
    uint8_t i;
    char num[6];

    /* sotet sav vilagos szoveggel, alatta elvalaszto vonal */
    fill_bkg_rect(0, 0, 20, 1, font_map_inv[0]);
    attr_rect(0, 0, 20, 1, PAL_HUD);
    fill_bkg_rect(0, 1, 20, 1, TILE_RULE);
    attr_rect(0, 1, 20, 1, PAL_TEXT);

    print_map(0, 0, "PONT", font_map_inv, 0, PAL_HUD);
    u16_str(num, g_score);
    print_map(5, 0, num, font_map_inv, 0, PAL_HUD);

    print_map(10, 0, "SZ", font_map_inv, 0, PAL_HUD);
    u16_str(num, g_level);
    print_map(12, 0, num, font_map_inv, 0, PAL_HUD);

    for (i = 0; i < MAX_LIVES; i++)
        buf[i] = (i < g_lives) ? TILE_HEART_INV : font_map_inv[0];
    set_bkg_tiles(15, 0, MAX_LIVES, 1, buf);
    attr_rect(15, 0, MAX_LIVES, 1, PAL_HUD_HEART);  /* ures hely: csupa-3 -> savszin */
}

/* tal 2x2 tile-ban; az adat oszloponkent jon (bal-fent, bal-lent,
   jobb-fent, jobb-lent). which: 0 = halas, 1 = szaraz, 2 = zoldseges */
void draw_bowl_at(uint8_t tx, uint8_t ty, uint8_t which)
{
    uint8_t base = (uint8_t)(BOWL_BASE + which * 4);
    uint8_t m[4];
    m[0] = base;              m[1] = (uint8_t)(base + 2);
    m[2] = (uint8_t)(base + 1); m[3] = (uint8_t)(base + 3);
    set_bkg_tiles(tx, ty, 2, 2, m);
    attr_rect(tx, ty, 2, 2, (uint8_t)(PAL_BOWL0 + which));
}

void draw_bowls(void)
{
    draw_bowl_at(1,  BOWL_ROW, 0);
    draw_bowl_at(9,  BOWL_ROW, 1);
    draw_bowl_at(16, BOWL_ROW, 2);
}

/* --- 2x meretu szoveg: futasidoben skalazott tile-ok --------------------- */
/* A nagy tile-ok a 128..202 regioba kerulnek (18 db 4-tile-os slot, az    */
/* INV_BASE alatt). A kerdes a 0..9 slotokat hasznalja; a cimkepernyon a   */
/* logo a 0..7-et, a START a 8..12-t - ezek sosem elnek egyszerre a        */
/* kerdessel.                                                              */
#define BIG_BASE 128
#define BIG_SLOTS 18

/* also 4 bit -> vizszintesen duplazott 8 bit */
static const uint8_t exp4[16] = {
    0x00,0x03,0x0C,0x0F,0x30,0x33,0x3C,0x3F,
    0xC0,0xC3,0xCC,0xCF,0xF0,0xF3,0xFC,0xFF
};

/* egy font-tile 2x-esre nagyitva = 4 uj tile (TL,TR,BL,BR) a slot helyere */
static void big_char_tiles(uint8_t tile, uint8_t slot)
{
    uint8_t buf[64];
    const uint8_t *src = font_tiles + (uint16_t)tile * 16;
    uint8_t *dst = buf;
    uint8_t quad, sy, lo, hi, elo, ehi, base;

    for (quad = 0; quad < 4; quad++) {
        base = (quad & 2) ? 4 : 0;              /* forras sor: 0..3 / 4..7 */
        for (sy = 0; sy < 4; sy++) {
            lo = src[(uint8_t)((base + sy) << 1)];
            hi = src[(uint8_t)(((base + sy) << 1) + 1)];
            if (quad & 1) { elo = exp4[lo & 15]; ehi = exp4[hi & 15]; }
            else          { elo = exp4[lo >> 4]; ehi = exp4[hi >> 4]; }
            *dst++ = elo; *dst++ = ehi;         /* fuggoleges duplazas */
            *dst++ = elo; *dst++ = ehi;
        }
    }
    set_bkg_data((uint8_t)(BIG_BASE + (slot << 2)), 4, buf);
}

/* a slot 4 tile-janak terkepbe irasa (2x2) az (x,y) tile-poziciora */
static void big_put(uint8_t slot, uint8_t x, uint8_t y, uint8_t pal)
{
    uint8_t m2[2];
    m2[0] = (uint8_t)(BIG_BASE + (slot << 2));
    m2[1] = (uint8_t)(m2[0] + 1);
    set_bkg_tiles(x, y, 2, 1, m2);
    m2[0] = (uint8_t)(m2[0] + 2);
    m2[1] = (uint8_t)(m2[0] + 1);
    set_bkg_tiles(x, (uint8_t)(y + 1), 2, 1, m2);
    attr_rect(x, y, 2, 2, pal);
}

/* 2x meretu szoveg az (x,y) tile-pozicion; slot0-tol foglalja a
 * BIG_BASE slotokat (karakterenkent egyet) */
void print_big(uint8_t x, uint8_t y, const char *s, uint8_t slot0, uint8_t pal)
{
    uint8_t i, c, t;

    for (i = 0; s[i] && (uint8_t)(slot0 + i) < BIG_SLOTS; i++) {
        c = (uint8_t)s[i];
        t = (c >= 0x80) ? (uint8_t)(c & 0x7F) : font_map[c - 32];
        big_char_tiles(t, (uint8_t)(slot0 + i));
        big_put((uint8_t)(slot0 + i), (uint8_t)(x + 2 * i), y, pal);
    }
}

/* a MATECSKA logo 2x meretben (16 tile szeles), kozepre zarva;
 * a 0..7 slotokat hasznalja */
void draw_logo_big(uint8_t y)
{
    uint8_t i;

    for (i = 0; i < LOGO_LEN; i++) {
        big_char_tiles(logo_map[i], i);
        big_put(i, (uint8_t)(2 + 2 * i), y, PAL_ACCENT);
    }
}

void draw_question(const MathQuestion *q)
{
    uint8_t len = str_len(q->text);

    fill_bkg_rect(0, QUESTION_ROW, 20, 2, 0);
    attr_rect(0, QUESTION_ROW, 20, 2, PAL_TEXT);

    if (len > 10) {                    /* hosszu szoveg: sima 1x font */
        print_center(QUESTION_ROW, q->text);
        return;
    }
    print_big((uint8_t)((20 - 2 * len) / 2), QUESTION_ROW, q->text, 0, PAL_TEXT);
}

/* a harom valasz kezdooszlopa/hossza - a kurzorrajzolasnak is kell */
static uint8_t ans_x[3], ans_len[3];
static const uint8_t slot_center[3] = { 2, 10, 17 };

void draw_answers(const MathQuestion *q)
{
    uint8_t i, len, x;

    fill_bkg_rect(0, ANSWER_ROW, 20, 1, 0);
    attr_rect(0, ANSWER_ROW, 20, 1, PAL_TEXT);
    for (i = 0; i < 3; i++) {
        len = str_len(q->answer_text[i]);
        x = (uint8_t)(slot_center[i] - len / 2);
        if (x > 200) x = 0;                  /* alulcsordulas vedelme */
        if (x + len > 20) x = (uint8_t)(20 - len);
        ans_x[i] = x; ans_len[i] = len;
        print_at(x, ANSWER_ROW, q->answer_text[i]);
    }
}

/* kijeloles: keret a tal korul (4x4 tile-os gyuru) */
static void put_tile(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t v = t;
    set_bkg_tiles(x, y, 1, 1, &v);
}

void draw_cursor(uint8_t sel)
{
    static const uint8_t frame_l[3] = { 0, 8, 15 };
    uint8_t row[4];
    uint8_t i, l, r;

    for (i = 0; i < 3; i++) {
        l = frame_l[i];
        r = (uint8_t)(l + 3);
        if (i == sel) {
            row[0] = TILE_FRAME_TL; row[1] = TILE_FRAME_T;
            row[2] = TILE_FRAME_T;  row[3] = TILE_FRAME_TR;
            set_bkg_tiles(l, CURSOR_ROW, 4, 1, row);
            row[0] = TILE_FRAME_BL; row[1] = TILE_FRAME_B;
            row[2] = TILE_FRAME_B;  row[3] = TILE_FRAME_BR;
            set_bkg_tiles(l, (uint8_t)(BOWL_ROW + 2), 4, 1, row);
            put_tile(l, BOWL_ROW, TILE_FRAME_L);
            put_tile(l, (uint8_t)(BOWL_ROW + 1), TILE_FRAME_L);
            put_tile(r, BOWL_ROW, TILE_FRAME_R);
            put_tile(r, (uint8_t)(BOWL_ROW + 1), TILE_FRAME_R);
        } else {
            fill_bkg_rect(l, CURSOR_ROW, 4, 1, 0);
            fill_bkg_rect(l, (uint8_t)(BOWL_ROW + 2), 4, 1, 0);
            put_tile(l, BOWL_ROW, 0);
            put_tile(l, (uint8_t)(BOWL_ROW + 1), 0);
            put_tile(r, BOWL_ROW, 0);
            put_tile(r, (uint8_t)(BOWL_ROW + 1), 0);
        }
        /* a keret 12 cellaja (a talat, l+1..l+2, nem erinti) */
        {
            uint8_t pal = (i == sel) ? PAL_ACCENT : PAL_TEXT;
            attr_rect(l, CURSOR_ROW, 4, 1, pal);
            attr_rect(l, (uint8_t)(BOWL_ROW + 2), 4, 1, pal);
            attr_rect(l, BOWL_ROW, 1, 2, pal);
            attr_rect(r, BOWL_ROW, 1, 2, pal);
        }
    }
}
