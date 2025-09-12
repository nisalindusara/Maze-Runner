#ifndef GAME_H
#define GAME_H

//Library inclusion 
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

//MACROS
#define NUM_OF_PLAYERS 3
#define NUM_OF_FLOORS 3
#define MAX_WIDTH 10
#define MAX_LENGTH 25

/* -------------------- Enum declarations -------------------- */

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
    CELL_BHAWANA       = 1 << 9,

    // Bhawana subtypes start here
    BHAWANA_BONUS           = 1 << 10,
    BHAWANA_FOOD_POISONING  = 1 << 11,
    BHAWANA_TRIGGER         = 1 << 12,
    BHAWANA_HAPPY           = 1 << 13,
    BHAWANA_DISORIENT       = 1 << 14,
    BHAWANA_ENTRANCE        = 1 << 15
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

typedef enum {START_FLOOR_LARGE} ConditionErrors;

/* -------------------- Teleports / Stairs / Movement -------------------- */

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

/* -------------------- cell data -------------------- */
typedef struct 
{
    bool access_pole;
    PoleData pole_data;

    short stair_count;
    Stair* stairs[2];

    bool has_movementpoint;
    MovementPointData mpdata;
} CellData;

/* -------------------- Cell structure -------------------- */
typedef struct Cell
{
    int floor;
    int width;
    int length;
    bool is_valid;
    int celltypes;
    CellData data;
}Cell;

/* -------------------- Player structure -------------------- */

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
} Player;

//Variable declarations
extern int game_round;
extern Player players[NUM_OF_PLAYERS];
extern Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

/*  Event Handling  */

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
    PLAYER_WIN
} MoveResult;

typedef struct
{
    MoveResult event;
    EventData data;
} MoveEvent;

#endif