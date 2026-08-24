/* MATECSKA - feladatgenerator interfesz */
#ifndef MATHGEN_H
#define MATHGEN_H

#include <stdint.h>

typedef struct {
    char    text[16];           /* pl. "2 KG = ? DKG"                */
    int16_t answers[3];         /* numerikus ertek (belso; idonel OO*100+PP) */
    char    answer_text[3][6];  /* megjelenitendo szoveg ("2000", "12:45")  */
    uint8_t correct_index;      /* 0..2 - melyik tal a helyes        */
    int16_t correct_value;
} MathQuestion;

/* --- temaregiszter ------------------------------------------------------ */
typedef void (*MgGenFn)(MathQuestion *q, uint8_t difficulty);

typedef struct {
    const char *name;   /* menufelirat, 3x5 font keszlete, max 17 kar. */
    MgGenFn     gen;
} MgTopic;

extern const MgTopic mg_topics[];
extern const uint8_t mg_topic_count;

/* felso hatar a temankent nyilvantartott allapotokhoz (mentes, tombok) */
#define MG_MAX_TOPICS 8

/* a szorzotabla mar-kihuzott parjainak bitmaskja - a mentes resze,
 * ezert publikus; tartalmat csak a gen_szorzas kezeli */
#define MG_MUL_BYTES 16
extern uint8_t mg_mul_seen[MG_MUL_BYTES];

void mg_seed(uint16_t s);
void mg_generate(uint8_t topic, MathQuestion *q, uint8_t difficulty);

#endif
