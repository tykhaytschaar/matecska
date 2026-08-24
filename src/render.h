/* MATECSKA - hatter-rajzolas: font, HUD, talak, menuk */
#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include "mathgen.h"

/* sprite/hatter tile-adat betoltese VRAM-ba (csak kikapcsolt kijelzonel
   vagy vblank alatt hivd) */
void render_init(void);

void cls(void);
void print_at(uint8_t x, uint8_t y, const char *s);
void print_at_inv(uint8_t x, uint8_t y, const char *s);  /* invertalt (kijeloles) */
void print_center(uint8_t y, const char *s);
void print_u16(uint8_t x, uint8_t y, uint16_t v);

void draw_logo(uint8_t y);           /* 1x logo (fomenu) */
void draw_logo_big(uint8_t y);       /* 2x logo (cimkepernyo), 0..7 slot */
void print_big(uint8_t x, uint8_t y, const char *s, uint8_t slot0);
void draw_hud(void);                 /* g_score / g_lives / g_level alapjan */
void draw_bowls(void);
void draw_bowl_at(uint8_t tx, uint8_t ty, uint8_t which);  /* 0=hal 1=szaraz 2=zoldseg */
void draw_question(const MathQuestion *q);
void draw_answers(const MathQuestion *q);
void draw_cursor(uint8_t sel);       /* kijeloles a 3 valasz kozul */

/* animacios sorrendek - a sprites.h-t a render.c definialja, ezek onnan
   latszanak ki (a sprites.h csak egy forrasfajlba includolhato) */
extern const uint8_t anim_eat[4];
extern const uint8_t anim_happy[4];
extern const uint8_t anim_yuck[4];

#endif
