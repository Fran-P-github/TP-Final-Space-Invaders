#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "scores.h"

#if PLATFORM == RPI
#define SCORE_FILE "scores_rpi.csv"
#elif PLATFORM == ALLEGRO
#define SCORE_FILE "scores_allegro.csv"
#else
#error no PLATFORM defined or invalid definition
#endif

void load_scores(highscore_t top_scores[]) {
    FILE *f = fopen(SCORE_FILE, "r");
    if (!f) {
        for (int i = 0; i < MAX_SCORES; ++i) {
            strcpy(top_scores[i].name, "---");
            top_scores[i].score = 0;
        }
        return;
    }

    char line[64];
    int i = 0;
    while (fgets(line, sizeof(line), f) && i < MAX_SCORES) {
        char *comma = strchr(line, ',');
        if (!comma) continue;
        *comma = '\0';
        strncpy(top_scores[i].name, line, NAME_LEN);
        top_scores[i].name[NAME_LEN] = '\0';
        top_scores[i].score = atoi(comma + 1);
        ++i;
    }

    fclose(f);
}

void save_scores(const highscore_t top_scores[]) {
    FILE *f = fopen(SCORE_FILE, "w");
    if (!f) return;

    for (int i = 0; i < MAX_SCORES; ++i) {
        fprintf(f, "%s,%d\n", top_scores[i].name, top_scores[i].score);
    }

    fclose(f);
}

bool try_insert_score(highscore_t top_scores[], const char *name, int score) {
    int pos = -1;
    for (int i = 0; i < MAX_SCORES; ++i) {
        if (score > top_scores[i].score) {
            pos = i;
            break;
        }
    }

    if (pos == -1) return false;

    for (int i = MAX_SCORES - 1; i > pos; --i) {
        top_scores[i] = top_scores[i - 1];
    }

    strncpy(top_scores[pos].name, name, NAME_LEN+1);
    top_scores[pos].name[NAME_LEN] = '\0';
    top_scores[pos].score = score;
    return true;
}
