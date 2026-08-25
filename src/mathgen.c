/* MATECSKA - feladatgenerator
 *
 * Platform-fuggetlen, sima C89. gcc-vel is fordul (make test),
 * hogy SDCC nelkul is lehessen rajta iteralni.
 *
 * Minden tema egy gen_xxx() fuggveny + egy sor az mg_topics[] tablaban.
 * A rossz valaszok tipikus tanuloi hibakbol jonnek, nem veletlenszeruek.
 *
 * Megj.: az osztas jele a szovegben ':' (magyar iskolai jeloles) - a
 * 3x5 fontban nincs '/' es ez amugy is a helyes forma.
 */

#include "mathgen.h"
#include "font.h"     /* csak az ekezet-makrokert (_AA, _OO, ...) */

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

static uint16_t rnd(uint16_t lo, uint16_t hi)
{
    if (hi <= lo) return lo;
    return (uint16_t)(lo + (mg_rand() % (uint16_t)(hi - lo + 1)));
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

static uint8_t put_str(char *dst, const char *s)
{
    uint8_t i = 0;
    while (*s) dst[i++] = *s++;
    return i;
}

/* --- valaszlista kezelese ----------------------------------------------- */
static int16_t g_lim = 999;     /* try_add felso hatara, temankent allithato */

static uint8_t contains(const int16_t *arr, uint8_t n, int16_t v)
{
    uint8_t i;
    for (i = 0; i < n; i++) if (arr[i] == v) return 1;
    return 0;
}

static uint8_t try_add(int16_t *arr, uint8_t *n, int16_t v)
{
    if (v < 0 || v > g_lim) return 0;
    if (contains(arr, *n, v)) return 0;
    arr[(*n)++] = v;
    return 1;
}

/* pool[0] a helyes valasz; feltoltes 3-ig, kiosztas, keveres, szovegezes.
 * time_fmt: 0 = sima szam, 1 = OO*100+PP kodolt ido, "O:PP" formatum */
static void finalize(MathQuestion *q, int16_t *pool, uint8_t np, uint8_t time_fmt)
{
    uint8_t i, j, n;
    int16_t tmp;
    char *t;

    /* vegso menedek: determinisztikus elcsuszas a helyes ertek korul */
    for (i = 1; np < 3 && i < 100; i++) {
        if (!try_add(pool, &np, (int16_t)(pool[0] + i)))
            try_add(pool, &np, (int16_t)(pool[0] - i));
    }

    q->correct_value = pool[0];
    for (i = 0; i < 3; i++) q->answers[i] = pool[i];

    /* keveres (Fisher-Yates) */
    q->correct_index = 0;
    for (i = 2; i > 0; i--) {
        j = (uint8_t)(mg_rand() % (uint16_t)(i + 1));
        tmp = q->answers[i]; q->answers[i] = q->answers[j]; q->answers[j] = tmp;
        if (q->correct_index == i)      q->correct_index = j;
        else if (q->correct_index == j) q->correct_index = i;
    }

    for (i = 0; i < 3; i++) {
        t = q->answer_text[i];
        if (time_fmt) {
            n = put_num(t, (int16_t)(q->answers[i] / 100));
            t[n++] = ':';
            t[n++] = (char)('0' + (q->answers[i] % 100) / 10);
            t[n++] = (char)('0' + q->answers[i] % 10);
        } else {
            n = put_num(t, q->answers[i]);
        }
        t[n] = '\0';
    }
}

/* hibajeloltek veletlen sorrendben - ne mindig ugyanaz a ketto keruljon be */
static void add_errors(int16_t *pool, uint8_t *np, const int16_t *cand, uint8_t nc)
{
    uint8_t start = (uint8_t)(mg_rand() % nc), i;
    for (i = 0; i < nc && *np < 3; i++)
        try_add(pool, np, cand[(uint8_t)((start + i) % nc)]);
}

/* ======================================================================= */
/* OSSZEADAS-KIVONAS - novekvo szamkorrel                                   */
/* tipikus hibak: kolcson/atvitel elrontva azon a helyierteken, ahol        */
/* tenyleg van; kivonasnal a klasszikus "nagyobbol a kisebbet"              */
/* ======================================================================= */
static void gen_osszeadas(MathQuestion *q, uint8_t d)
{
    static const uint16_t lo[4] = { 1, 10,  50, 100 };
    static const uint16_t hi[4] = { 20, 99, 499, 899 };
    int16_t a, b, correct, pool[3], cand[3], v, w;
    uint8_t np = 0, len, sub, nc = 0, bor1;

    g_lim = 999;
    if (d > 3) d = 3;

    a = (int16_t)rnd(lo[d], hi[d]);
    b = (int16_t)rnd(lo[d], hi[d]);
    sub = (uint8_t)(mg_rand() & 1);
    if (sub && a < b) { correct = a; a = b; b = correct; } /* csere */
    if (!sub && a + b > 999) b = (int16_t)(999 - a);
    correct = sub ? (int16_t)(a - b) : (int16_t)(a + b);

    len = put_num(q->text, a);
    q->text[len++] = sub ? '-' : '+';
    len += put_num(q->text + len, b);
    q->text[len++] = '=';
    q->text[len] = '\0';

    try_add(pool, &np, correct);
    if (sub) {
        /* kolcsonkert, de a kovetkezo helyierteket nem csokkentette --
         * csak ott jelolt, ahol tenyleg van kolcsonzes */
        bor1 = (uint8_t)(a % 10 < b % 10);
        if (bor1) cand[nc++] = (int16_t)(correct + 10);
        if ((a / 10) % 10 - bor1 < (b / 10) % 10)
            cand[nc++] = (int16_t)(correct + 100);
        /* klasszikus: helyiertekenkent a nagyobbol a kisebbet (10-2 -> 12) */
        v = (int16_t)(a % 10 - b % 10);           if (v < 0) v = (int16_t)(-v);
        w = (int16_t)((a / 10) % 10 - (b / 10) % 10); if (w < 0) w = (int16_t)(-w);
        cand[nc++] = (int16_t)((a / 100 - b / 100) * 100 + w * 10 + v);
    } else {
        /* atvitel elfelejtve -- csak ott jelolt, ahol tenyleg van atvitel */
        if (a % 10 + b % 10 >= 10) cand[nc++] = (int16_t)(correct - 10);
        if (a % 100 + b % 100 >= 100) cand[nc++] = (int16_t)(correct - 100);
    }
    if (nc) add_errors(pool, &np, cand, nc);
    /* ha nincs eleg tipikus hiba, a finalize +-1/+-2 elcsuszassal tolt fel */
    finalize(q, pool, np, 0);
}

/* ======================================================================= */
/* SZORZOTABLA - tipikus hibak: szomszedos sor/oszlop, +-a/+-b              */
/*                                                                         */
/* Nem ismetel: a mar kihuzott (a,b) parokat bitmask jeloli, es amig az    */
/* aktualis szamkor osszes parja ki nem jott, ujra nem johet egyik sem.    */
/* A forditott sorrend kulon parnak szamit (3*6 != 6*3). 2..12 -> 11x11 =  */
/* 121 bit = 16 bajt. A mask a mentes resze (mathgen.h: mg_mul_seen),     */
/* szettek es jatekulesek kozott is el.                                    */
/* ======================================================================= */
#define MUL_MIN 2
#define MUL_N   11                       /* ertekek: 2..12 */
uint8_t mg_mul_seen[MG_MUL_BYTES];

static uint8_t mul_test(uint8_t idx)
{
    return (uint8_t)(mg_mul_seen[idx >> 3] & (uint8_t)(1 << (idx & 7)));
}

/* a k-adik meg szabad par indexe az n x n-es szamkorbol (sorfolytonosan) */
static uint8_t mul_pick(uint8_t n, uint8_t k)
{
    uint8_t i, j, idx;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) {
            idx = (uint8_t)(i * MUL_N + j);
            if (mul_test(idx)) continue;
            if (k == 0) return idx;
            k--;
        }
    return 0;   /* nem fordulhat elo: k < szabad parok szama */
}

static void gen_szorzas(MathQuestion *q, uint8_t d)
{
    static const uint16_t hi[4] = { 10, 10, 10, 12 };  /* 10x10-ig, 4. szinttol 12-ig */
    int16_t a, b, correct, pool[3], cand[6];
    uint8_t np = 0, len, n, i, j, nfree, idx;

    g_lim = 999;
    if (d > 3) d = 3;
    n = (uint8_t)(hi[d] - 1);            /* 2..hi[d] -> ennyi ertek */

    nfree = 0;
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            if (!mul_test((uint8_t)(i * MUL_N + j))) nfree++;
    if (!nfree) {                        /* a szamkor kimerult: ujraindul */
        for (i = 0; i < (uint8_t)sizeof(mg_mul_seen); i++) mg_mul_seen[i] = 0;
        nfree = (uint8_t)(n * n);
    }
    idx = mul_pick(n, (uint8_t)(mg_rand() % nfree));
    mg_mul_seen[idx >> 3] |= (uint8_t)(1 << (idx & 7));

    a = (int16_t)(MUL_MIN + idx / MUL_N);
    b = (int16_t)(MUL_MIN + idx % MUL_N);
    correct = (int16_t)(a * b);

    len = put_num(q->text, a);
    q->text[len++] = '*';
    len += put_num(q->text + len, b);
    q->text[len++] = '=';
    q->text[len] = '\0';

    try_add(pool, &np, correct);
    cand[0] = (int16_t)((a + 1) * b);
    cand[1] = (int16_t)((a - 1) * b);
    cand[2] = (int16_t)(a * (b + 1));
    cand[3] = (int16_t)(a * (b - 1));
    cand[4] = (int16_t)(correct + a);
    cand[5] = (int16_t)(correct - b);
    add_errors(pool, &np, cand, 6);
    finalize(q, pool, np, 0);
}

/* ======================================================================= */
/* OSZTAS - maradek nelkul, a szorzotabla inverze                          */
/* tipikus hibak: hanyados +-1/+-2, osztoval keverve                        */
/* ======================================================================= */
static void gen_osztas(MathQuestion *q, uint8_t d)
{
    static const uint16_t hi[4] = { 10, 10, 10, 12 };  /* 10x10-ig, 4. szinttol 12-ig */
    int16_t b, qv, a, pool[3], cand[5];
    uint8_t np = 0, len;

    g_lim = 999;
    if (d > 3) d = 3;

    b  = (int16_t)rnd(2, hi[d]);
    qv = (int16_t)rnd(2, hi[d]);
    a  = (int16_t)(b * qv);

    len = put_num(q->text, a);
    q->text[len++] = ':';
    len += put_num(q->text + len, b);
    q->text[len++] = '=';
    q->text[len] = '\0';

    try_add(pool, &np, qv);
    cand[0] = (int16_t)(qv + 1);
    cand[1] = (int16_t)(qv - 1);
    cand[2] = (int16_t)(qv + 2);
    cand[3] = (int16_t)(qv - 2);
    cand[4] = b;                     /* oszto es hanyados osszekeverve */
    add_errors(pool, &np, cand, 5);
    finalize(q, pool, np, 0);
}

/* ======================================================================= */
/* SORREND - ketmuveletes kifejezes, muveleti sorrend                       */
/* tipikus hibak: balrol jobbra szamolas, elojelhiba                        */
/* ======================================================================= */
static int16_t apply(int16_t a, char op, int16_t b)
{
    switch (op) {
        case '+': return (int16_t)(a + b);
        case '-': return (int16_t)(a - b);
        case '*': return (int16_t)(a * b);
        case ':': return b ? (int16_t)(a / b) : 0;
    }
    return 0;
}

static uint8_t is_high(char op) { return (op == '*' || op == ':') ? 1 : 0; }

static int16_t eval_correct(int16_t a, char o1, int16_t b, char o2, int16_t c)
{
    if (!is_high(o1) && is_high(o2))
        return apply(a, o1, apply(b, o2, c));
    return apply(apply(a, o1, b), o2, c);
}

static int16_t eval_ltr(int16_t a, char o1, int16_t b, char o2, int16_t c)
{
    return apply(apply(a, o1, b), o2, c);
}

static void gen_sorrend(MathQuestion *q, uint8_t d)
{
    int16_t a, b, c, correct, pool[3], cand[4];
    uint8_t np = 0, len;
    char o1, o2;

    g_lim = 999;
    if (d > 3) d = 3;

    /* mindig ket muvelet, es a magas precedencia mindig a masodik tag
     * (a +- b*c ill. a +- b:c) - igy minden szinten van sorrend-csapda,
     * a balrol jobbra szamolas mindig rossz eredmenyt ad */
    for (;;) {
        o1 = (mg_rand() & 1) ? '+' : '-';
        o2 = '*';
        switch (d) {
            case 0:  /* kicsi szamok, kis szorzotabla */
                a = (int16_t)rnd(1, 20); b = (int16_t)rnd(2, 5); c = (int16_t)rnd(2, 5);
                break;
            case 1:
                a = (int16_t)rnd(5, 40); b = (int16_t)rnd(2, 10); c = (int16_t)rnd(2, 5);
                break;
            case 2:
                a = (int16_t)rnd(5, 99); b = (int16_t)rnd(2, 12); c = (int16_t)rnd(2, 9);
                break;
            default: /* felerészt osztas is */
                a = (int16_t)rnd(5, 99);
                if (mg_rand() & 1) {
                    b = (int16_t)rnd(2, 12); c = (int16_t)rnd(2, 9);
                } else {
                    c = (int16_t)rnd(2, 9);
                    b = (int16_t)(c * (int16_t)rnd(2, 11));  /* maradek nelkul */
                    o2 = ':';
                }
                break;
        }

        correct = eval_correct(a, o1, b, o2, c);
        if (correct >= 0 && correct <= 999
            && eval_ltr(a, o1, b, o2, c) != correct) break;
    }

    len = put_num(q->text, a);
    q->text[len++] = o1;
    len += put_num(q->text + len, b);
    q->text[len++] = o2;
    len += put_num(q->text + len, c);
    q->text[len++] = '=';
    q->text[len] = '\0';

    try_add(pool, &np, correct);
    /* balrol jobbra szamolt eredmeny (a leggyakoribb hiba) */
    try_add(pool, &np, eval_ltr(a, o1, b, o2, c));
    /* elojelhiba az elso muveletnel */
    try_add(pool, &np, eval_correct(a, (o1 == '+') ? '-' : '+', b, o2, c));
    cand[0] = (int16_t)(correct + 10);
    cand[1] = (int16_t)(correct - 10);
    cand[2] = (int16_t)(correct + (int16_t)rnd(1, 3));
    cand[3] = (int16_t)(correct - (int16_t)rnd(1, 3));
    add_errors(pool, &np, cand, 4);
    finalize(q, pool, np, 0);
}

/* ======================================================================= */
/* ATVALTAS - mertekegysegek: tomeg, hossz, urmertek                        */
/* tipikus hibak: rossz tizes hatvany, elmaradt/forditott valtas            */
/* ======================================================================= */
typedef struct {
    const char *names[5];   /* nagytol a kicsiig */
    uint16_t    step[4];    /* szorzo a kovetkezo (kisebb) egyseghez */
    uint8_t     n;
} MgChain;

static const MgChain chains[3] = {
    { { "T", "KG", "DKG", "G", 0 },     { 1000, 100, 10, 0 },  4 },
    { { "KM", "M", "DM", "CM", "MM" },  { 1000, 10, 10, 10 },  5 },
    { { "HL", "L", "DL", "CL", "ML" },  { 100, 10, 10, 10 },   5 },
};

static void gen_atvaltas(MathQuestion *q, uint8_t d)
{
    const MgChain *ch = &chains[0];
    int16_t n = 2, correct, pool[3], cand[3];
    uint16_t factor = 10;
    uint8_t np = 0, len, i = 2, steps = 1, rev, guard;

    g_lim = 9999;
    if (d > 3) d = 3;

    for (guard = 0; guard < 50; guard++) {
        ch    = &chains[mg_rand() % 3];
        steps = (uint8_t)((d >= 2 && (mg_rand() & 1)) ? 2 : 1);
        if (steps >= ch->n) steps = 1;
        i = (uint8_t)rnd(0, (uint16_t)(ch->n - 1 - steps));

        factor = ch->step[i];
        if (steps == 2) {
            if ((uint32_t)factor * ch->step[i + 1] > 9999) continue;
            factor = (uint16_t)(factor * ch->step[i + 1]);
        }

        n = (int16_t)rnd(1, (d == 0) ? 9 : 20);
        if ((uint32_t)n * factor > 9999) continue;
        break;
    }
    if (guard >= 50) {   /* elmeletileg nem fordul elo - biztos alapertek */
        ch = &chains[0]; i = 2; steps = 1; factor = 10; n = 2;
    }

    correct = (int16_t)(n * factor);
    rev = (uint8_t)(d >= 3 && (mg_rand() & 1));   /* kicsibol nagyba */

    if (rev) {
        /* pl. "2000 G = ? KG" */
        len  = put_num(q->text, correct);
        len += put_str(q->text + len, " ");
        len += put_str(q->text + len, ch->names[i + steps]);
        len += put_str(q->text + len, "=? ");
        len += put_str(q->text + len, ch->names[i]);
        q->text[len] = '\0';
        correct = n;
        try_add(pool, &np, correct);
        cand[0] = (int16_t)(n * 10);              /* rossz tizes hatvany */
        cand[1] = (n >= 10) ? (int16_t)(n / 10) : (int16_t)(n * 100);
        cand[2] = (int16_t)(n * factor);          /* nem valtott at      */
    } else {
        /* pl. "2 KG = ? DKG" */
        len  = put_num(q->text, n);
        len += put_str(q->text + len, " ");
        len += put_str(q->text + len, ch->names[i]);
        len += put_str(q->text + len, "=? ");
        len += put_str(q->text + len, ch->names[i + steps]);
        q->text[len] = '\0';
        try_add(pool, &np, correct);
        cand[0] = (int16_t)(correct / 10);        /* rossz tizes hatvany */
        cand[1] = (correct <= 999) ? (int16_t)(correct * 10) : (int16_t)(correct / 100);
        cand[2] = n;                              /* nem valtott at      */
    }
    add_errors(pool, &np, cand, 3);
    finalize(q, pool, np, 0);
}

/* ======================================================================= */
/* IDO - ora:perc + percek hozzaadasa                                       */
/* tipikus hibak: 60-as valtas elrontva (100-zal szamol), oralepes kihagyva */
/* Az answers[] kodolasa: OO*100 + PP (igy a "3:75" tipusu hiba is leirhato)*/
/* ======================================================================= */
static void gen_ido(MathQuestion *q, uint8_t d)
{
    int16_t h, m, add, th, tm, pool[3], cand[4];
    uint8_t np = 0, len;

    g_lim = 1299;   /* 12:xx fole nem megyunk */
    if (d > 3) d = 3;

    for (;;) {
        h = (int16_t)rnd(1, 10);
        m = (d == 0) ? (int16_t)(5 * rnd(0, 11)) : (int16_t)rnd(0, 59);
        add = (d == 0) ? (int16_t)(5 * rnd(1, 6))
            : (d == 1) ? (int16_t)(5 * rnd(1, 11))
                       : (int16_t)rnd(5, 55);
        if (d >= 1 && m + add < 60) continue;   /* d1+: legyen oraatlepes */
        if (h * 60 + m + add < 12 * 60) break;
    }

    th = (int16_t)((h * 60 + m + add) / 60);
    tm = (int16_t)((h * 60 + m + add) % 60);

    len  = put_num(q->text, h);
    q->text[len++] = ':';
    q->text[len++] = (char)('0' + m / 10);
    q->text[len++] = (char)('0' + m % 10);
    len += put_str(q->text + len, "+");
    len += put_num(q->text + len, add);
    len += put_str(q->text + len, " P=");
    q->text[len] = '\0';

    try_add(pool, &np, (int16_t)(th * 100 + tm));
    /* 100-zal szamolt (pl. 3:75) - csak amig ket szamjegyu a "perc" */
    cand[0] = (m + add <= 99) ? (int16_t)(h * 100 + m + add)
                              : (int16_t)(th * 100 + tm + 10);
    cand[1] = (m + add >= 60) ? (int16_t)(h * 100 + m + add - 60)  /* ora kihagyva */
                              : (int16_t)((h + 1) * 100 + m + add); /* felesleges +1 ora */
    cand[2] = (int16_t)(((h * 60 + m + add + 5) / 60) * 100 + (h * 60 + m + add + 5) % 60);
    cand[3] = (int16_t)(((h * 60 + m + add - 5) / 60) * 100 + (h * 60 + m + add - 5) % 60);
    add_errors(pool, &np, cand, 4);
    finalize(q, pool, np, 1);
}

/* --- temaregiszter: uj tema = gen_xxx() + egy sor ide -------------------- */
const MgTopic mg_topics[] = {
    { _OE "SSZEAD" _AA "S-KIVON" _AA "S", gen_osszeadas },
    { "SZORZ" _OO "T" _AA "BLA",     gen_szorzas   },
    { "OSZT" _AA "S",                gen_osztas    },
    { "SORREND",                     gen_sorrend   },
    { _AA "TV" _AA "LT" _AA "S",     gen_atvaltas  },
    { "ID" _OEE,                     gen_ido       },
};
const uint8_t mg_topic_count = sizeof(mg_topics) / sizeof(mg_topics[0]);

void mg_generate(uint8_t topic, MathQuestion *q, uint8_t difficulty)
{
    if (topic >= mg_topic_count) topic = 0;
    mg_topics[topic].gen(q, difficulty);
}
