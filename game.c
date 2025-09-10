#include "game.h"

#define GAME_SETTING 5
#define MAX_LINE_LEN 256

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

Cell** stairs_array;
int stair_count = 0;
int stair_array_capacity = 6;

Cell* cell_flag;

int game_round = 0;
bool game_on = true;

void mark_cells(int floor, int start_w, int end_w, int start_l, int end_l, bool is_valid, CellType type) {
    for(int w = start_w; w <= end_w; w++)
        for(int l = start_l; l <= end_l; l++) {
            cells[floor][w][l].is_valid = is_valid;
            if(cells[floor][w][l].is_valid) cells[floor][w][l].celltypes = type;
        }
}

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
                cells[floor][width][length].data.has_pole = false;
                cells[floor][width][length].data.stair_count = 0;
                cells[floor][width][length].data.has_movementpoint = false;
            }
        }
    }
    mark_cells(0, 6, 9, 8, 16, true, CELL_STARTING_AREA);
    mark_cells(1, 0, 5, 8, 16, false, CELL_NONE);
    mark_cells(2, 6, 9, 0, 7, false, CELL_NONE);
    mark_cells(2, 6, 9, 17, 24, false, CELL_NONE);
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

    players[0].first_cell = players[0].player_pos = &cells[0][5][12];
    players[1].first_cell = players[1].player_pos = &cells[0][9][7];
    players[2].first_cell = players[2].player_pos = &cells[0][9][17];
}

/*
    read input data, validate each line while updating the relevant cells
*/

void print_file_error(FileErrors error, char filename[], FILE *logfile)
{
    switch(error)
    {
        case FILE_NOT_OPEN:
            fprintf(logfile, "[ERROR] Cannot open (file: %s)\n", filename);
            break;
        case EMPTY_FILE:
            fprintf(logfile, "[ERROR] File is empty (file: %s). No data to process.\n", filename);
            break;
        default:
            fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");
    }
}

void print_file_error_line(FileErrors error, char filename[], FILE *logfile, int line_number)
{
    switch (error)
    {
        case INVALID_NUM_DIGITS:
            fprintf(logfile, "[WARN] Invalid number of digits (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case NO_DIGITS:
            fprintf(logfile, "[WARN] No digits found (file %s, line %d). Line skipped\n", filename, line_number);
            break;
        case INVALID_FORMAT_LINE:
            fprintf(logfile, "[WARN] Invalid format (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        default: 
            fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");
    }
}

void print_range_error(RangeError error, char filename[], FILE* logfile, int line_number)
{
    switch(error)
    {
        case FLOOR:
            fprintf(logfile, "[WARN] Range error on floor (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case START_FLOOR:
            fprintf(logfile, "[WARN] Range error on start floor (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case END_FLOOR:
            fprintf(logfile, "[WARN] Range error on end floor (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case START_WIDTH:
            fprintf(logfile, "[WARN] Range error on start width (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case START_LENGTH:
            fprintf(logfile, "[WARN] Range error on start length (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case END_WIDTH:
            fprintf(logfile, "[WARN] Range error on end width (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case END_LENGTH:
            fprintf(logfile, "[WARN] Range error on end length (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case WIDTH:
            fprintf(logfile, "[WARN] Range error on width (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        case LENGTH:
            fprintf(logfile, "[WARN] Range error on length (file %s, line %d). Line skipped.\n", filename, line_number);
            break;
        default: 
            fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");
    }
}

bool check_cell_types(Cell *c, int types) 
{
    return (c->celltypes & types) != 0;
}
void add_cell_type(Cell *c, int type)
{
    c->celltypes |= type;
}

void load_flag(int* digits, char filename[], FILE* logfile, int num_lines)
{
    int floor = digits[0];
    int width_num = digits[1];
    int length_num = digits[2];
    if( floor<0 || floor>2 ) 
    {
        print_range_error(FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( width_num<0 || width_num>9 ) 
    {
        print_range_error(WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( length_num<0 || length_num>24 ) 
    {
        print_range_error(LENGTH, filename, logfile, num_lines); 
        return;
    }
    //Init flag
    if(cells[floor][width_num][length_num].is_valid && !check_cell_types(&cells[floor][width_num][length_num], CELL_STARTING_AREA) )
    {
        cells[floor][width_num][length_num].celltypes = CELL_FLAG;
        cell_flag = &cells[floor][width_num][length_num];
    }
}


void load_poles(int* digits, char filename[], FILE* logfile, int num_lines)
{
    int start_floor = digits[0], end_floor = digits[1];
    int width_num = digits[2], length_num = digits[3];

    if( start_floor!=1 && start_floor!=0 ) 
    {
        print_range_error(START_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( end_floor!=1 && end_floor!=2 ) 
    {
        print_range_error(END_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( width_num<0 || width_num>=MAX_WIDTH ) 
    {
        print_range_error(WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( length_num<0 || length_num>=MAX_LENGTH ) 
    {
        print_range_error(LENGTH, filename, logfile, num_lines); 
        return;
    }
    //Handle conditions, return _numif a condition is false, if get to the last statement add the pole
    if(start_floor>=end_floor)
    {
        return;
    }
    if(cells[end_floor][width_num][length_num].is_valid==false)
    {
        return;
    }
    if(cells[start_floor][width_num][length_num].is_valid==false)
    {
        return;
    }

    if(start_floor==0 && end_floor==2 && cells[1][width_num][length_num].is_valid == false)
    {
        return;
    }
    else
    {
        add_cell_type(&cells[1][width_num][length_num], CELL_POLE_ENTER);
        cells[1][width_num][length_num].data.has_pole = true;
        cells[1][width_num][length_num].data.pole_data.dest_cell = &cells[end_floor][width_num][length_num];
    }

    add_cell_type(&cells[end_floor][width_num][length_num], CELL_POLE_ENTER);
    cells[end_floor][width_num][length_num].data.has_pole = true;
    cells[end_floor][width_num][length_num].data.pole_data.dest_cell = &cells[end_floor][width_num][length_num];

    add_cell_type(&cells[start_floor][width_num][length_num], CELL_POLE_EXIT);

}

void load_stairs(int* digits, char filename[], FILE* logfile, int num_lines, int* stair_count, int* stair_array_capacity)
{
    int start_floor = digits[0], start_width_num = digits[1], start_length_num = digits[2];
    int end_floor = digits[3], end_width_num = digits[4], end_length_num = digits[5];

    if( start_floor<0 || start_floor>1 ) 
    {
        print_range_error(START_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( start_width_num<0 || start_width_num>=MAX_WIDTH ) 
    {
        print_range_error(START_WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( start_length_num<0 || start_length_num>=MAX_LENGTH ) 
    {
        print_range_error(START_LENGTH, filename, logfile, num_lines); 
        return;
    }
    if( end_floor!=1 && end_floor!=2 ) 
    {
        print_range_error(END_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( end_width_num<0 || end_width_num>=MAX_WIDTH ) 
    {
        print_range_error(END_WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( end_length_num<0 || end_length_num>=MAX_LENGTH ) 
    {
        print_range_error(END_LENGTH, filename, logfile, num_lines); 
        return;
    }

    Cell* stair_start_cell = &cells[start_floor][start_width_num][start_length_num];
    Cell* stair_end_cell = &cells[end_floor][end_width_num][end_length_num];
    //Handle conditions, return if a condition is false, if get to the last statement add the stair
    if(start_floor >= end_floor)
    {
        return;
    }
    if(stair_start_cell->is_valid==false)
    {
        return;
    }
    if(stair_end_cell->is_valid==false)
    {
        return;
    }

    //INIT stairs
    if(stair_start_cell->data.stair_count != 2 && stair_end_cell->data.stair_count != 2)
    {
        int start_idx = (stair_start_cell->data.stair_count)++;
        stair_start_cell->data.staircelldata.stairs[start_idx].direction = UPDOWN;
        stair_start_cell->data.staircelldata.stairs[start_idx].dest_cell = stair_end_cell;
        add_cell_type(stair_start_cell, CELL_STAIR_START);

        //add the stair start cell to stair array
        if (*stair_count >= *stair_array_capacity) 
        {
            (*stair_array_capacity) *= 2;  // geometric growth
            stairs_array = realloc(stairs_array, (*stair_array_capacity) * sizeof(Cell*));
        }
        stairs_array[(*stair_count)++] = stair_start_cell;

        int end_idx = (stair_end_cell->data.stair_count)++;
        stair_end_cell->data.staircelldata.stairs[end_idx].direction = UPDOWN;
        stair_end_cell->data.staircelldata.stairs[end_idx].dest_cell = stair_start_cell;
        add_cell_type(stair_end_cell, CELL_STAIR_END);
    }
    else
    {
        return;
    }

    
}

int check_wall_init_conditions(Cell* cell)
{
    if(cell->is_valid == true && 
        check_cell_types(cell, CELL_FLAG | CELL_STAIR_START | CELL_STAIR_END | CELL_POLE_ENTER | CELL_POLE_EXIT | CELL_BHAWANA))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void load_walls(int* digits, char filename[], FILE* logfile, int num_lines)
{
    int floor = digits[0], start_width_num = digits[1], start_length_num = digits[2];
    int end_width_num = digits[3], end_length_num = digits[4];

    if( floor<0 || floor>2 ) 
    {
        print_range_error(START_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( start_width_num<0 || start_width_num>=MAX_WIDTH ) 
    {
        print_range_error(START_WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( start_length_num<0 || start_length_num>=MAX_LENGTH ) 
    {
        print_range_error(START_LENGTH, filename, logfile, num_lines); 
        return;
    }
    if( end_width_num<0 || end_width_num>=MAX_WIDTH ) 
    {
        print_range_error(END_WIDTH, filename, logfile, num_lines); 
        return;
    }
    if( end_length_num<0 || end_length_num>=MAX_LENGTH ) 
    {
        print_range_error(END_LENGTH, filename, logfile, num_lines); 
        return;
    }
    //Handle conditions
    if(start_width_num == end_width_num)
    {
        if(start_length_num == end_length_num)  //single cell wall
        {
            if(check_wall_init_conditions(&cells[floor][start_width_num][start_length_num]))
            {
                add_cell_type(&cells[floor][start_width_num][start_length_num], CELL_WALL);
            }
            else return;
        }
        else    //horizontal wall
        {
            if(start_length_num > end_length_num)   
            {
                for(int length = start_length_num; length <= end_length_num; length--)   
                {
                    if(check_wall_init_conditions(&cells[floor][start_width_num][length]))
                    {
                        add_cell_type(&cells[floor][start_width_num][length], CELL_WALL);
                    }
                    else return;
                }
            }
            else
            {
                for(int length = start_length_num; length <= end_length_num; length++)  
                {
                    if(check_wall_init_conditions(&cells[floor][start_width_num][length]))
                    {
                        add_cell_type(&cells[floor][start_width_num][length], CELL_WALL);
                    }
                    else return;
                }
            }
        }
    }
    else
    {

        if(start_length_num == end_length_num)  //vertical wall
        {
            if(start_width_num > end_width_num)
            {
                for(int width = start_width_num; width <= end_width_num; width--)   
                {
                    if(check_wall_init_conditions(&cells[floor][width][start_length_num]))
                    {
                        add_cell_type(&cells[floor][width][start_length_num], CELL_WALL);
                    }
                    else return;
                }
            }
            else
            {
                for(int width = start_width_num; width <= end_width_num; width++)   
                {
                    if(check_wall_init_conditions(&cells[floor][width][start_length_num]))
                    {
                        add_cell_type(&cells[floor][width][start_length_num], CELL_WALL);
                    }
                    else return;
                }
            }
        }
        else    //diagonal wall [NOT ALLOWED]
        {   
            return;
        }    
    }
}

void check_digits(int* digits, int count, char filename[], FILE* logfile, int num_lines)
{
    switch(count)
    {
        case 6:
            load_stairs(digits, filename, logfile, num_lines, &stair_count, &stair_array_capacity);
            break;
        case 5:
            load_walls(digits, filename, logfile, num_lines);
            break;
        case 4:
            load_poles(digits, filename, logfile, num_lines);
            break;
        case 3:
            load_flag(digits, filename, logfile, num_lines);
            break;
        default:
            print_file_error_line(INVALID_NUM_DIGITS, filename, logfile, num_lines);
    }
}

int parse_and_process_line(const char* line, char filename[], FILE* logfile, int num_lines)
{
    const char* p =  line;
    
    while(isspace(*p)) p++;     
    
    if(*p != '[') return 0;     
    
    p++;
    
    int count = 0;
    int digits_array[7];
    
    while(*p && *p != ']')
    {
        while(isspace(*p)) p++;
        
        if(!isdigit(*p)) return 0;
        
        int num = 0;
        while (isdigit(*p)) 
        {
            num = num * 10 + (*p - '0');
            p++;
        }
        
        if(count >= 7) return 0;
        digits_array[count++] = num;
        
        while(isspace(*p)) p++;
        
        if(*p == ',') p++;
        else if(*p != ']') return 0;
    }

    if(*p != ']') return 0;
    
    p++;
    
    while(isspace(*p)) p++;
    
    if(*p != '\0') return 0;

    if(count > 0)
    {

        check_digits(digits_array, count, filename, logfile, num_lines);
    }
    else
    {
        print_file_error_line(NO_DIGITS, filename, logfile, num_lines);
    }
    
    return 1;
}

int validate_file(char filename[])
{
    FILE* logf = fopen("log.txt", "a");
    if(logf == NULL) perror("log.txt crashed!");
    
    FILE* dataf = fopen(filename, "r");
    if(dataf == NULL)  
    {
        print_file_error(FILE_NOT_OPEN, filename, logf);
        // exit(1);
        return 0;
    }

    int ch = fgetc(dataf);
    if (ch == EOF) 
    {
        print_file_error(EMPTY_FILE, filename, logf);
        // exit(1);
        return 0;
    }

    rewind(dataf);  

    char buffer[1024]; 
    int line_num = 1;

    while(fgets(buffer, sizeof(buffer), dataf))     
    {
        buffer[strcspn(buffer, "\n")] = '\0';       
        if(!parse_and_process_line(buffer, filename, logf, line_num))
        {
            print_file_error_line(INVALID_FORMAT_LINE, filename, logf, line_num);
        }
        line_num++;
    }
    fclose(logf);
    fclose(dataf);
    return 1;
}

void read_data()
{
    char* files[] = {"flag.txt", "poles.txt", "stairs.txt", "walls.txt"};

    for(int index = 0; index < 4; index++)
    {
        validate_file(files[index]);
    }
}

/*
    initiate the move function
*/
int movement_dice()
{
    return ((rand() % 6) + 1);
}

Direction direction_dice(Player* plyr)
{
    int dir =  (rand() % 6) + 1;
    switch(dir){
        case 1:
            return  plyr->direction;
        case 2:
            return NORTH;
        case 3:
            return EAST;
        case 4:
            return SOUTH;
        case 5:
            return WEST;
        case 6:
            return plyr->direction;
    }
}

typedef enum
{
    NO_BOUNDS,
    NORTH_BOUND,
    SOUTH_BOUND,
    EAST_BOUND,
    WEST_BOUND
} Bounds;

Bounds move_one_cell(Player* plyr, Direction dir)
{
    int floor = plyr->player_pos->floor;
    int width = plyr->player_pos->width;
    int length = plyr->player_pos->length;

    switch(dir){
        case NORTH:
            width--; //move up
            break;
        case SOUTH:
            width++; //move down
            break;
        case EAST:
            length--; //move left
            break;
        case WEST:
            length++; //move right
            break;
    }

    if(width < 0)
    {
        return NORTH_BOUND;     // [LATER] add custom output
    }
    else if(width == MAX_WIDTH)
    {
        return SOUTH_BOUND;
    }
    else if(length < 0)
    {
        return EAST_BOUND;
    }
    else if(length == MAX_LENGTH)
    {
        return WEST_BOUND;
    }

    plyr->player_pos = &cells[floor][width][length];
    return NO_BOUNDS;
}

typedef enum {
    CONTINUE_STEP,
    ABORT_MOVE,
    WIN_GAME
} HandlerResult;

typedef HandlerResult (*CellHandler)(Player* );

HandlerResult handle_normal_consumable(Player* plyr)
{
    return CONTINUE_STEP;
}

HandlerResult handle_normal_bonus(Player* plyr)
{
    return CONTINUE_STEP;
}

HandlerResult handle_wall(Player* plyr)
{
    return ABORT_MOVE;
}

int score_stair(Cell* stair, Cell* flag) {
    int floor_diff = abs(stair->floor - flag->floor);        // how far in floors
    int manhattan = abs(stair->width - flag->width) + abs(stair->length - flag->length);  // width/length distance

    int random_noise = rand() % 3;  // small random to break ties
    int score = 2 * floor_diff + 1 * manhattan + random_noise; 
    return score;
}

Cell* pick_stair_heuristic(Cell* dest1, Cell* dest2, Cell* flag) {
    // Check if either leads directly to flag
    if(dest1->floor == flag->floor && dest1->width == flag->width && dest1->length == flag->length)
        return dest1;
    if(dest2->floor == flag->floor && dest2->width == flag->width && dest2->length == flag->length)
        return dest2;

    int score1 = score_stair(dest1, flag);
    int score2 = score_stair(dest2, flag);

    if(score1 < score2) return dest1;
    if(score2 < score1) return dest2;
    
    // tie → random
    return (rand() % 2) ? dest1 : dest2;
}

HandlerResult handle_stair(Player* plyr, StairDirection wrong_direction)
{
    //handle 2 stairs
    if(plyr->player_pos->data.stair_count == 2)
    {
        //both stairs have the correct direction
        if( (plyr->player_pos->data.staircelldata.stairs[0].direction != wrong_direction) &&
            (plyr->player_pos->data.staircelldata.stairs[1].direction != wrong_direction) )
        {
            plyr->player_pos = pick_stair_heuristic(plyr->player_pos->data.staircelldata.stairs[0].dest_cell, 
                plyr->player_pos->data.staircelldata.stairs[1].dest_cell, cell_flag);
        }
        else
        {
            //only first stair has the correct direction
            if( (plyr->player_pos->data.staircelldata.stairs[0].direction != wrong_direction) &&
                (plyr->player_pos->data.staircelldata.stairs[1].direction = wrong_direction) )
            {
                plyr->player_pos = plyr->player_pos->data.staircelldata.stairs[0].dest_cell;
            }
            else
            {   
                //only second stair has correct direction 
                if( (plyr->player_pos->data.staircelldata.stairs[0].direction = wrong_direction) &&
                    (plyr->player_pos->data.staircelldata.stairs[1].direction != wrong_direction) )
                {
                   plyr->player_pos = plyr->player_pos->data.staircelldata.stairs[1].dest_cell;
                }
                //no stairs have the correct direction
                else
                {
                    return ABORT_MOVE;
                }
            }
        }
    }
    else
    {
        if(plyr->player_pos->data.staircelldata.stairs[0].direction != wrong_direction)
        {
            plyr->player_pos = plyr->player_pos->data.staircelldata.stairs[0].dest_cell;
        } 
    }
    return CONTINUE_STEP;
}

HandlerResult handle_stair_start(Player* plyr)
{
    return handle_stair(plyr, DOWN);
}

HandlerResult handle_stair_end(Player* plyr)
{
    return handle_stair(plyr, UP);
}

HandlerResult handle_pole_enter(Player* plyr)
{
    plyr->player_pos = plyr->player_pos->data.pole_data.dest_cell;
    return CONTINUE_STEP;
}

HandlerResult handle_pole_exit(Player* plyr)
{
    return CONTINUE_STEP;
}

HandlerResult handle_starting_area(Player* plyr)
{
    //can be called because a player went staright to the starting area or went through a pole or a stair. need to handle both
    return ABORT_MOVE;
}

HandlerResult handle_flag(Player* plyr)
{
    return WIN_GAME;
}

HandlerResult handle_bhawana(Player* plyr)
{
    //[LATER]
    return CONTINUE_STEP;
}

typedef struct
{
    int flag;
    CellHandler handler;
} CellAction;

static const CellAction actions[] = {                           //order matters [CHECK]
    { CELL_FLAG,                handle_flag             },
    { CELL_NORMAL_CONSUMABLE,   handle_normal_consumable},
    { CELL_NORMAL_BONUS,        handle_normal_bonus     },
    { CELL_POLE_ENTER,          handle_pole_enter       },
    { CELL_STAIR_START,         handle_stair_start      },
    { CELL_STAIR_END,           handle_stair_end        },
    { CELL_WALL,                handle_wall             },
    { CELL_POLE_EXIT,           handle_pole_exit        },
    { CELL_STARTING_AREA,       handle_starting_area    },
    { CELL_BHAWANA,             handle_bhawana          },
};

void moveplayer(Player* plyr)
{
    plyr->move_value = movement_dice();

    if(plyr->player_state == ACTIVE)
    {
        if(plyr->throw_count % 4 ==0)
        {
            plyr->direction = direction_dice(plyr);
        }

        Cell* position_before_move = plyr->player_pos;

        for(int step = 0; step < plyr->move_value; step++)
        {
            if(move_one_cell(plyr, plyr->direction) != NO_BOUNDS) break;    //terminates step

            if(plyr->player_pos->is_valid)
            {
                for(int j = 0; j < sizeof(actions)/sizeof(actions[0]); j++)
                {
                    if(plyr->player_pos->celltypes & actions[j].flag)
                    {
                        if(actions[j].handler(plyr) == CONTINUE_STEP)
                        {
                            break;
                        }
                        else if(actions[j].handler(plyr) == ABORT_MOVE)
                        {
                            plyr->player_pos = position_before_move;
                            goto exit_moving;
                        }
                        else
                        {
                            game_on = false;
                            printf("print output message for player win.\n");
                            return;
                        }
                        break;  //if anything goes wrong above
                    }   
                }
            }
            else
            {
                plyr->player_pos = position_before_move;
                break;
            }
        }
        exit_moving:;
        plyr->throw_count++;
    }
    else
    {
        if(plyr->move_value == 6)
        {
            plyr->player_pos = plyr->first_cell;
            plyr->player_state = ACTIVE;
        }
    }
}

void change_stair_direction()
{
    for(int index = 0; index < stair_count; index++)
    {
        // stairs_array[index]->data.staircelldata.
    }
}

//for compilation purpose only
int main()
{
    init_cells();
    init_players();

    stairs_array = malloc(stair_array_capacity * sizeof(Cell*));

    read_data();

    int player_index = 0;
    int max_turns = 12;
    while(game_on && (max_turns--)>0)
    {
        moveplayer(&players[player_index]);

        player_index = (player_index + 1) % NUM_OF_PLAYERS;

        if(player_index == 0) game_round++;
        if(game_round % GAME_SETTING == 0) change_stair_direction();
    }

    free(stairs_array);
    //test code
    printf("No errors so far!\n");
}