/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     scores.h

     Description:
     Handles hi-scores saving and reading.
     Provides highscore_t struct to handle hi-scores


  ******************************************************************************/

#ifndef SCORES_H
#define SCORES_H

#include <stdbool.h>

#if PLATFORM == RPI
#define MAX_SCORES 5
#elif PLATFORM == ALLEGRO
#define MAX_SCORES 20
#else
#define MAX_SCORES 10
#endif

#define NAME_LEN 3

typedef struct {
  char name[NAME_LEN + 1];
  int score;
} highscore_t;

// Loads the top scores from the file
void load_scores(highscore_t top_scores[]);

// Saves the top scores to the file
void save_scores(const highscore_t top_scores[]);

// Inserts a new score if applicable, returns true if it makes it into the top
bool try_insert_score(highscore_t top_scores[], const char *name, int score);

#endif // SCORES_H
