#ifndef GAME_H
#define GAME_H

//Library inclusion 
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

//Enum declarations
typedef enum {NORMAL_CONSUMABLE, NORMAL_BONUS, WALL, STAIR_START, STAIR_END, POLE_ENTER, POLE_EXIT, STARTING_AREA, FLAG,
                BHAWANA} CellType;
typedef enum {NORMAL_BONUS, FOOD_POISONING, TRIGGER, HAPPY, DISORIENT} BhawanaCellType;
typedef enum {NORTH, SOUTH, EAST, WEST, UNCHANGED} Direction;
typedef enum {ACTIVE, BLOCKED, STARTING_AREA} PlayerState;
typedef enum {UPDOWN, UP, DOWN} StairDirection;

//Union declarations

typedef union 
{
    TeleportData teleport;
    MovementPointData movementpoint;
} SpecialCellData;

//Structure declarations

typedef struct{
    int dest_floor;
    int dest_width;
    int dest_length;
} TeleportData;

typedef struct{
    char factor;
    int value;
} MovementPointData;

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

#endif