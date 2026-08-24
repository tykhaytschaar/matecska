/* MATECSKA - feladatgenerator teszt (natic, gcc: make test)
 *
 * Minden temat minden nehezsegen sokszor legeneral, invariansokat
 * ellenoriz, es temankent kiir par mintat szemrevetelezesre.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mathgen.h"

static int failures = 0;

/* a temanevekben ekezet-makrok vannak (font tile-kodok) - visszairjuk
   ekezet nelkuli betunek, hogy a terminalban olvashato legyen */
static void print_name(const char *s)
{
    static const char acc[] = "AEIOOOUUU";
    unsigned char c;
    while ((c = (unsigned char)*s++) != 0) {
        if (c >= 0xAB && c <= 0xB3) putchar(acc[c - 0xAB]);
        else putchar(c);
    }
}

static void check(int cond, const char *msg, const MathQuestion *q,
                  uint8_t topic, uint8_t d)
{
    if (!cond) {
        failures++;
        printf("HIBA [");
        print_name(mg_topics[topic].name);
        printf(" d=%d] %s  text='%s' valaszok: %d|%d|%d helyes: #%d\n",
               d, msg, q->text,
               q->answers[0], q->answers[1], q->answers[2], q->correct_index);
    }
}

int main(void)
{
    MathQuestion q;
    uint8_t t, d;
    int i, n;

    mg_seed(12345);

    for (t = 0; t < mg_topic_count; t++) {
        for (d = 0; d < 4; d++) {
            for (i = 0; i < 500; i++) {
                memset(&q, 0xAA, sizeof(q));
                mg_generate(t, &q, d);

                check(q.correct_index < 3, "correct_index >= 3", &q, t, d);
                check(q.answers[q.correct_index] == q.correct_value,
                      "correct_value nem egyezik", &q, t, d);
                check(q.answers[0] != q.answers[1] &&
                      q.answers[0] != q.answers[2] &&
                      q.answers[1] != q.answers[2],
                      "azonos valaszok", &q, t, d);
                check(strlen(q.text) > 0 && strlen(q.text) < sizeof(q.text),
                      "text hossz", &q, t, d);
                for (n = 0; n < 3; n++)
                    check(strlen(q.answer_text[n]) > 0 &&
                          strlen(q.answer_text[n]) < sizeof(q.answer_text[n]),
                          "answer_text hossz", &q, t, d);
            }
        }
    }

    /* mintak szemrevetelezesre */
    for (t = 0; t < mg_topic_count; t++) {
        printf("--- ");
        print_name(mg_topics[t].name);
        printf(" ---\n");
        for (d = 0; d < 4; d++) {
            for (i = 0; i < 2; i++) {
                mg_generate(t, &q, d);
                printf("  d%d  %-15s", d, q.text);
                for (n = 0; n < 3; n++)
                    printf(" %c) %-5s%c", 'A' + n, q.answer_text[n],
                           (n == q.correct_index) ? '*' : ' ');
                printf("\n");
            }
        }
    }

    if (failures) {
        printf("\n%d HIBA\n", failures);
        return 1;
    }
    printf("\nOK - minden invarians teljesul (%d tema x 4 szint x 500)\n",
           mg_topic_count);
    return 0;
}
