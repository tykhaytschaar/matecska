/* MATECSKA - fo ciklus es allapotgep
 *
 * TITLE -> MAINMENU -> TOPIC -> QUESTION -> WALK -> EAT -> REACT
 *   -> (REVEAL rossz valasznal, utana a szett megszakad -> TOPIC)
 *      | QUESTION (jo valasz, megy tovabb a szett)
 *      | TOPIC (hibatlan szett vege)
 *      | MSG (tema teljesitve / szintlepes / szintvesztes, A-ra TOPIC)
 *      | GAMEOVER (csak 1-es szinten, ha elfogytak az eletek)
 */

#include <gb/gb.h>
#include <stdint.h>

#include "game.h"
#include "mathgen.h"
#include "render.h"
#include "font.h"     /* ekezet-makrok (_AA, _EE, ...) */
#include "version.h"  /* GENERALT: MATECSKA_VERSION (build/, a git tagbol) */

/* ----------------------------------------------------------- globalisok */
GameState g_state  = ST_TITLE;
uint8_t   g_lives  = START_LIVES;
uint16_t  g_score  = 0;
uint8_t   g_level  = 1;
uint8_t   g_cursor = 1;

/* a talak bkg-tile-on ulnek (1., 9., 16. oszlop), a macska ezek fole all */
const uint8_t g_bowl_x[3] = { 8, 72, 128 };

static MathQuestion g_q;
static uint8_t   g_frame;        /* animacios szamlalo   */
static uint8_t   g_anim_step;
static uint8_t   g_cat_x, g_cat_y;
static Direction g_dir;
static uint8_t   g_prev_keys;

static uint8_t g_topic;          /* kivalasztott tema (mg_topics index) */
static uint8_t g_menu_sel;       /* menu-kurzor (fomenu / temavalaszto) */
static uint8_t g_has_save;
static uint8_t g_q_in_set;       /* hanyadik kerdes a szettben (0-tol)  */
static uint8_t g_set_flawless;
static uint8_t g_last_ok;

/* szintlepes: temankent STREAK_TARGET hibatlan szett zsinorban = a tema
 * teljesitve; ha MINDEN tema teljesitve, jon a szintlepes (+1 elet) es
 * a teljesitesek torlodnek. Hibas szett csak az adott tema sorozatat
 * nullazza, a mar teljesitett temak teljesitve maradnak. */
static uint8_t g_tstreak[MG_MAX_TOPICS];  /* hibatlan szettek zsinorban, temankent */
static uint8_t g_tdone;                   /* bitmask: teljesitett temak            */

/* szett vegi uzenet (ST_MSG) */
#define MSG_TOPIC_DONE   1
#define MSG_LEVELUP      2
#define MSG_LEVELUP_LIFE 3    /* szintlepes +1 elettel */
#define MSG_LEVELDOWN    4    /* elfogytak az eletek: egy szint vissza */
static uint8_t g_msg;

/* cimkepernyos seta */
static uint8_t t_right, t_pause;

/* cimkepernyo: a macska a ket tal kozott setal (a talak tile x=4 ill.
 * x=14-nel ulnek, px 32 ill. 112), a talaknal 1-1 mp-re megall "enni" */
#define TITLE_CAT_Y   58
#define TITLE_X_MIN   34
#define TITLE_X_MAX  110
#define TITLE_PAUSE   60         /* 1 mp a talaknal */

#define CUR_DIFF() ((uint8_t)((g_level > 4) ? 3 : (g_level - 1)))

/* --------------------------------------------------------- SRAM mentes
 * layout (v2):
 *   0..2   'M' 'C' verzio
 *   3      szint          4      eletek
 *   5,6    pontszam lo,hi
 *   7      teljesitett temak (bitmask)
 *   8..15  temankenti hibatlan-szett sorozat (MG_MAX_TOPICS)
 *   16..31 szorzotabla-maszk (mg_mul_seen, MG_MUL_BYTES)
 *   32     checksum a 3..31 tartomanyra
 */
#define SRAM ((volatile uint8_t *)0xA000)
#define SAVE_VER      2
#define SAVE_STREAK   8
#define SAVE_MUL      (SAVE_STREAK + MG_MAX_TOPICS)
#define SAVE_SUM_POS  (SAVE_MUL + MG_MUL_BYTES)

static uint8_t save_calc(void)   /* SRAM mar engedelyezve */
{
    uint8_t s = 0x5A, i;
    for (i = 3; i < SAVE_SUM_POS; i++)
        s ^= (uint8_t)(SRAM[i] + i);
    return s;
}

static void save_write(void)
{
    uint8_t i;
    ENABLE_RAM;
    SRAM[0] = 'M'; SRAM[1] = 'C'; SRAM[2] = SAVE_VER;
    SRAM[3] = g_level;
    SRAM[4] = g_lives;
    SRAM[5] = (uint8_t)g_score;
    SRAM[6] = (uint8_t)(g_score >> 8);
    SRAM[7] = g_tdone;
    for (i = 0; i < MG_MAX_TOPICS; i++) SRAM[SAVE_STREAK + i] = g_tstreak[i];
    for (i = 0; i < MG_MUL_BYTES; i++)  SRAM[SAVE_MUL + i]    = mg_mul_seen[i];
    SRAM[SAVE_SUM_POS] = save_calc();
    DISABLE_RAM;
    g_has_save = 1;
}

static uint8_t save_valid(void)
{
    uint8_t ok;
    ENABLE_RAM;
    ok = SRAM[0] == 'M' && SRAM[1] == 'C' && SRAM[2] == SAVE_VER
      && SRAM[SAVE_SUM_POS] == save_calc()
      && SRAM[3] >= 1 && SRAM[3] <= MAX_LEVEL
      && SRAM[4] >= 1 && SRAM[4] <= MAX_LIVES;
    DISABLE_RAM;
    return ok;
}

static void save_load(void)
{
    uint8_t i;
    ENABLE_RAM;
    g_level = SRAM[3];
    g_lives = SRAM[4];
    g_score = (uint16_t)SRAM[5] | ((uint16_t)SRAM[6] << 8);
    g_tdone = SRAM[7];
    for (i = 0; i < MG_MAX_TOPICS; i++) g_tstreak[i]  = SRAM[SAVE_STREAK + i];
    for (i = 0; i < MG_MUL_BYTES; i++)  mg_mul_seen[i] = SRAM[SAVE_MUL + i];
    DISABLE_RAM;
}

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

static void fx_hide(void)
{
    move_sprite(2, 0, 0);
}

static void fx_draw(uint8_t tile, uint8_t x, uint8_t y)
{
    set_sprite_tile(2, tile);
    set_sprite_prop(2, 0);
    move_sprite(2, (uint8_t)(x + OBJ_OX), (uint8_t)(y + OBJ_OY));
}

static void cat_hide(void)
{
    move_sprite(0, 0, 0);
    move_sprite(1, 0, 0);
    fx_hide();
}

/* csak bekapcsolt kijelzonel hivhato (utana szabadon irhato a VRAM) */
static void screen_off(void)
{
    wait_vbl_done();
    DISPLAY_OFF;
}

/* ---------------------------------------------------- kepernyo-epitok */
static void screen_title(void)
{
    screen_off();
    cls();
    cat_hide();
    /* a design/cimkepernyo.png kompozicioja */
    draw_logo_big(1);
    print_center(4, "MATEK MACSK" _AA "KKAL");
    print_at(1, 6,  "+");  print_at(18, 6,  "-");
    print_at(1, 11, "*");  print_at(18, 11, "=");
    draw_bowl_at(4, 8, 0);            /* halas tal balra   */
    draw_bowl_at(14, 8, 2);           /* zoldseges jobbra  */
    print_big(5, 13, "START", 8);     /* a logo (0..7) utani slotok */
    print_center(16, "C 2026");
    DISPLAY_ON;

    g_cat_x = CAT_START_X;
    t_right = 1;
    t_pause = 0;
    g_state = ST_TITLE;
}

static void mainmenu_cursor(void)
{
    print_at(4, 8,  (g_menu_sel == 0) ? "*" : " ");
    if (g_has_save)
        print_at(4, 10, (g_menu_sel == 1) ? "*" : " ");
}

static void screen_mainmenu(void)
{
    screen_off();
    cls();
    cat_hide();
    draw_logo(2);
    print_center(4, "V" MATECSKA_VERSION);
    g_has_save = save_valid();
    print_at(6, 8, _UU "J J" _AA "T" _EE "K");
    if (g_has_save) print_at(6, 10, "FOLYTAT" _AA "S");
    g_menu_sel = 0;
    mainmenu_cursor();
    DISPLAY_ON;
    g_state = ST_MAINMENU;
}

/* egy temasor: | szam (hatralevo hibatlan szettek) | nev | - a kijelolt
 * sor teljes szelessegeben invertalva, teljesitett temanal nincs szam */
static void topic_row(uint8_t i, uint8_t sel)
{
    char buf[21];
    const char *s = mg_topics[i].name;
    uint8_t n = 3, rem;

    rem = (g_tdone & (uint8_t)(1 << i)) ? 0
        : (uint8_t)(STREAK_TARGET - g_tstreak[i]);
    buf[0] = ' ';
    buf[1] = rem ? (char)('0' + rem) : ' ';
    buf[2] = ' ';
    while (*s && n < 20) buf[n++] = *s++;
    while (n < 20) buf[n++] = ' ';
    buf[20] = '\0';
    if (sel) print_at_inv(0, (uint8_t)(4 + 2 * i), buf);
    else     print_at(0, (uint8_t)(4 + 2 * i), buf);
}

static void topic_select(uint8_t sel)
{
    uint8_t old = g_menu_sel;
    g_menu_sel = sel;
    if (old != sel) topic_row(old, 0);
    topic_row(sel, 1);
}

static void screen_topic(void)
{
    uint8_t i;

    screen_off();
    cls();
    cat_hide();
    draw_hud();
    print_center(2, "MIT GYAKOROLJUNK?");
    g_menu_sel = g_topic;
    for (i = 0; i < mg_topic_count; i++)
        topic_row(i, (uint8_t)(i == g_menu_sel));
    DISPLAY_ON;
    g_state = ST_TOPIC;
}

/* szett vegi uzenet: tema teljesitve / szintlepes - A-ra tovabb */
static void screen_msg(void)
{
    screen_off();
    cls();
    cat_hide();
    draw_hud();
    if (g_msg == MSG_TOPIC_DONE) {
        print_center(6, mg_topics[g_topic].name);
        print_center(8, "TELJES" _II "TVE!");
    } else if (g_msg == MSG_LEVELDOWN) {
        print_center(6, "ELFOGYTAK AZ " _EE "LETEK!");
        print_center(8, "EGY SZINT VISSZA");
    } else {
        print_center(6, "MINDEN T" _EE "MA K" _EE "SZ!");
        print_center(8, "SZINTL" _EE "P" _EE "S!");
        if (g_msg == MSG_LEVELUP_LIFE) print_center(10, "+1 " _EE "LET");
    }
    print_center(14, "NYOMJ A-T!");
    DISPLAY_ON;
    g_state = ST_MSG;
}

static void screen_question(void)
{
    char prog[4];

    screen_off();
    cls();
    draw_hud();
    /* szett-allas jobbra fent: "1/5" .. "5/5" */
    prog[0] = (char)('1' + g_q_in_set);
    prog[1] = '/';
    prog[2] = (char)('0' + SET_SIZE);
    prog[3] = '\0';
    print_at(17, PROGRESS_ROW, prog);
    draw_question(&g_q);
    draw_bowls();
    draw_answers(&g_q);
    g_cursor = 1;
    draw_cursor(g_cursor);
    g_cat_x = CAT_START_X;
    g_cat_y = CAT_START_Y;
    cat_draw(SPR_DOWN_A, g_cat_x, g_cat_y, 0);
    DISPLAY_ON;
    g_state = ST_QUESTION;
}

static void next_question(void)
{
    mg_generate(g_topic, &g_q, CUR_DIFF());
    screen_question();
}

static void screen_gameover(void)
{
    screen_off();
    cls();
    cat_hide();
    print_center(6, "GAME OVER");
    print_at(5, 8, "PONT:");
    print_u16(11, 8, g_score);
    print_center(12, "NYOMJ A-T!");
    DISPLAY_ON;

    /* csak 1-es szinten johet: a pont es a teljesitesek nullazodnak,
     * az eletek visszaallnak (a szint ugyis 1) */
    {
        uint8_t i;
        for (i = 0; i < MG_MAX_TOPICS; i++) g_tstreak[i] = 0;
    }
    g_tdone  = 0;
    g_score  = 0;
    g_lives  = START_LIVES;
    save_write();
    g_state = ST_GAMEOVER;
}

/* ------------------------------------------------- szett-adminisztracio */
static void start_set(void)
{
    g_q_in_set = 0;
    g_set_flawless = 1;
    next_question();
}

static void end_question(void)
{
    if (g_lives == 0) {
        if (g_level > 1) {
            /* szintvesztes: egy szint vissza, teljesitesek nullazva,
             * eletek ujra 3 - a pont megmarad, a jatek folytatodik */
            uint8_t i;
            g_level--;
            g_lives = START_LIVES;
            g_tdone = 0;
            for (i = 0; i < MG_MAX_TOPICS; i++) g_tstreak[i] = 0;
            g_msg = MSG_LEVELDOWN;
            save_write();
            screen_msg();
        } else {
            screen_gameover();
        }
        return;
    }

    if (!g_set_flawless) {
        /* rontott feladat: a szett azonnal veget er, az adott tema
         * sorozata nullazodik, vissza a valasztora */
        g_tstreak[g_topic] = 0;
        save_write();
        screen_topic();
        return;
    }

    g_q_in_set++;
    if (g_q_in_set >= SET_SIZE) {        /* hibatlan szett vegigjatszva */
        uint8_t i, all;

        g_msg = 0;
        if (!(g_tdone & (uint8_t)(1 << g_topic))) {
            g_tstreak[g_topic]++;
            if (g_tstreak[g_topic] >= STREAK_TARGET) {
                g_tdone |= (uint8_t)(1 << g_topic);
                g_msg = MSG_TOPIC_DONE;

                all = 1;
                for (i = 0; i < mg_topic_count; i++)
                    if (!(g_tdone & (uint8_t)(1 << i))) { all = 0; break; }
                if (all) {               /* minden tema kesz: szintlepes */
                    if (g_level < MAX_LEVEL) g_level++;
                    if (g_lives < MAX_LIVES) { g_lives++; g_msg = MSG_LEVELUP_LIFE; }
                    else                     g_msg = MSG_LEVELUP;
                    g_tdone = 0;
                    for (i = 0; i < MG_MAX_TOPICS; i++) g_tstreak[i] = 0;
                }
            }
        }
        save_write();
        if (g_msg) screen_msg();
        else       screen_topic();
        return;
    }
    save_write();
    next_question();
}

/* --------------------------------------------------------------- TITLE */
static void title_update(uint8_t pressed)
{
    uint8_t spr;

    if (pressed & (J_START | J_A)) {
        /* seed: a gombnyomas idozitesebol - eleg jo entropia GB-n */
        mg_seed((uint16_t)(((uint16_t)DIV_REG << 8) | g_frame));
        screen_mainmenu();
        return;
    }

    if (t_pause) {
        t_pause--;
        /* a talnal eszik, sziv a feje folott (mint a designon) */
        cat_draw(anim_eat[(t_pause >> 3) & 3], g_cat_x, TITLE_CAT_Y, 0);
        fx_draw(FX_TILE_HEART, (uint8_t)(g_cat_x + 4), TITLE_CAT_Y - 14);
        if (!t_pause) { t_right = (uint8_t)!t_right; fx_hide(); }
        return;
    }

    if (g_frame & 1) {                    /* fel sebesseg, komotos seta */
        if (t_right) g_cat_x++; else g_cat_x--;
        if (g_cat_x >= TITLE_X_MAX || g_cat_x <= TITLE_X_MIN)
            t_pause = TITLE_PAUSE;
    }
    spr = (g_frame & 8) ? SPR_DL_B : SPR_DL_A;
    cat_draw(spr, g_cat_x, TITLE_CAT_Y, t_right);
}

/* ------------------------------------------------------------ MAINMENU */
static void mainmenu_update(uint8_t pressed)
{
    if ((pressed & (J_UP | J_DOWN)) && g_has_save) {
        g_menu_sel = (uint8_t)!g_menu_sel;
        mainmenu_cursor();
    }
    if (pressed & J_B) { screen_title(); return; }
    if (pressed & J_A) {
        if (g_menu_sel == 1) {
            save_load();
        } else {
            uint8_t i;
            g_level = 1; g_lives = START_LIVES;
            g_score = 0; g_tdone = 0;
            for (i = 0; i < MG_MAX_TOPICS; i++) g_tstreak[i]  = 0;
            for (i = 0; i < MG_MUL_BYTES; i++)  mg_mul_seen[i] = 0;
        }
        g_topic = 0;
        screen_topic();
    }
}

/* --------------------------------------------------------------- TOPIC */
static void topic_update(uint8_t pressed)
{
    if ((pressed & J_UP) && g_menu_sel > 0)
        topic_select((uint8_t)(g_menu_sel - 1));
    if ((pressed & J_DOWN) && g_menu_sel < (uint8_t)(mg_topic_count - 1))
        topic_select((uint8_t)(g_menu_sel + 1));
    if (pressed & J_B) { screen_mainmenu(); return; }
    if (pressed & J_A) {
        g_topic = g_menu_sel;
        start_set();
    }
}

/* ------------------------------------------------------------ QUESTION */
static void question_update(uint8_t pressed)
{
    if ((pressed & J_LEFT)  && g_cursor > 0) { g_cursor--; draw_cursor(g_cursor); }
    if ((pressed & J_RIGHT) && g_cursor < 2) { g_cursor++; draw_cursor(g_cursor); }

    if (pressed & J_B) {
        /* visszalepes: a szett megszakad (nem hibatlan -> a tema sorozata
         * nullazodik), elet nem vesz el */
        g_tstreak[g_topic] = 0;
        save_write();
        screen_topic();
        return;
    }
    if (pressed & J_A) {
        g_dir   = (g_cursor == 0) ? DIR_DL : (g_cursor == 2 ? DIR_DR : DIR_DOWN);
        g_state = ST_WALK;
    }
}

/* ---------------------------------------------------------------- WALK */
static void walk_update(void)
{
    uint8_t target_x = g_bowl_x[g_cursor];
    uint8_t target_y = (uint8_t)(BOWL_Y - 16);
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
            g_last_ok = (uint8_t)(g_cursor == g_q.correct_index);
        }
    }
}

/* --------------------------------------------------------------- REACT */
static void react_update(void)
{
    uint8_t step = (uint8_t)(g_anim_step & 3);
    uint8_t yoff = 0, xoff = 0, spr;

    if (g_last_ok) {
        spr = anim_happy[step];
        if (step == 1) yoff = 2;            /* ugras: -2 px */
        if (step == 2) yoff = 4;            /* ugras: -4 px */
        cat_draw(spr, g_cat_x, (uint8_t)(g_cat_y - yoff), 0);
        /* lebego sziv a macska folott */
        fx_draw(FX_TILE_HEART, (uint8_t)(g_cat_x + 4),
                (uint8_t)(g_cat_y - 14 - (yoff << 1)));
    } else {
        spr = anim_yuck[step];
        xoff = (step == 1) ? 1 : 0;         /* fejrazas */
        cat_draw(spr, (step == 3) ? (uint8_t)(g_cat_x + 1)
                                  : (uint8_t)(g_cat_x - xoff), g_cat_y, 0);
        /* lecsorgo csepp a fej mellett */
        fx_draw(FX_TILE_DROP, (uint8_t)(g_cat_x + 12),
                (uint8_t)(g_cat_y - 8 + g_anim_step));
    }

    if (++g_frame >= 8) {
        g_frame = 0;
        if (++g_anim_step >= 8) {
            fx_hide();
            if (g_last_ok) {
                g_score += (uint16_t)g_level * POINTS_PER_LEVEL;
                if (g_score > 9999) g_score = 9999;   /* HUD: 4 szamjegy */
                draw_hud();
                end_question();
            } else {
                /* elet csak akkor vesz el, ha ebben a temaban meg mind a
                 * 3 szett teljesitendo (nincs megkezdett sorozat es a
                 * tema sincs teljesitve) */
                if (!(g_tdone & (uint8_t)(1 << g_topic))
                    && g_tstreak[g_topic] == 0)
                    g_lives--;
                g_set_flawless = 0;
                draw_hud();
                /* a helyes valasz megmutatasa, A-ra tovabb */
                {
                    static const char pre[] = "A J" _OO " V" _AA "LASZ: ";
                    char buf[20];
                    const char *a = g_q.answer_text[g_q.correct_index];
                    uint8_t n = 0;
                    while (pre[n]) { buf[n] = pre[n]; n++; }
                    while (*a && n < 19) buf[n++] = *a++;
                    buf[n] = '\0';
                    print_center(REVEAL_ROW, buf);
                }
                g_state = ST_REVEAL;
            }
        }
    }
}

/* -------------------------------------------------------------- REVEAL */
static void reveal_update(uint8_t pressed)
{
    if (pressed & J_A) end_question();
}

/* ------------------------------------------------------------------ fo */
void main(void)
{
    uint8_t keys, pressed, i;

    SPRITES_8x16;
    screen_off();
    render_init();
    for (i = 0; i < 40; i++) move_sprite(i, 0, 0);   /* OAM-szemet ki */
    DISPLAY_ON;
    SHOW_SPRITES;
    SHOW_BKG;

    g_has_save = save_valid();
    screen_title();

    for (;;) {
        keys    = joypad();
        pressed = (uint8_t)(keys & ~g_prev_keys);
        g_prev_keys = keys;
        g_frame++;

        switch (g_state) {
            case ST_TITLE:    title_update(pressed);    break;
            case ST_MAINMENU: mainmenu_update(pressed); break;
            case ST_TOPIC:    topic_update(pressed);    break;
            case ST_QUESTION: question_update(pressed); break;
            case ST_WALK:     walk_update();            break;
            case ST_EAT:      eat_update();             break;
            case ST_REACT:    react_update();           break;
            case ST_REVEAL:   reveal_update(pressed);   break;
            case ST_MSG:
                if (pressed & J_A) screen_topic();
                break;
            case ST_GAMEOVER:
                if (pressed & J_A) screen_mainmenu();
                break;
        }

        wait_vbl_done();
    }
}
