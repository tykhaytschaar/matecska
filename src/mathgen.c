/* MATECSKA - feladatgenerator
 *
 * Platform-fuggetlen, sima C89. gcc-vel is fordul (make test),
 * hogy SDCC nelkul is lehessen rajta iteralni.
 */

#include "mathgen.h"

/* --- sajat RNG: az SDCC rand()-ja gyenge es dragan van ------------------ */
static uint16_t rng_state = 0xACE1u;

void mg_seed(uint16_t s)
{
    rng_state = s ? s : 0xACE1u;
}

/* 16 bites xorshift */
static uint16_t mg_rand(void)
{
    rng_state ^= (uint16_t)(rng_state << 7);
    rng_state ^= (uint16_t)(rng_state >> 9);
    rng_state ^= (uint16_t)(rng_state << 8);
    return rng_state;
}

static uint8_t rnd(uint8_t lo, uint8_t hi)
{
    if (hi <= lo) return lo;
    return (uint8_t)(lo + (mg_rand() % (uint16_t)(hi - lo + 1)));
}

/* --- szamformazas ------------------------------------------------------- */
static uint8_t put_num(char *dst, int16_t v)
{
    char tmp[6];
    uint8_t n = 0, i = 0;
    uint16_t u;

    if (v < 0) { dst[i++] = '-'; u = (uint16_t)(-v); }
    else u = (uint16_t)v;

    do { tmp[n++] = (char)('0' + (u % 10)); u /= 10; } while (u);
    while (n) dst[i++] = tmp[--n];
    return i;
}

/* --- kiertekeles -------------------------------------------------------- */
static int16_t apply(int16_t a, char op, int16_t b)
{
    switch (op) {
        case '+': return (int16_t)(a + b);
        case '-': return (int16_t)(a - b);
        case '*': return (int16_t)(a * b);
        case '/': return b ? (int16_t)(a / b) : 0;
    }
    return 0;
}

static uint8_t is_high(char op) { return (op == '*' || op == '/') ? 1 : 0; }

/* a OP1 b OP2 c, helyes muveleti sorrenddel */
static int16_t eval_correct(int16_t a, char o1, int16_t b, char o2, int16_t c)
{
    if (!is_high(o1) && is_high(o2))
        return apply(a, o1, apply(b, o2, c));
    return apply(apply(a, o1, b), o2, c);
}

/* a tipikus hiba: balrol jobbra, sorrend figyelmen kivul hagyva */
static int16_t eval_ltr(int16_t a, char o1, int16_t b, char o2, int16_t c)
{
    return apply(apply(a, o1, b), o2, c);
}

/* --- valaszlista kezelese ----------------------------------------------- */
static uint8_t contains(const int16_t *arr, uint8_t n, int16_t v)
{
    uint8_t i;
    for (i = 0; i < n; i++) if (arr[i] == v) return 1;
    return 0;
}

static uint8_t try_add(int16_t *arr, uint8_t *n, int16_t v)
{
    if (v < 0 || v > 999) return 0;
    if (contains(arr, *n, v)) return 0;
    arr[(*n)++] = v;
    return 1;
}

/* --- fo generator ------------------------------------------------------- */
void mg_generate(MathQuestion *q, uint8_t difficulty)
{
    int16_t a, b, c, correct;
    int16_t pool[8];
    uint8_t np = 0;
    char o1, o2;
    uint8_t i, j, len, hi;
    int16_t tmp;

    if (difficulty > 3) difficulty = 3;

    /* --- operandusok es operatorok a nehezseghez igazitva --------------- */
    for (;;) {
        switch (difficulty) {
            case 0:  /* egy muvelet, kicsi szamok */
                a = rnd(1, 20); b = rnd(1, 20); c = 0;
                o1 = (mg_rand() & 1) ? '+' : '-';
                o2 = 0;
                break;
            case 1:  /* ket osszeadas/kivonas */
                a = rnd(5, 40); b = rnd(1, 20); c = rnd(1, 20);
                o1 = (mg_rand() & 1) ? '+' : '-';
                o2 = (mg_rand() & 1) ? '+' : '-';
                break;
            case 2:  /* muveleti sorrend: + - es * keverve */
                a = rnd(5, 60); b = rnd(2, 12); c = rnd(2, 9);
                o1 = (mg_rand() & 1) ? '+' : '-';
                o2 = '*';
                if (mg_rand() & 1) { o1 = '*'; o2 = (mg_rand() & 1) ? '+' : '-'; }
                break;
            default: /* osztas is */
                c = rnd(2, 9);
                b = (int16_t)(c * rnd(2, 11));   /* osztas maradek nelkul */
                a = rnd(5, 60);
                o1 = (mg_rand() & 1) ? '+' : '-';
                o2 = '/';
                break;
        }

        correct = (o2 == 0) ? apply(a, o1, b)
                            : eval_correct(a, o1, b, o2, c);
        if (correct >= 0 && correct <= 999) break;
    }

    q->correct_value = correct;

    /* --- kifejezes szovege ---------------------------------------------- */
    len = 0;
    len += put_num(q->text + len, a);
    q->text[len++] = o1;
    len += put_num(q->text + len, b);
    if (o2) {
        q->text[len++] = o2;
        len += put_num(q->text + len, c);
    }
    q->text[len++] = '=';
    q->text[len] = '\0';

    /* --- rossz valaszok: tipikus hibakbol -------------------------------- */
    try_add(pool, &np, correct);

    if (o2) {
        /* 1. balrol jobbra szamolt eredmeny (a leggyakoribb hiba) */
        try_add(pool, &np, eval_ltr(a, o1, b, o2, c));
        /* 2. rossz elojel a masodik muveletnel */
        try_add(pool, &np, eval_correct(a, o1, b, (o2 == '+') ? '-' : '+', c));
    }
    /* 3. elcsuszasok */
    try_add(pool, &np, (int16_t)(correct + rnd(1, 3)));
    try_add(pool, &np, (int16_t)(correct - rnd(1, 3)));
    try_add(pool, &np, (int16_t)(correct + 10));
    try_add(pool, &np, (int16_t)(correct - 10));
    /* 4. vegso menedek */
    while (np < 3) try_add(pool, &np, (int16_t)rnd(0, 99));

    /* az elso harom marad: correct + a ket legjellemzobb hiba */
    for (i = 0; i < 3; i++) q->answers[i] = pool[i];

    /* --- keveres (Fisher-Yates) ------------------------------------------ */
    q->correct_index = 0;
    for (i = 2; i > 0; i--) {
        j = (uint8_t)(mg_rand() % (uint16_t)(i + 1));
        tmp = q->answers[i]; q->answers[i] = q->answers[j]; q->answers[j] = tmp;
        if (q->correct_index == i)      q->correct_index = j;
        else if (q->correct_index == j) q->correct_index = i;
    }

    /* --- valaszok szovege ------------------------------------------------ */
    for (i = 0; i < 3; i++) {
        hi = put_num(q->answer_text[i], q->answers[i]);
        q->answer_text[i][hi] = '\0';
    }
}
