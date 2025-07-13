/***************************************************************************/ /**
     TP-FINAL-SPACE-INVADERS
     front_allegro.c
     25/6/25

     Description:



  ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define _IS_BACK_C_
#include "back.h"

/*******************************************************************************
 * PREPROCESSOR CONSTANT AND MACRO DEFINITIONS
 ******************************************************************************/

#define MAX_ULL -1ULL

#define INITIAL_PLAYER_X_COORDINATE ((WORLD_WIDTH - PLAYER_W) / 2)

#if PLATFORM == ALLEGRO

#define ALIENS_MOVE_MIN_INTERVAL 0.01
#define ALIENS_MOVE_MAX_INTERVAL 0.5

#define MOTHERSHIP_X_VELOCITY 180
#define MOTHERSHIP_DX (MOTHERSHIP_X_VELOCITY / FRAME_RATE)

#define ALIENS_X_VELOCITY 500
#define ALIENS_Y_VELOCITY 500
#define ALIENS_DX (ALIENS_X_VELOCITY / FRAME_RATE)
#define ALIENS_DY (ALIENS_Y_VELOCITY / FRAME_RATE)

#define PLAYER_VELOCITY 200
#define PLAYER_DX (PLAYER_VELOCITY / FRAME_RATE)

#define SHOT_VELOCITY_ALIEN 200
#define SHOT_VELOCITY_PLAYER 300
#define SHOT_DY_ALIEN (SHOT_VELOCITY_ALIEN / FRAME_RATE)
#define SHOT_DY_PLAYER (SHOT_VELOCITY_PLAYER / FRAME_RATE)

#elif PLATFORM == RPI

#define ALIENS_MOVE_MIN_INTERVAL 0.1
#define ALIENS_MOVE_MAX_INTERVAL 0.8

#define MOTHERSHIP_DX 0.05

#define ALIENS_DX 1
#define ALIENS_DY 1

#define PLAYER_DX 1

#define SHOT_DY_ALIEN 0.1
#define SHOT_DY_PLAYER 0.1

#endif

/*******************************************************************************
 * GLOBAL PUBLIC VARIABLES
 ******************************************************************************/

bool aliensMoved = false;
bool alienWasHit = false;
bool alienWasKilled = false;
bool playerDied = false;
bool shieldWasHit = false;

/*******************************************************************************
 * ENUMERATIONS, STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
  int x, y;
  int lives;
  int points; // Point given to player when killed
  alien_type_t type;
} alien_t;

typedef struct {
  int x, y;
  int lives;
  int score;
} player_t;

struct shield {
  int x, y;
  int lives; // Shots that a block of the shield can resist
};
typedef struct shield shield_t[SHIELD_H][SHIELD_W];

typedef struct {
  int x;
#if PLATFORM == ALLEGRO
  int y;
#elif PLATFORM == RPI
  double y;
#endif
  int dy;
  bool is_used;
} shot_t;

typedef enum movement {
  MOVEMENT_RIGHT = 0,
  MOVEMENT_LEFT,
  MOVEMENT_DOWN,
  NO_MOVEMENT
} movement_t;

typedef struct {
  #if PLATFORM == ALLEGRO
  int x;
#elif PLATFORM == RPI
  double x;
#endif
  int y;
  int dx;
  bool is_active;
  int points; // Point given to player when shot
} mothership_t;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

// Inits with aliens_rows and aliens_cols alive. Lives go from lives_min to lives_max
static void aliens_init(unsigned aliens_rows, unsigned aliens_cols, unsigned lives_min, unsigned lives_max);
// Inits player. Lives are as defined in general_defines
static void player_init();
// Reset player lives to number specified in general_defines
static void player_reset_lives();
// Inits shields with specified lives
static void shields_init(unsigned lives);
// Inits one shield in given coordinates
static void shield_init(unsigned shield, int x, int y, unsigned lives);
// Resets player status for a new level
static void player_reset_on_new_level();

// Returns true if aliens win (reach the bottom of the screen)
static bool aliens_update(unsigned current_level, bool new_level);
static void mothership_update();
// Returns true when aliens hit player
static bool shots_update(); 

// Detects collition between a and b
static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2);

// Returns true when mothership is to be spawned. Probability depends on elapsed_time
static bool should_spawn_mothership(double elapsed_time);

static void player_shot_update();
// Returns true when aliens hit player
static bool alien_shot_update();

// Move player
static void player_move(int x, int y);

// Move alien and wrappers

static void aliens_move(int x, int y, unsigned row);
static void aliens_move_right(unsigned row);
static void aliens_move_left(unsigned row);
static void aliens_move_down(unsigned row);

// Updates aliens position and returns type of movement done this frame
static movement_t aliens_update_position(unsigned row, bool new_level);
// Updates aliens speed depending on level and aliens alive
static void update_aliens_speed(unsigned current_level);
// Detects collition to shields when aliens are close to bottom of the screen
void aliens_shield_collition();

// Returns: index of column above player, or -1 if none is
static int get_alien_column_above_player();
// Returns: index of column above a shield block, or -1 if none is. TODO: que no devuelva siempre la columna de mas a la izq, mejor que sea una random
static int get_alien_column_above_shield();
// Returns: how many aliens are alive in column c
static unsigned aliens_alive_in_column(unsigned c);

// Returns: how many aliens are alive in row r
static unsigned aliens_alive_in_row(unsigned r);

// Returns: lowest index for row with alive aliens
static int get_top_alien_row();

// Returns greatest index of row with aliens alive
static int get_lowest_alien_row();

// Returns: index of the lowest alien alive in the column, or -1 if no aliens are alive
#define lowest_alien_alive_index(c) (aliens_alive_in_column(c) - 1)

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static player_t player;

static shield_t shields[SHIELDS_CANT];

static alien_t aliens[ALIENS_ROWS][ALIENS_COLUMNS];
static double aliens_move_interval; // Seconds. Time in between aliens movements

// Player and Aliens can have only one active shot at a time
static shot_t player_shot;
static shot_t alien_shot;

static mothership_t mothership;

explosion_t current_explosion;

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

unsigned long long get_millis() {
#if defined(__linux__) || defined(__unix__)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long long) (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#elif defined(_WIN32)
  static LARGE_INTEGER freq;
  static BOOL initialized = FALSE;
  if ( !initialized ) {
    QueryPerformanceFrequency(&freq);
    initialized = TRUE;
  }
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return (unsigned long long) ((counter.QuadPart * 1000) / freq.QuadPart);
#else
#error "get_millis() not implemented for this platform"
#endif
}

void shuffle(unsigned *array, unsigned n) {
  for (unsigned i = n - 1; i > 0; --i) {
    unsigned j = rand() % (i + 1);
    unsigned tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
}

int rand_between(int lo, int hi) {
  return lo + rand() % (hi - lo + 1);
}

float rand_between_f(float lo, float hi){
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

int mothership_get_x() {
  return mothership.x;
}
int mothership_get_y() {
  return mothership.y;
}
bool mothership_is_active() {
  return mothership.is_active;
}
int mothership_get_points(){
  return mothership.points;
}

int player_get_x() {
  return player.x;
}
int player_get_y() {
  return player.y;
}
int player_get_lives() {
  return player.lives;
}
int player_get_score() {
  return player.score;
}
int player_shot_get_x() {
  return player_shot.x;
}
int player_shot_get_y() {
  return player_shot.y;
}
bool player_shot_is_used() {
  return player_shot.is_used;
}

#define VALIDATE_ALIEN_COORDINATES(i,j) if(i >= ALIENS_ROWS || j >= ALIENS_COLUMNS) {fprintf(stderr, "Invalid alien index: (%u, %u)\n", i, j); exit(-1);}
int aliens_get_x(unsigned i, unsigned j) {
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].x;
}
int aliens_get_y(unsigned i, unsigned j) {
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].y;
}
int aliens_get_points(unsigned i, unsigned j){
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].points;
}
bool aliens_is_alive(unsigned i, unsigned j) {
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].lives;
}
int aliens_get_lives(unsigned i, unsigned j) {
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].lives;
}
alien_type_t alines_get_type(unsigned i, unsigned j){
  VALIDATE_ALIEN_COORDINATES(i,j)
  return aliens[i][j].type;
}
int alien_shot_get_x() {
  return alien_shot.x;
}
float aliens_get_relative_speed(){
  return (ALIENS_MOVE_MAX_INTERVAL - aliens_move_interval) /
           (ALIENS_MOVE_MAX_INTERVAL - ALIENS_MOVE_MIN_INTERVAL);
}
int alien_shot_get_y() {
  return alien_shot.y;
}
bool alien_shot_is_used() {
  return alien_shot.is_used;
}

#define VALIDATE_SHIELD_AND_COORDINATES(s,y,x) if(s>=SHIELDS_CANT || y>=SHIELD_H || x>=SHIELD_W) {fprintf(stderr, "Invalid shield or block: (shield:%u ; coord: %u,%u)\n", s, x, y); exit(-1);}
int shield_get_x(unsigned s, unsigned y, unsigned x) {
  return shields[s][y][x].x;
}
int shield_get_y(unsigned s, unsigned y, unsigned x) {
  return shields[s][y][x].y;
}
int shield_get_lives(unsigned s, unsigned y, unsigned x) {
  return shields[s][y][x].lives;
}

void back_init() {
  srand(time(NULL));
  player_init();
}

void level_init(unsigned level, unsigned aliens_rows, unsigned aliens_cols, unsigned aliens_lives_min, unsigned aliens_lives_max, unsigned shield_block_lives) {
  player_reset_on_new_level();
  aliens_init(aliens_rows, aliens_cols, aliens_lives_min, aliens_lives_max);
  shields_init(shield_block_lives);
  update_aliens_speed(level); // Set aliens_move_interval on level start
  aliensMoved = false;
  alienWasHit = false;
  alienWasKilled = false;
  playerDied = false;
  shieldWasHit = false;
  current_explosion.type = NO_EXPLOSION;
  mothership.is_active = false;
}

bool get_explosion_state(explosion_t* explosion_state){
  bool result = true;
  if(!explosion_state || current_explosion.type == NO_EXPLOSION) result = false;
  else memcpy(explosion_state, &current_explosion, sizeof(explosion_t));
  return result;
}

int get_best_alien_column_to_shoot() {
  int col;
  col = get_alien_column_above_player();
  if ( col >= 0 ) return col;
  col = get_alien_column_above_shield();
  if ( col >= 0 ) return col;
  return -1;
}

void player_reset_on_new_game() {
  player_reset_lives();
  player.score = 0;
  playerDied = false;
}

void player_move_right() {
  if ( player.x + PLAYER_DX + PLAYER_W <= WORLD_WIDTH - PLAYER_MARGIN )
    player_move(PLAYER_DX, 0);
}
void player_move_left() {
  if ( player.x - PLAYER_DX >= PLAYER_MARGIN )
    player_move(-PLAYER_DX, 0);
}

level_state_t back_update(unsigned current_level, bool new_level) {
  shieldWasHit = false;
  alienWasHit = false;
  alienWasKilled = false;
  current_explosion.type = NO_EXPLOSION;

  static unsigned long long player_death_start = MAX_ULL; // Variable is MAX_ULL while player is not in death state
  
  if(shots_update()){
    if(player.lives <= 0) return ALIENS_WIN;

    player_death_start = get_millis();
  }

  mothership_update();

  if ( aliens_update(current_level, new_level) ) {
    player_death_start = MAX_ULL;
    return ALIENS_WIN;
  }

  if ( total_aliens_alive() == 0 ) {
    player.lives++;
    player_death_start = MAX_ULL;
    return PLAYER_WINS;
  }

  double elapsed = (double) (get_millis() - player_death_start);
  if(elapsed <= 700) return LEVEL_NOT_DONE;
  if(player_death_start != MAX_ULL){
    bool shot_before = player_shot.is_used; // player_reset_on_new_level will set player_shot.is_used to false, we undo that
    player_reset_on_new_level();
    player_shot.is_used = shot_before;
    player_death_start = MAX_ULL;
  }

  return LEVEL_NOT_DONE;
}

bool player_try_shoot() {
  if ( player_shot.is_used || playerDied ) return false;

  player_shot.is_used = true;
  player_shot.x = player.x + PLAYER_W / 2 - SHOT_W / 2;
  player_shot.y = player.y - SHOT_H;
  return true;
}

bool alien_try_shoot(unsigned c) {
  unsigned alive_aliens = aliens_alive_in_column(c);
  if ( alien_shot.is_used || !alive_aliens ) return false;
  alien_shot.is_used = true;
  alien_shot.x = aliens[alive_aliens - 1][c].x + ALIENS_W / 2 - SHOT_W / 2;
  alien_shot.y = aliens[alive_aliens - 1][c].y + ALIENS_H;
  return true;
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void player_reset_on_new_level() {
  player.x = INITIAL_PLAYER_X_COORDINATE;
  player.y = WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H;
  player_shot.is_used = false;
  playerDied = false;
}

static bool aliens_update(unsigned current_level, bool new_level) {
  if ( !total_aliens_alive() ) return false;

  static unsigned row_to_move = ALIENS_ROWS - 1; // Move lowest row first
  if ( new_level ) {
    row_to_move = get_lowest_alien_row();
  }

  // Decrement row_to_move, based on las aliens movement. This will be used by next frame
  movement_t last_movement;
  if ( (last_movement = aliens_update_position(row_to_move, new_level)) != NO_MOVEMENT )
    row_to_move = (row_to_move - 1) >= ALIENS_ROWS ? (ALIENS_ROWS - 1) : (row_to_move - 1);
  // Get next row with alive aliens
  while ( !aliens_alive_in_row(row_to_move) ) {
    row_to_move = (row_to_move - 1) >= ALIENS_ROWS ? ALIENS_ROWS - 1 : (row_to_move - 1);
  }

  update_aliens_speed(current_level);
  aliens_shield_collition();

  unsigned i;
  for ( i = ALIENS_ROWS - 1; i < ALIENS_ROWS; --i ) {
    if ( aliens_alive_in_row(i) && aliens[i][0].y + ALIENS_H - 1 >= player.y ) return true;
  }

  return false;
}

static bool shots_update() {
  player_shot_update();
  return alien_shot_update();
}

static void mothership_update() {
  static unsigned long long start = 0;
  double elapsed = (double) (get_millis() - start) / 1000;
  if ( !mothership.is_active && !should_spawn_mothership(elapsed) ) return; // Mothership inactive and not activated yet
  start = get_millis();

  static bool spawn_right;

  if ( !mothership.is_active ) {
    spawn_right = rand() % 2;

    // Select points for mothership
    int r = rand_between(1, 100); // Random 1 to 100
    if(r <= 57) {
        mothership.points = MOTHERSHIP_POINTS_SILVER;  // ~57%
    } else if(r <= 86) {
        mothership.points = MOTHERSHIP_POINTS_GOLD;    // ~29%
    } else {
        mothership.points = MOTHERSHIP_POINTS_NEON; // ~14%
    }

    mothership.is_active = true;
    mothership.y = MOTHERSHIP_MARGIN;
    if ( spawn_right ) {
      mothership.x = WORLD_WIDTH;
    } else {
      mothership.x = -MOTHERSHIP_W;
    }
  }

  if ( spawn_right ) {
    if ( mothership.x + MOTHERSHIP_W - 1 < 0 ) {
      mothership.is_active = false;
    }
    mothership.x -= MOTHERSHIP_DX;
  } else {
    if ( mothership.x > WORLD_WIDTH - 1 ) {
      mothership.is_active = false;
    }
    mothership.x += MOTHERSHIP_DX;
  }
}

static bool collide(int ax1, int ay1, int ax2, int ay2, int bx1, int by1, int bx2, int by2) {
  if ( ax1 > bx2 ) return false;
  if ( ax2 < bx1 ) return false;
  if ( ay1 > by2 ) return false;
  if ( ay2 < by1 ) return false;

  return true;
}

static bool should_spawn_mothership(double elapsed_time) {
#if PLATFORM == ALLEGRO
  const double max_prob = 0.5; // max
  const double rate = 0.00005; // increase per second
#elif PLATFORM == RPI
  const double max_prob = 0.5; // max
  const double rate = 0.00005;    // increase per second
#endif

  double probability = elapsed_time * rate;

  // Clamp to the maximum 
  if ( probability > max_prob )
    probability = max_prob;

  //generates a number between 0 and 1
  double r = (double) rand() / RAND_MAX;

  return r < probability;
}

#define FIRST_ALIEN_X_COORDINATE ((WORLD_WIDTH - (cols * ALIENS_W + (cols - 1) * ALIENS_HORIZONTAL_SEPARATION)) / 2)
static void aliens_init(unsigned rows, unsigned cols, unsigned lives_min, unsigned lives_max) {
  if(!lives_min) lives_min = 1;
  if(lives_max < lives_min) lives_max = lives_min;

  unsigned i, j;
  int x = FIRST_ALIEN_X_COORDINATE;
  int y = ALIENS_MARGIN;
  for ( i = 0; i < ALIENS_ROWS; ++i ) {
    for ( j = 0; j < ALIENS_COLUMNS; ++j ) {
      if(i + 1 > (double)2/3 * rows){
        aliens[i][j].type = ALIEN_FAT;
        aliens[i][j].points = ALIEN_FAT_POINTS;
      }else if(i + 1 > (double)1/3 * rows){
        aliens[i][j].type = ALIEN_WINGED;
        aliens[i][j].points = ALIEN_WINGED_POINTS;
      }else{
        aliens[i][j].type = ALIEN_SMALL;
        aliens[i][j].points = ALIEN_SMALL_POINTS;
      }
      
      // 2% neon, 4% gold, 8%, silver
      int r = rand_between(1, 100);
      if(r <= 2) {
          aliens[i][j].points = ALIEN_POINTS_NEON;
      } else if(r <= 6) {
          aliens[i][j].points = ALIEN_POINTS_GOLD;
      } else if(r <= 14) {
          aliens[i][j].points = ALIEN_POINTS_SILVER;
      }

      aliens[i][j].x = x;
      aliens[i][j].y = y;

      if((i < rows && j < cols)){
        aliens[i][j].lives = rows > 1 ? lives_min + ( (lives_max - lives_min) * i ) / (rows - 1) : lives_max;
      }else{
        aliens[i][j].lives = 0;
      }

      x += ALIENS_W + ALIENS_HORIZONTAL_SEPARATION;
    }
    y += ALIENS_H + ALIENS_VERTICAL_SEPARATION;
    x = FIRST_ALIEN_X_COORDINATE;
  }
  alien_shot.is_used = false;
}

static void player_reset_lives() {
  player.lives = PLAYER_INITIAL_LIVES;
}

static void player_init() {
  player_reset_on_new_game();
  player_reset_on_new_level();
}

#define INITIAL_SHIELD_Y_COORDINATE (WORLD_HEIGHT - PLAYER_MARGIN - PLAYER_H - SHIELD_TO_PLAYER_MARGIN - SHIELD_H * SHIELD_BLOCK_H)
static void shields_init(unsigned lives) {
  unsigned i;
  for ( i = 0; i < SHIELDS_CANT; ++i ) {
    int x = (i + 1) * WORLD_WIDTH / (SHIELDS_CANT + 1) - SHIELD_W * SHIELD_BLOCK_W / 2;
    int y = INITIAL_SHIELD_Y_COORDINATE;
    shield_init(i, x, y, lives);
  }
}

static void shield_init(unsigned k, int x, int y, unsigned lives) {
  char form[SHIELD_H][SHIELD_W] = SHIELD_FORM;
  unsigned i, j;
  for ( i = 0; i < SHIELD_H; ++i ) {
    for ( j = 0; j < SHIELD_W; ++j ) {
      shields[k][i][j].x = x + j * SHIELD_BLOCK_W;
      shields[k][i][j].y = y + i * SHIELD_BLOCK_H;
      shields[k][i][j].lives = (form[i][j] == '*') ? lives : 0;
    }
  }
}

static void player_move(int x, int y) {
  if(playerDied) return;
  player.x += x;
  player.y += y;
}

static void aliens_move(int x, int y, unsigned row) {
  unsigned j;
  for ( j = 0; j < ALIENS_COLUMNS; ++j ) {
    aliens[row][j].x += x;
    aliens[row][j].y += y;
  }
}

static void aliens_move_right(unsigned row) {
  aliens_move(ALIENS_DX, 0, row);
}
static void aliens_move_left(unsigned row) {
  aliens_move(-ALIENS_DX, 0, row);
}
static void aliens_move_down(unsigned row) {
  aliens_move(0, ALIENS_DY, row);
}

static int get_top_alien_row() {
  unsigned i;
  for ( i = 0; i < ALIENS_ROWS; ++i ) {
    if ( aliens_alive_in_row(i) ) return i;
  }
  return -1;
}

static int get_lowest_alien_row(){
    for(int i = ALIENS_ROWS-1; i >= 0; --i){
        if(aliens_alive_in_row(i)){
            return i;
        }
    }
    return -1;
}

static int get_alien_column_above_player() {
  if(playerDied) return -1;

  for ( unsigned j = 0; j < ALIENS_COLUMNS; ++j ) {
    int row = lowest_alien_alive_index(j);
    if ( !aliens[row][j].lives ) continue;

    int ax1 = aliens[row][j].x;
    int ax2 = ax1 + ALIENS_W - 1;
    int px1 = player.x;
    int px2 = px1 + PLAYER_W - 1;

    // If there is horizontal overlap between alien and player
    if ( !(ax2 < px1 || ax1 > px2) ) {
      return j;
    }
  }

  return -1; // No alien is above player
}

static int get_alien_column_above_shield() {
  unsigned columns[ALIENS_COLUMNS];
  for (unsigned i = 0; i < ALIENS_COLUMNS; ++i) {
    columns[i] = i;
  }
  shuffle(columns, ALIENS_COLUMNS);

  for (unsigned idx = 0; idx < ALIENS_COLUMNS; ++idx) {
    unsigned j = columns[idx]; // Random column
    int row = lowest_alien_alive_index(j);
    if ( row < 0 || aliens[row][j].lives < 0 ) continue;

    int ax1 = aliens[row][j].x;
    int ax2 = ax1 + ALIENS_W - 1;

    for ( unsigned s = 0; s < SHIELDS_CANT; ++s ) {
      for ( unsigned y = 0; y < SHIELD_H; ++y ) {
        for ( unsigned x = 0; x < SHIELD_W; ++x ) {
          if ( !shields[s][y][x].lives ) continue;

          int sx1 = shields[s][y][x].x;
          int sx2 = sx1 + SHIELD_BLOCK_W - 1;

          // If there is horizontal overlap
          if ( !(ax2 < sx1 || ax1 > sx2) ) {
            return j; // Random alien column overlapping with a shield
          }
        }
      }
    }
  }

  return -1; // No column overlapping with shields
}

unsigned total_aliens_alive() {
  unsigned count = 0;
  for ( unsigned i = 0; i < ALIENS_ROWS; ++i ) {
    for ( unsigned j = 0; j < ALIENS_COLUMNS; ++j ) {
      if ( aliens[i][j].lives )
        ++count;
    }
  }
  return count;
}

static unsigned aliens_alive_in_column(unsigned c) {
  if ( c >= ALIENS_COLUMNS ) return 0;
  unsigned i;
  for ( i = ALIENS_ROWS - 1; i < ALIENS_ROWS; --i ) {
    if ( aliens[i][c].lives ) return i + 1;
  }
  return 0;
}

static unsigned aliens_alive_in_row(unsigned r) {
  if ( r >= ALIENS_ROWS ) return 0;
  unsigned rta = 0;
  unsigned i;
  for ( i = 0; i < ALIENS_COLUMNS; ++i ) {
    if ( aliens[r][i].lives ) ++rta;
  }
  return rta;
}

static void update_aliens_speed(unsigned level) {

  unsigned total = ALIENS_ROWS * ALIENS_COLUMNS;
  unsigned alive = total_aliens_alive();

  double alive_ratio = (double) alive / total;

  // The fewer aliens there are, the faster they move. Also increases with level.
  aliens_move_interval = ALIENS_MOVE_MAX_INTERVAL * alive_ratio;

  // Apply scaling based on level (increases base speed)
  aliens_move_interval /= (1 + 1. * level); // 20% faster per level

  // Clamp to the minimum
  if ( aliens_move_interval < ALIENS_MOVE_MIN_INTERVAL )
    aliens_move_interval = ALIENS_MOVE_MIN_INTERVAL;
}

static movement_t aliens_update_position(unsigned row, bool new_level) {
  if ( row >= ALIENS_ROWS ) return false;
  static unsigned long long start = 0;
  double elapsed = (double) (get_millis() - start) / 1000;
  int i, j;
  static movement_t movement = MOVEMENT_RIGHT;
  static movement_t movement_post_down = MOVEMENT_LEFT;
  if(new_level){
    movement = MOVEMENT_RIGHT;
    movement_post_down = MOVEMENT_LEFT;
  }
  
  int topRow = get_top_alien_row();

  // Check movement direction
  if(movement != MOVEMENT_DOWN){
    for ( i = ALIENS_COLUMNS - 1; i >= 0; --i ) {
      if ( aliens_alive_in_column(i) ){
        for ( j = ALIENS_ROWS - 1; j >= 0; --j ){
          if(!aliens_alive_in_row(j)) continue;
          if (movement==MOVEMENT_RIGHT && (aliens[j][i].x + ALIENS_W-1 + ALIENS_DX > WORLD_WIDTH-1) ) { // Reached right side of the screen
            if(j == topRow){
              movement = MOVEMENT_DOWN;
              movement_post_down = MOVEMENT_LEFT;
            }
          }
          else if (movement==MOVEMENT_LEFT && (aliens[j][i].x - ALIENS_DX < 0) ) { // Reached left side of the screen
            if(j == topRow){
              movement = MOVEMENT_DOWN;
              movement_post_down = MOVEMENT_RIGHT;
            }
          }
        }
      }
    }
  }

  // Move aliens
  aliensMoved = elapsed >= aliens_move_interval;
  if ( elapsed >= aliens_move_interval) {
    start = get_millis();
    switch ( movement ) {
      case MOVEMENT_RIGHT:
        aliens_move_right(row);
        break;
      case MOVEMENT_LEFT:
        aliens_move_left(row);
        break;
      case MOVEMENT_DOWN:
        aliens_move_down(row);
        if (row == topRow) {
          if ( movement_post_down == MOVEMENT_RIGHT ) {
            movement = MOVEMENT_RIGHT;
            movement_post_down = MOVEMENT_LEFT;
          } else {
            movement = MOVEMENT_LEFT;
            movement_post_down = MOVEMENT_RIGHT;
          }
        }
        break;
      case NO_MOVEMENT:
        break;
    }
    return movement;
  }
  return NO_MOVEMENT;
}

void aliens_shield_collition() {
  // Shield collition
  unsigned shield, i, j;
  unsigned alien_row, alien_column;
  for ( shield = 0; shield < SHIELDS_CANT; ++shield ) {
    for ( i = 0; i < SHIELD_H; ++i ) {
      for ( j = 0; j < SHIELD_W; ++j ) {
        if ( shields[shield][i][j].lives == 0 )
          continue;

        for ( alien_row = 0; alien_row < ALIENS_ROWS; ++alien_row ) {
          for ( alien_column = 0; alien_column < ALIENS_COLUMNS; ++alien_column ) {
            if ( !aliens[alien_row][alien_column].lives )
              continue;

            if ( collide(
                     aliens[alien_row][alien_column].x, aliens[alien_row][alien_column].y, aliens[alien_row][alien_column].x + ALIENS_W - 1, aliens[alien_row][alien_column].y + ALIENS_H - 1, shields[shield][i][j].x, shields[shield][i][j].y, shields[shield][i][j].x + SHIELD_BLOCK_W - 1, shields[shield][i][j].y + SHIELD_BLOCK_H - 1) ) {

              shields[shield][i][j].lives = 0;
              goto next_block; // Done with this block, check for next block
            }
          }
        }
      next_block:;
      }
    }
  }
}

static void player_shot_update() {
  if ( !player_shot.is_used ) return;

  player_shot.y -= SHOT_DY_PLAYER;
  unsigned i, j;

  // Mothership collition
  if (mothership.is_active && collide(player_shot.x, player_shot.y, player_shot.x + SHOT_W - 1, player_shot.y + SHOT_H - 1, mothership.x, mothership.y, mothership.x + MOTHERSHIP_W - 1, mothership.y + MOTHERSHIP_H - 1) ) {
    player_shot.is_used = false;
    player.score += mothership.points;
    mothership.is_active = false;
    current_explosion.x = mothership.x;
    current_explosion.y = mothership.y;
    current_explosion.type = UFO_EXPLOSION;
    alienWasKilled = true;
  }

  // Alien_shot collition
  if ( collide(player_shot.x, player_shot.y, player_shot.x + SHOT_W - 1, player_shot.y + SHOT_H - 1, alien_shot.x, alien_shot.y, alien_shot.x + SHOT_W - 1, alien_shot.y + SHOT_H - 1) ) {
    player_shot.is_used = false;
    alien_shot.is_used = false;
  }

  // Alien collition
  for ( i = 0; i < ALIENS_ROWS && player_shot.is_used; ++i ) {
    for ( j = 0; j < ALIENS_COLUMNS; ++j ) {
      if ( aliens[i][j].lives && collide(player_shot.x, player_shot.y, player_shot.x + SHOT_W - 1, player_shot.y + SHOT_H - 1, aliens[i][j].x, aliens[i][j].y, aliens[i][j].x + ALIENS_W - 1, aliens[i][j].y + ALIENS_H - 1) ) {
        player_shot.is_used = false;
        if(--aliens[i][j].lives == 0){
          current_explosion.x = aliens[i][j].x;
          current_explosion.y = aliens[i][j].y;
          current_explosion.type = ALIEN_EXPLOSION;
          player.score += aliens[i][j].points;
          alienWasKilled = true;
        }
        else{
          alienWasHit = true;
        }
        break;
      }
    }
  }

  // Shield collition
  unsigned k; // shield
  if ( player_shot.y + SHOT_H >= INITIAL_SHIELD_Y_COORDINATE ) {
    for ( k = 0; k < SHIELDS_CANT && player_shot.is_used; ++k ) {
      for ( j = 0; j < SHIELD_W && player_shot.is_used; ++j ) {
        for ( i = SHIELD_H - 1; i < SHIELD_H; --i ) {
          if ( shields[k][i][j].lives && collide(player_shot.x, player_shot.y, player_shot.x + SHOT_W - 1, player_shot.y + SHOT_H - 1, shields[k][i][j].x, shields[k][i][j].y, shields[k][i][j].x + SHIELD_BLOCK_W - 1, shields[k][i][j].y + SHIELD_BLOCK_H - 1) ) {
            player_shot.is_used = false;
            shields[k][i][j].lives--;
            shieldWasHit = true;
            break;
          }
        }
      }
    }
  }

  // Window limit
  if ( player_shot.y < 0 ) // Out of bounds
    player_shot.is_used = false;
}

static bool alien_shot_update() {
  if ( !alien_shot.is_used ) return false;

  alien_shot.y += SHOT_DY_ALIEN;

  // Player_shot collition: it is in player_shot_update

  // Shield collition
  unsigned i, j;
  unsigned k; // shield
  if ( alien_shot.y <= INITIAL_SHIELD_Y_COORDINATE + SHIELD_H * SHIELD_BLOCK_H ) {
    for ( k = 0; k < SHIELDS_CANT && alien_shot.is_used; ++k ) {
      for ( j = 0; j < SHIELD_W && alien_shot.is_used; ++j ) {
        for ( i = 0; i < SHIELD_H; ++i ) {
          if ( shields[k][i][j].lives && collide(alien_shot.x, alien_shot.y, alien_shot.x + SHOT_W - 1, alien_shot.y + SHOT_H - 1, shields[k][i][j].x, shields[k][i][j].y, shields[k][i][j].x + SHIELD_BLOCK_W - 1, shields[k][i][j].y + SHIELD_BLOCK_H - 1) ) {
            alien_shot.is_used = false;
            shields[k][i][j].lives--;
            shieldWasHit = true;
            break;
          }
        }
      }
    }
  }

  // Window limit
  if ( alien_shot.y + SHOT_H - 1 > WORLD_HEIGHT ) // Out of bounds
    alien_shot.is_used = false;

  // Player collition
  if ( !playerDied && collide(alien_shot.x, alien_shot.y, alien_shot.x + SHOT_W - 1, alien_shot.y + SHOT_H - 1, player.x, player.y, player.x + PLAYER_W - 1, player.y + PLAYER_H - 1) ) {
    alien_shot.is_used = false;
    player.lives--;
    playerDied = true;
    return true;
  }
  return false;
}
