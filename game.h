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

//Enum declarations
typedef enum {NONE, NORMAL_CONSUMABLE, NORMAL_BONUS, WALL, STAIR_START, STAIR_END, POLE_ENTER, POLE_EXIT, STARTING_AREA, FLAG,
                BHAWANA} CellType;
typedef enum {BONUS, FOOD_POISONING, TRIGGER, HAPPY, DISORIENT} BhawanaCellType;
typedef enum {NORTH, SOUTH, EAST, WEST, UNCHANGED} Direction;
typedef enum {ACTIVE, BLOCKED, INACTIVE} PlayerState;
typedef enum {UPDOWN, UP, DOWN} StairDirection;

typedef enum {INVALID_NUM_DIGITS, NO_DIGITS, FILE_NOT_OPEN, EMPTY_FILE, INVALID_FORMAT_LINE} FileErrors;
typedef enum {START_FLOOR, END_FLOOR, START_WIDTH, END_WIDTH, START_LENGTH, END_LENGTH, WIDTH, LENGTH} RangeError;
typedef enum {START_FLOOR_LARGE} ConditionErrors;

//Structures and Unions declarations
typedef struct{
    int dest_floor;
    int dest_width;
    int dest_length;
} TeleportData;

typedef struct{
    int num_of_stairs;
    TeleportData stairs[2];
} StairData;
typedef struct{
    char factor;
    int value;
} MovementPointData;
typedef union 
{
    TeleportData pole;
    StairData stair;
    MovementPointData movementpoint;
} SpecialCellData;
typedef struct{
    int floor;
    int width;
    int length;
    bool is_valid;
    CellType celltype;
    SpecialCellData data;
}Cell;

typedef struct{
    char name;
    Direction direction;
    Cell* starting_cell;
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