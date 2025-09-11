#include "game.h"

#define GAME_SETTING 5
#define MAX_LINE_LEN 256

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

Stair** stairs_array;           //dynamic array of all stairs
int stair_count = 0;            //number of stairs
int stair_array_capacity = 6;   //initial capacity

Cell* cell_flag;
Cell* bhawana_entrance_cell_ptr;

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
                cells[floor][width][length].celltypes = CELL_NONE;
                cells[floor][width][length].data.access_pole = false;
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
        players[i].mp_score = 100;
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
            fprintf(logfile, "[ERROR] Cannot open (file: %s) %s\n", filename);
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
    int exit_floor = digits[0], enter_floor = digits[1];
    int width_num = digits[2], length_num = digits[3];

    if( exit_floor!=1 && exit_floor!=0 ) 
    {
        print_range_error(END_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( enter_floor!=1 && enter_floor!=2 ) 
    {
        print_range_error(START_FLOOR, filename, logfile, num_lines); 
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
    if(exit_floor>=enter_floor)
    {
        return;
    }
    if(cells[enter_floor][width_num][length_num].is_valid==false)
    {
        return;
    }
    if(cells[exit_floor][width_num][length_num].is_valid==false)
    {
        return;
    }

    if(exit_floor==0 && enter_floor==2 && cells[1][width_num][length_num].is_valid == false)
    {
        return;
    }
    else
    {
        add_cell_type(&cells[1][width_num][length_num], CELL_POLE_ENTER);
        cells[1][width_num][length_num].data.access_pole = true;
        cells[1][width_num][length_num].data.pole_data.dest_cell = &cells[exit_floor][width_num][length_num];
    }

    add_cell_type(&cells[enter_floor][width_num][length_num], CELL_POLE_ENTER);
    cells[enter_floor][width_num][length_num].data.access_pole = true;
    cells[enter_floor][width_num][length_num].data.pole_data.dest_cell = &cells[exit_floor][width_num][length_num];

    add_cell_type(&cells[exit_floor][width_num][length_num], CELL_POLE_EXIT);

}

void load_stairs(int* digits, char filename[], FILE* logfile, int num_lines)
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
        // Allocate the Stair object once
        Stair* new_stair = malloc(sizeof(Stair));
        new_stair->start_cell = stair_start_cell;
        new_stair->end_cell   = stair_end_cell;
        new_stair->direction  = UPDOWN;

        // Add to the stairs array
        if(stair_count >= stair_array_capacity) {
            stair_array_capacity *= 2;
            stairs_array = realloc(stairs_array, stair_array_capacity * sizeof(Stair*));
        }
        stairs_array[stair_count++] = new_stair;

        // Point start and end cells to the stair object
        stair_start_cell->data.stairs[stair_start_cell->data.stair_count++] = new_stair;
        stair_end_cell->data.stairs[stair_end_cell->data.stair_count++] = new_stair;

        add_cell_type(stair_start_cell, CELL_STAIR_START);
        add_cell_type(stair_end_cell, CELL_STAIR_END);

        if(stair_start_cell->floor == 0 && stair_end_cell->floor == 2)
        {
            //make the middle floor cell valid [CHECK] whether the intercepting cell should be a valid or not
        }
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
                for(int length = start_length_num; length >= end_length_num; length--)   
                {
                    if(check_wall_init_conditions(&cells[floor][start_width_num][length]))
                    {
                        add_cell_type(&cells[floor][start_width_num][length], CELL_WALL);
                    }
                    else break; //stop this wall, but keep loading the other walls
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
                    else break;
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
                for(int width = start_width_num; width >= end_width_num; width--)   
                {
                    if(check_wall_init_conditions(&cells[floor][width][start_length_num]))
                    {
                        add_cell_type(&cells[floor][width][start_length_num], CELL_WALL);
                    }
                    else break;
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
                    else break;
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
            load_stairs(digits, filename, logfile, num_lines);
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
    FILE* logf = fopen("log.txt", "w");
    if(logf == NULL) perror("log.txt crashed!");
    fclose(logf);

    logf = fopen("log.txt", "a");
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

typedef enum
{
    REMAIN_AT_SPAWN,
    EXIT_STARTING_AREA,
    MOVEMENT_BLOCKED,
    COMPLETE_MOVE
} MoveResult;

char* get_player_dir_string(Player* plyr)
{
    switch (plyr->direction)
    {
    case NORTH: return "North";
    case SOUTH: return "South";
    case EAST: return "East";
    case WEST: return "West";
    }
}

void print_output(MoveResult result, Player* p, int path_cost)
{
    switch(result)
    {
        case REMAIN_AT_SPAWN:
            printf("Player %c is at the starting area and rolls %d on the movement dice cannot enter the maze.\n", p->name, p->move_value);
            return;
        case EXIT_STARTING_AREA:
            printf("Player %c is at the starting area and rolls 6 on the movement dice and is placed on [%d, %d, %d] of the maze.\n", p->name, p->first_cell->floor, p->first_cell->width, p->first_cell->length);
            return;
        case MOVEMENT_BLOCKED:
            printf("Player %c rolls and %d on the movement dice and cannot move in the %s. Player remains at [%d, %d, %d]\n", p->name, p->move_value, get_player_dir_string(p), p->player_pos->floor, p->player_pos->width, p->player_pos->length);
            return;
        case COMPLETE_MOVE:
            printf("Player %c moved %d that cost %d movement points and is left with %d and is moving in the %s.\n", p->name, p->move_value, path_cost, p->mp_score, get_player_dir_string(p));
            return;
    }
}

// typedef enum
// {

// } BhawanaOutputSignal;

void print_output();

/*
    initiate the move function
*/
int movement_dice()
{
    return ((rand() % 6) + 1);
}

PlayerDirection direction_dice(Player* plyr)
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

Bounds move_one_cell(Player* plyr, PlayerDirection dir)
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
    else if(width >= MAX_WIDTH)
    {
        return SOUTH_BOUND;
    }
    else if(length < 0)
    {
        return EAST_BOUND;
    }
    else if(length >= MAX_LENGTH)
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

typedef HandlerResult (*CellHandler)(Player*, int*);

HandlerResult handle_normal_consumable(Player* plyr, int* cost)
{
    *cost -= plyr->player_pos->data.mpdata.value;
    return CONTINUE_STEP;
}

HandlerResult handle_normal_bonus(Player* plyr, int* cost)
{
    if(plyr->player_pos->data.mpdata.factor == '+')
    {
        *cost += plyr->player_pos->data.mpdata.value;
    }
    else
    {
        *cost *= plyr->player_pos->data.mpdata.value;
    }
    return CONTINUE_STEP;
}

HandlerResult handle_wall(Player* plyr, int* ignore_val)
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

HandlerResult handle_stair(Player* plyr, StairDirection wrong_dir, Cell* stair0_dest_cell, Cell* stair1_dest_cell)
{
    Cell* cell = plyr->player_pos;
    if(cell->data.stair_count == 1)
    {
        if(cell->data.stairs[0]->direction != wrong_dir)
        {
            plyr->player_pos = stair0_dest_cell;
            return CONTINUE_STEP;
        }
        else return ABORT_MOVE;
    }
    else
    {
        if((cell->data.stairs[0]->start_cell == cell)&&(cell->data.stairs[1]->start_cell == cell))
        {
            if((cell->data.stairs[0]->direction != wrong_dir) && (cell->data.stairs[1]->direction != wrong_dir))
            {
                plyr->player_pos = pick_stair_heuristic(stair0_dest_cell, stair1_dest_cell, cell_flag);
                return CONTINUE_STEP;
            }
            else if(cell->data.stairs[0]->direction != wrong_dir)
            {
                plyr->player_pos = stair0_dest_cell;
                return CONTINUE_STEP;
            }
            else if(cell->data.stairs[1]->direction != wrong_dir)
            {
                plyr->player_pos = stair1_dest_cell;
                return CONTINUE_STEP;
            }
            else return ABORT_MOVE;
        }
        else if((cell->data.stairs[0]->start_cell == cell)&&(cell->data.stairs[0]->direction != wrong_dir))
        {
            plyr->player_pos = stair0_dest_cell;
            return CONTINUE_STEP;
        }
        else if((cell->data.stairs[1]->start_cell == cell)&&(cell->data.stairs[1]->direction != wrong_dir))
        {
            plyr->player_pos = stair1_dest_cell;
            return CONTINUE_STEP;
        }
        else return ABORT_MOVE;
    }
}

HandlerResult handle_stair_start(Player* plyr, int* ignore_val)
{
    return handle_stair(plyr, DOWN, plyr->player_pos->data.stairs[0]->end_cell, plyr->player_pos->data.stairs[1]->end_cell);
}

HandlerResult handle_stair_end(Player* plyr, int* ignore_val)
{
    return handle_stair(plyr, UP, plyr->player_pos->data.stairs[0]->start_cell, plyr->player_pos->data.stairs[1]->start_cell);
}

HandlerResult handle_pole_enter(Player* plyr, int* ignore_val)
{
    plyr->player_pos = plyr->player_pos->data.pole_data.dest_cell;
    return CONTINUE_STEP;
}

HandlerResult handle_no_special_effect_cell(Player* plyr, int* ignore_val)
{
    return CONTINUE_STEP;
}

HandlerResult handle_starting_area(Player* plyr, int* ignore_val)
{
    //can be called because a player went staright to the starting area or went through a pole or a stair. need to handle both
    return ABORT_MOVE;
}

HandlerResult handle_flag(Player* plyr, int* ignore_val)
{
    return WIN_GAME;
}

HandlerResult handle_bhawana(Player* plyr, int* ignore_val)
{
    return ABORT_MOVE;
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
    { CELL_POLE_EXIT,           handle_no_special_effect_cell   },
    { CELL_STARTING_AREA,       handle_starting_area            },
    { CELL_BHAWANA,             handle_bhawana                  },
    { CELL_NONE,                handle_no_special_effect_cell   },
};

void capture_player(int current_player_index)
{
    for(int p_index = 0; p_index < NUM_OF_PLAYERS; p_index++)
    {
        if(p_index == current_player_index) continue;
        
        if(players[current_player_index].player_pos == players[p_index].player_pos)
        {
            players[p_index].player_pos = players[p_index].starting_cell;
        }
    }
}

void player_to_bhawana(Player* plyr)
{
    plyr->player_pos = &cells[0][(rand() % 3) + 7][(rand() % 4) + 21];
    if(check_cell_types(plyr->player_pos, BHAWANA_BONUS))
    {
        plyr->player_state = ACTIVE;
        plyr->mp_score = (rand() % 91) + 10;
        plyr->player_pos = bhawana_entrance_cell_ptr;
    }
    else if(check_cell_types(plyr->player_pos, BHAWANA_FOOD_POISONING))
    {
        plyr->player_state = FOOD_POISONED;
        plyr->effect_turns = 3;
    }   
    else if(check_cell_types(plyr->player_pos, BHAWANA_DISORIENT))
    {
        plyr->player_state = DISORIENTED;
        plyr->mp_score = 50;
        plyr->player_pos = bhawana_entrance_cell_ptr;
        plyr->effect_turns = 4;
        //player moves in a random direction for each time for the next 4 throws, override the direction dice
    }
    else if(check_cell_types(plyr->player_pos, BHAWANA_TRIGGER))
    {
        plyr->player_state = TRIGGERED;
        plyr->mp_score = 50;
        plyr->player_pos = bhawana_entrance_cell_ptr;
        //moves twices as fast(? for how many moves)
    }
    else if(check_cell_types(plyr->player_pos, BHAWANA_HAPPY))
    {
        plyr->player_state = ACTIVE;    
        plyr->mp_score = 200;
        plyr->player_pos = bhawana_entrance_cell_ptr;
    }
}

PlayerDirection generate_player_direction()
{
    switch(rand() % 4)
    {
        case 0: return NORTH;
        case 1: return SOUTH;
        case 2: return EAST;
        case 3: return WEST;
    }
}

void handle_cell_traversal(Player* plyr, PlayerDirection move_direction,int player_index)
{
    Cell* position_before_move = plyr->player_pos;

    int path_cost = 0;
    for(int step = 0; step < plyr->move_value; step++)
    {
        if(move_one_cell(plyr, move_direction) != NO_BOUNDS)
        {
            plyr->player_pos = position_before_move;
            print_output(MOVEMENT_BLOCKED, plyr, 0);
            goto partial_move;    //terminates step
        }

        if(plyr->player_pos->is_valid)
        {
            for(int j = 0; j < sizeof(actions)/sizeof(actions[0]); j++)
            {
                if(plyr->player_pos->celltypes & actions[j].flag)
                {
                    HandlerResult step_result = actions[j].handler(plyr, &path_cost);
                    if(step_result == CONTINUE_STEP)
                    {
                        break;
                    }
                    else if(step_result == ABORT_MOVE)
                    {
                        plyr->player_pos = position_before_move;
                        print_output(MOVEMENT_BLOCKED, plyr, 0);
                        goto partial_move;
                    }
                    else    //game won
                    {
                        game_on = false;
                        printf("Player %c has win.\n", plyr->name);
                        return;
                    }
                }   
            }
        }
        else
        {
            plyr->player_pos = position_before_move;
            break;
        }
    }
    capture_player(player_index);
    plyr->mp_score += path_cost;
    print_output(COMPLETE_MOVE, plyr, path_cost);
    if(plyr->mp_score <= 0)
    {
        // plyr->player_pos = &cells[0][(rand() % 3) + 7][(rand() % 4) + 21];
        // apply bhawana effects
        player_to_bhawana(plyr);
    }
    
    partial_move:;
    plyr->throw_count++;
}

void handle_player_state_movement(Player* plyr, int player_index)
{
    plyr->move_value = movement_dice();

    if(plyr->throw_count % 4 ==0)
    {
        plyr->direction = direction_dice(plyr);
    }

    if(plyr->player_state == ACTIVE)
    {
        handle_cell_traversal(plyr, plyr->direction, player_index);
    }
    else
    {
        if(plyr->player_state == INACTIVE)
        {
            if(plyr->move_value == 6)
            {
                plyr->player_pos = plyr->first_cell;
                plyr->player_state = ACTIVE;
                print_output(EXIT_STARTING_AREA, plyr, 0);
            }
            else
            {
                print_output(REMAIN_AT_SPAWN, plyr, 0);
            }
        } 
        else
        { 
            if(plyr->player_state == FOOD_POISONED)
            {
                printf("The Player %c is food poisoned.\n", plyr->name);
                plyr->throw_count++;
                if(plyr->effect_turns == 0)
                {
                    plyr->player_pos = &cells[0][(rand() % 3) + 7][(rand() % 4) + 21];
                    return;
                }
                plyr->effect_turns--;
            }
            else
            {
                if(plyr->player_state == DISORIENTED)
                {
                    printf("The Player %c is disoriented.\n", plyr->name);
                    plyr->throw_count++;
                    if(plyr->effect_turns == 0)
                    {
                        plyr->player_state = ACTIVE;
                        return;
                    }
                    handle_cell_traversal(plyr, generate_player_direction(), player_index);
                }
                else
                {
                    printf("The Player %c didn't matched any states.\n", plyr->name);
                }
            }
        }
    }
}

StairDirection get_random_stair_direction()
{
    int randint = rand() % 2;
    switch (randint)
    {
        case 0: return UP;
        case 1: return DOWN;
    }
}

void change_stair_direction()
{
    for(int index = 0; index < stair_count; index++)
    {
       stairs_array[index]->direction = get_random_stair_direction();
    }
}

int weighted_random_number(const double* probs)
{
    double randnum = (double)rand() / RAND_MAX;
    double cumulative = 0.0;

    for(int i = 0; i < 5; i++)
    {
        cumulative += probs[i];
        if(randnum < cumulative)
        {
            return i;
        }
    }
    //fallback in case of rounding errors
    return 0;
}

void set_normal_cells()
{
    //Define probabilities for 0 to 4
    double probabilities[5] = {0.35, 0.25, 0.25, 0.1, 0.05};

    for(int floor = 0; floor < NUM_OF_FLOORS; floor++)
    {
        for(int width = 0; width < MAX_WIDTH; width++)
        {
            for(int length = 0; length < MAX_LENGTH; length++)
            {
                if(cells[floor][width][length].celltypes == CELL_NONE)
                {
                    Cell* cell = &cells[floor][width][length];
                    switch(weighted_random_number(probabilities))
                    {
                        case 0:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '-';
                            cell->data.mpdata.value = (rand() % 4) + 1;
                            add_cell_type(cell, CELL_NORMAL_CONSUMABLE);
                            break;
                        case 2:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '+';
                            cell->data.mpdata.value = (rand() % 2) + 1;
                            add_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                        case 3:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '+';
                            cell->data.mpdata.value = (rand() % 3) + 3;
                            add_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                        case 4:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '*';
                            cell->data.mpdata.value = (rand() % 2) + 2;
                            add_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                    }
                }
            }
        }
    }
}

void create_randomised_array(int couples[12][2])
{
    int set1[] = {7, 8, 9};
    int set2[] = {21, 22, 23, 24};

    int k = 0;

    // Generate all 12 distinct couples
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            couples[k][0] = set1[i];
            couples[k][1] = set2[j];
            k++;
        }
    }

    // Shuffle using Fisher-Yates
    for (int i = 11; i > 0; i--) {
        int j = rand() % (i + 1);
        // swap couples[i] with couples[j]
        int temp0 = couples[i][0];
        int temp1 = couples[i][1];
        couples[i][0] = couples[j][0];
        couples[i][1] = couples[j][1];
        couples[j][0] = temp0;
        couples[j][1] = temp1;
    }
}

void init_bhawana()
{
    add_cell_type(&cells[0][9][19], BHAWANA_ENTRANCE);
    //bhawana wall
    for(int length = 20; length < MAX_LENGTH; length++)
    {
        add_cell_type(&cells[0][6][length], CELL_WALL);
    }
    for(int width = 7; width < MAX_WIDTH; width++)
    {
        add_cell_type(&cells[0][width][20], CELL_WALL);
    }
    //bhawana cells
    int couples[12][2];  // 12 couples
    create_randomised_array(couples);

    //first 4 cells is set to normal bonus
    for(int i = 0; i < 4; i++)
    {
        add_cell_type(&cells[0][couples[i][0]][couples[i][1]], BHAWANA_BONUS);
    }
    //next 2 cells will set to food poison
    add_cell_type(&cells[0][couples[4][0]][couples[4][1]], BHAWANA_FOOD_POISONING);
    add_cell_type(&cells[0][couples[5][0]][couples[5][1]], BHAWANA_FOOD_POISONING);
    //next 2 cells will set to disoriented
    add_cell_type(&cells[0][couples[6][0]][couples[6][1]], BHAWANA_DISORIENT);
    add_cell_type(&cells[0][couples[7][0]][couples[7][1]], BHAWANA_DISORIENT);
    //next 2 cells will set to triggered
    add_cell_type(&cells[0][couples[8][0]][couples[8][1]], BHAWANA_TRIGGER);
    add_cell_type(&cells[0][couples[9][0]][couples[9][1]], BHAWANA_TRIGGER);
    //next 2 cells will set to happy
    add_cell_type(&cells[0][couples[10][0]][couples[10][1]], BHAWANA_HAPPY);
    add_cell_type(&cells[0][couples[11][0]][couples[11][1]], BHAWANA_HAPPY);
}

//for compilation purpose only
int main()
{
    srand(time(NULL));
    init_cells();
    init_players();

    stairs_array = malloc(stair_array_capacity * sizeof(Stair*));
    
    init_bhawana();
    read_data();
    set_normal_cells();

    int player_index = 0;
    // int max_turns = 20;
    //  && (max_turns--)>0
    while(game_on)
    {
        if(player_index == 0)
        {
            game_round++;
            printf("GAME ROUND: %d\n", game_round);
        }
        if(game_round % GAME_SETTING == 0) change_stair_direction();

        handle_player_state_movement(&players[player_index], player_index);

        // get the player index for the next player 
        player_index = (player_index + 1) % NUM_OF_PLAYERS;

    }

    for(int i = 0; i < stair_count; i++) free(stairs_array[i]);
    free(stairs_array);

    //test code
    printf("No errors so far!\n");
}