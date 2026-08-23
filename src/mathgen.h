/* MATECSKA - feladatgenerator interfesz */
#ifndef MATHGEN_H
#define MATHGEN_H

#include <stdint.h>

typedef struct {
    char    text[12];           /* pl. "36+9*2="                     */
    int16_t answers[3];         /* a harom valasz, mar megkeverve    */
    char    answer_text[3][5];  /* ugyanaz szovegkent                */
    uint8_t correct_index;      /* 0..2 - melyik tal a helyes        */
    int16_t correct_value;
} MathQuestion;

void mg_seed(uint16_t s);
void mg_generate(MathQuestion *q, uint8_t difficulty);

#endif
