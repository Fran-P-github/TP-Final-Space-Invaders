#ifndef SCORES_H
#define SCORES_H

#include <stdbool.h>

#define MAX_SCORES 10
#define NAME_LEN 3

typedef struct {
    char name[NAME_LEN+1];
    int score;
} highscore_t;

// Carga el top 10 desde el archivo
void load_scores(highscore_t top_scores[]);

// Guarda el top 10 en el archivo
void save_scores(const highscore_t top_scores[]);

// Inserta un nuevo puntaje si corresponde, devuelve true si entra al top
bool try_insert_score(highscore_t top_scores[], const char *name, int score);

#endif // SCORES_H
