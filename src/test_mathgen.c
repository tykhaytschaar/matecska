/* gcc -o /tmp/t test_mathgen.c mathgen.c && /tmp/t */
#include <stdio.h>
#include "mathgen.h"

int main(void)
{
    MathQuestion q;
    uint8_t d, i, k;
    mg_seed(1234);
    for (d = 0; d < 4; d++) {
        printf("--- difficulty %u ---\n", d);
        for (k = 0; k < 6; k++) {
            mg_generate(&q, d);
            printf("%-12s", q.text);
            for (i = 0; i < 3; i++)
                printf("  %c) %-4s%s", 'A' + i, q.answer_text[i],
                       i == q.correct_index ? "*" : " ");
            printf("\n");
        }
    }
    return 0;
}
