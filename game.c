#include "game.h"

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

int game_round = 0;

void init_cells()
{
    for(int floor = 0; floor < NUM_OF_FLOORS; floor++)
    {
        for(int width = 0; width < MAX_WIDTH; width++)
        {
            for(int length = 0; length < MAX_LENGTH; length++)
            {
                cells[floor][width][length].floor = floor;
                cells[floor][width][length].width = width;
                cells[floor][width][length].length = length;
                cells[floor][width][length].is_valid = true;
            }
        }
    }
    //Initializing the stariting area on floor 0
    for(int width = 6; width < 10; width++)
    {
        for(int length = 8; length < 17; length++)
        {
            cells[0][width][length].celltype = STARTING_AREA;
            //no SpecialCellData for celltype STARTING_AREA
        }
    }
    //Initializing the invalid area in floor 1
    for(int width = 0; width < 6; width++)
    {
        for(int length = 8; length < 17; length++)
        {
            cells[1][width][length].is_valid = false;
        }
    }
    //Initializing the first invalid area in floor 2
    for(int width = 0; width < MAX_WIDTH; width++)
    {
        for(int length = 0; length < 8; length++)
        {
            cells[1][width][length].is_valid = false;
        }
    }
    //Initializing the second invalid area in floor 2
    for(int width = 0; width < MAX_WIDTH; width++)
    {
        for(int length = 17; length < MAX_LENGTH; length++)
        {
            cells[1][width][length].is_valid = false;
        }
    }
}

void init_players()
{
    char name_array[NUM_OF_PLAYERS] = {'A', 'B', 'C'};
    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        players[i].name = name_array[i];
        players[i].player_state = INACTIVE;
        players[i].throw_count = 0;
        players[i].move_value = 0;
    }
    players[0].direction = NORTH;
    players[1].direction = WEST;
    players[2].direction = EAST;

    players[0].starting_cell = players[0].player_pos = &cells[0][6][12];
    players[1].starting_cell = players[1].player_pos = &cells[0][9][8];
    players[2].starting_cell = players[2].player_pos = &cells[0][9][16];
}

//for compilation purpose only
int main()
{}