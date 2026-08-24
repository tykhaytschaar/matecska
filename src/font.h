/* MATECSKA - GENERALT font interfesz (tools/gen_font.py) */
#ifndef MATECSKA_FONT_H
#define MATECSKA_FONT_H
#include <stdint.h>

#define FONT_NTILES 88
#define FONT_TEXT_NTILES 53  /* a print_at-keszlet (invertalando) */
#define LOGO_LEN    8

#define TILE_HEART      70
#define TILE_RULE       71
#define TILE_FRAME_TL   72
#define TILE_FRAME_T    73
#define TILE_FRAME_TR   74
#define TILE_FRAME_L    75
#define TILE_FRAME_R    76
#define TILE_FRAME_BL   77
#define TILE_FRAME_B    78
#define TILE_FRAME_BR   79
#define TILE_HEART_INV  80

/* ekezetes betuk: string-literalba fuzve, pl. "SZORZ" _OO "T" _AA "BLA" */
#define _AA  "\xab"  /* Á */
#define _EE  "\xac"  /* É */
#define _II  "\xad"  /* Í */
#define _OO  "\xae"  /* Ó */
#define _OE  "\xaf"  /* Ö */
#define _OEE "\xb0"  /* Ő */
#define _UU  "\xb1"  /* Ú */
#define _UE  "\xb2"  /* Ü */
#define _UEE "\xb3"  /* Ű */

extern const uint8_t font_tiles[1408];
extern const uint8_t font_map[64];      /* ASCII 32..95 -> tile */
extern const uint8_t font_map_inv[64];  /* ua. sotet HUD-savra */
extern const uint8_t logo_map[LOGO_LEN];

#endif
