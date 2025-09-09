#include "game.h"

#define MAX_LINE_LEN 256

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

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
                cells[floor][width][length].data.has_stairs = false;
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
    if( end_floor!=1 || end_floor!=2 ) 
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
    //Handle conditions, return if a condition is false, if get to the last statement add the stair
    if(start_floor >= end_floor)
    {
        return;
    }
    if( cells[start_floor][start_width_num][start_length_num].is_valid==false ||
        check_cell_types(&cells[start_floor][start_width_num][start_length_num], CELL_STARTING_AREA | CELL_BHAWANA))
    {
        return;
    }
    if(cells[end_floor][end_width_num][end_length_num].is_valid==false)
    {
        return;
    }
    //add to the cell
    add_cell_type(&cells[start_floor][start_width_num][start_length_num], CELL_STAIR_START);
    add_cell_type(&cells[end_floor][end_width_num][end_length_num], CELL_STAIR_END);

    //handle the data additions for each stair
}

int check_wall_init_conditions(Cell* cell)
{
    if(cell->is_valid == true && check_cell_types(cell, CELL_FLAG | CELL_STAIR_START | CELL_STAIR_END | CELL_POLE_ENTER | CELL_POLE_EXIT))
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
}

void load_poles(int* digits, char filename[], FILE* logfile, int num_lines)
{
    int start_floor = digits[0], end_floor = digits[1];
    int width_num = digits[2], length_num = digits[3];

    if( start_floor!=1 || start_floor!=0 ) 
    {
        print_range_error(START_FLOOR, filename, logfile, num_lines); 
        return;
    }
    if( end_floor!=1 || end_floor!=2 ) 
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

    add_cell_type(&cells[end_floor][width_num][length_num], CELL_POLE_ENTER);
    add_cell_type(&cells[start_floor][width_num][length_num], CELL_POLE_EXIT);

    if(start_floor==0 && end_floor==2 && cells[1][width_num][length_num].is_valid)
    {
        add_cell_type(&cells[1][width_num][length_num], CELL_POLE_EXIT);
    }
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
    if(cells[floor][width_num][length_num].is_valid && !check_cell_types(&cells[floor][width_num][length_num], CELL_STARTING_AREA))
    {
        cells[floor][width_num][length_num].celltypes = CELL_FLAG;
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

int read_data(char filename[])
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

void move_one_cell(Player* plyr, Direction dir)
{
    switch(dir){
            case NORTH:
                (plyr->player_pos->width)--; //move up
                break;
            case SOUTH:
                (plyr->player_pos->width)++; //move down
                break;
            case EAST:
                (plyr->player_pos->length)--; //move left
                break;
            case WEST:
                (plyr->player_pos->length)++; //move right
                break;
        }
}

typedef enum {
    CONTINUE_STEP,
    ABORT_MOVE,
    WIN_GAME
} HandlerResult;

typedef HandlerResult (*CellHandler)(Player* );

HandlerResult handle_flag(Player* plyr)
{
    return;
}

HandlerResult handle_pole_enter(Player* plyr)
{
    return;
}

typedef struct
{
    int flag;
    CellHandler handler;
} CellAction;

static const CellAction actions[] = {
    { CELL_FLAG,        handle_flag },
    { CELL_POLE_ENTER,  handle_pole_enter}
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
            move_one_cell(plyr, plyr->direction);

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
                            goto exit_loops;
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
        exit_loops:;
        plyr->throw_count++;
    }
    else
    {
        if(plyr->move_value == 6)
        {
            plyr->player_pos = plyr->first_cell;
        }
    }
}


//for compilation purpose only
int main()
{
    init_cells();
    init_players();

    read_data("poles.txt");
    read_data("stairs.txt");
    read_data("walls.txt");

    int player_index = 0;
    while(game_on)
    {
        moveplayer(&players[player_index]);

        player_index = (player_index + 1) % NUM_OF_PLAYERS;
    }

    //test code
    printf("No errors so far!\n");
}