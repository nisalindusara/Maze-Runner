#ifndef GAME_H
#define GAME_H

/* -------------------- PUBLIC API -------------------- */
void play_game(void);

/* -------------------- INTERNAL TYPES & FUNCTIONS -------------------- */
#ifdef GAME_INTERNAL   // only visible to game.c
 
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#define NUM_OF_PLAYERS 3
#define NUM_OF_FLOORS 3
#define MAX_WIDTH 10
#define MAX_LENGTH 25

#define GAME_SETTING 5
#define MAX_LINE_LEN 256
#define MAX_EVENTS_PER_EACH_MOVE 20

typedef enum 
{
    INVALID_NUM_DIGITS, 
    NO_DIGITS, 
    FILE_NOT_OPEN, 
    EMPTY_FILE, 
    INVALID_FORMAT_LINE,
} FileErrors;

typedef enum 
{
    FLOOR,
    START_FLOOR, 
    END_FLOOR, 
    START_WIDTH, 
    END_WIDTH, 
    START_LENGTH, 
    END_LENGTH, 
    WIDTH, 
    LENGTH
} RangeError;

typedef enum
{
    INVALID_FLAG_POSITION,
    INVALID_POLE_DEFINITION,
    POLE_ENTRANCE_INVALID,
    POLE_EXIT_INVALID,
    POLE_INTERCEPTION_INVALID,
    INVALID_STAIR_DEFINITION,
    STAIR_START_INVALID,
    STAIR_END_INVALID,
    STAIR_START_FULL,
    STAIR_END_FULL,
    INVALID_WALL_POSITION,
    SKIP_WALL_CELL,
    DIAGONAL_WALL
} LogicError;

typedef enum 
{
    CELL_NONE          = 0,
    CELL_NORMAL_CONSUMABLE = 1 << 0,
    CELL_NORMAL_BONUS  = 1 << 1,
    CELL_WALL          = 1 << 2,
    CELL_STAIR_START   = 1 << 3,
    CELL_STAIR_END     = 1 << 4,
    CELL_POLE_ENTER    = 1 << 5,
    CELL_POLE_EXIT     = 1 << 6,
    CELL_STARTING_AREA = 1 << 7,
    CELL_FLAG          = 1 << 8,
    CELL_BAWANA       = 1 << 9,
    // Bawana subtypes start here
    BAWANA_BONUS           = 1 << 10,
    BAWANA_FOOD_POISONING  = 1 << 11,
    BAWANA_TRIGGER         = 1 << 12,
    BAWANA_HAPPY           = 1 << 13,
    BAWANA_DISORIENT       = 1 << 14,
    // not a Bawana subtype 
    BAWANA_ENTRANCE        = 1 << 15
} CellType;

typedef enum 
{   
    NORTH, 
    SOUTH, 
    EAST, 
    WEST
} PlayerDirection;

typedef enum 
{
    ACTIVE, 
    INACTIVE, 
    FOOD_POISONED,
    DISORIENTED,
    TRIGGERED
} PlayerState;

typedef enum {UPDOWN, UP, DOWN} StairDirection;

typedef enum {START_FLOOR_LARGE} ConditionErrors;

typedef struct Cell Cell; //forward declaration of Cell
typedef struct
{
    char factor;    // '+', '-', '*'
    int value;
} MovementPointData;
typedef struct
{
    Cell* dest_cell;
} PoleData;

typedef struct
{
    Cell* start_cell;
    Cell* end_cell;
    StairDirection direction;
} Stair;

typedef struct 
{
    bool access_pole;
    PoleData pole_data;

    short stair_count;
    Stair* stairs[2];

    bool has_movementpoint;
    MovementPointData mpdata;
} CellData;
typedef struct Cell
{
    int floor;
    int width;
    int length;
    bool is_valid;
    int celltypes;
    CellData data;
}Cell;

typedef struct
{
    char name;
    PlayerDirection direction;
    Cell* first_cell;   //cell in maze 
    Cell* starting_cell; //cell in starting area
    PlayerState player_state;
    int throw_count;
    int move_value;
    Cell* player_pos;
    int mp_score;
    int effect_turns;
    PlayerDirection init_dir;
} Player;

extern int game_round;
extern Player players[NUM_OF_PLAYERS];
extern Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

typedef struct 
{
    Cell* trigger_cell;
    Cell* dest_cell;
}EventData;

typedef enum
{
    REMAIN_AT_SPAWN,
    EXIT_STARTING_AREA,
    MOVEMENT_BLOCKED,
    COMPLETE_MOVE,
    START_MOVE,
    START_MOVE_WITH_DIRECTION,
    TAKE_POLE,
    TAKE_STAIR,
    PLAYER_WIN,
    TO_BAWANA,
    TO_BAWANA_BONUS,
    TO_BAWANA_FP,
    TO_BAWANA_DISORIENT,
    TO_BAWANA_TRIGGER,
    TO_BAWANA_HAPPY
} MoveResult;

typedef struct
{
    MoveResult event;
    EventData data;
} MoveEvent;

typedef struct {
    char name;
    PlayerDirection direction;
    int start_floor, start_w, start_l;   // starting cell
    int first_floor, first_w, first_l;   // first cell after spawn
} PlayerInit;

typedef enum
{
  NO_BOUNDS,
  NORTH_BOUND,
  SOUTH_BOUND,
  EAST_BOUND,
  WEST_BOUND
} Bounds;

typedef enum {
    CONTINUE_STEP,
    ABORT_MOVE,
    WIN_GAME,
    PLACED_ON_BAWANA
} HandlerResult;

typedef HandlerResult (*CellHandler)(Player*, int*);

typedef struct
{
    int flag;
    CellHandler handler;
} CellAction;

typedef enum {
    CHECK_START_CELL,
    CHECK_END_CELL
} StairCheckType;

//dynamic array of all stairs
Stair** stairs_array;           

Cell* cell_flag;
Cell* bawana_entrance_cell_ptr;

char* time_buffer;

FILE* log_fp;

#endif  //GAME_INTERNAL

#endif  //GAME_H