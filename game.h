#ifndef GAME_H
#define GAME_H

//Library inclusion 
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

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
    BHAWANA_DISORIENT       = 1 << 14
} CellType;

typedef enum 
{   
    NORTH, 
    SOUTH, 
    EAST, 
    WEST
} Direction;

typedef enum {ACTIVE, INACTIVE} PlayerState;
typedef enum {UPDOWN, UP, DOWN} StairDirection;

typedef enum 
{
    INVALID_NUM_DIGITS, 
    NO_DIGITS, 
    FILE_NOT_OPEN, 
    EMPTY_FILE, 
    INVALID_FORMAT_LINE
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
typedef struct
{
    int dest_floor;
    int dest_width;
    int dest_length;
} TeleportData;

typedef struct
{
    int num_of_stairs;
    TeleportData stairs[2];    // 1 or 2
} StairData;
typedef struct
{
    char factor;    // '+', '-', '*'
    int value;
} MovementPointData;
/* -------------------- Simple cell data / Complex cell data -------------------- */
typedef union 
{
    TeleportData pole;
    StairData stair;
    MovementPointData movementpoint;
} SimpleCellData;
typedef struct 
{
    bool has_pole;
    TeleportData pole;

    bool has_stair;
    StairData stair;

    bool has_movementpoint;
    MovementPointData movementpoint;
} ComplexCellData;

/* -------------------- Cell structure -------------------- */
typedef struct
{
    int floor;
    int width;
    int length;
    bool is_valid;
    int celltypes;
    bool is_complex;
    union 
    {
        SimpleCellData simple;   // if only one special type
        ComplexCellData* complex; // pointer if multiple types
    } data;
}Cell;

/* -------------------- Player structure -------------------- */

typedef struct
{
    char name;
    Direction direction;
    Cell* first_cell;   //cell in maze 
    Cell* starting_cell; //cell in starting area
    PlayerState player_state;
    int throw_count;
    int move_value;
    Cell* player_pos;
} Player;

//Variable declarations
extern int game_round;
extern Player players[NUM_OF_PLAYERS];
extern Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

#endif