#define GAME_INTERNAL
#include "game.h"

/* ======================== GLOBAL VAR DECLARATION ======================== */

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];
int stair_count = 0;            
int stair_array_capacity = 6;   
int game_round = 0;
bool game_on = true;
MoveEvent event_list[MAX_EVENTS_PER_EACH_MOVE];
int front = -1, rear = -1;
bool flag_loaded = false;

/* ======================== SHARED UTILITIES ======================== */

/*
    Shared functions that support both game initialization
    and input handling.
*/

bool check_cell_types(const Cell const *c, const int types) 
{
    //return true if the cell have the cell type (not the case for CELL_NONE)
    return (c->celltypes & types) != 0;     
}

void add_cell_type(Cell *c, const int type)
{
    c->celltypes |= type;
}

void set_cell_type(Cell* const c, const int type)
{
    c->celltypes = type;
}

/* ======================== GAME INIT FUNCTIONS ======================== */

/*
    Initializes all players, all valid and invalid cells, and Bawana area.
*/

void mark_cells(const int floor, const int start_w, const int end_w, const int start_l, const int end_l, const bool is_valid, const CellType type) 
{
    for(int w = start_w; w <= end_w; w++)
        for(int l = start_l; l <= end_l; l++) {
            cells[floor][w][l].is_valid = is_valid;
            set_cell_type(&cells[floor][w][l], type);
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
    int starting_cells[NUM_OF_PLAYERS][3] = {{0,6,12},{0,9,8},{0,9,16}};
    int first_cells[NUM_OF_PLAYERS][3] = {{0,5,12},{0,9,7},{0,9,17}};
    PlayerDirection dir_array[NUM_OF_PLAYERS] = {NORTH, WEST, EAST};

    for(int i = 0; i < NUM_OF_PLAYERS; i++)
    {
        players[i].name = name_array[i];
        players[i].player_state = INACTIVE;
        players[i].throw_count = 0;
        players[i].move_value = 0;
        players[i].mp_score = 10;
        players[i].direction = dir_array[i];
        players[i].starting_cell = &cells[starting_cells[i][0]][ starting_cells[i][1] ][ starting_cells[i][2] ];
        players[i].first_cell = &cells[first_cells[i][0]][ first_cells[i][1] ][ first_cells[i][2] ];
        players[i].init_dir = dir_array[i];
        // initial position
        players[i].player_pos = players[i].starting_cell;
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

void init_bawana()
{
    bawana_entrance_cell_ptr = &cells[0][9][19];
    set_cell_type(bawana_entrance_cell_ptr, BAWANA_ENTRANCE);
    //bawana wall
    for(int length = 20; length < MAX_LENGTH; length++)
    {
        set_cell_type(&cells[0][6][length], CELL_WALL);
    }
    for(int width = 7; width < MAX_WIDTH; width++)
    {
        set_cell_type(&cells[0][width][20], CELL_WALL);
    }
    //bawana cells
    int couples[12][2];  // 12 couples
    create_randomised_array(couples);

    //first 4 cells is set to normal bonus
    for(int i = 0; i < 4; i++)
    {
        set_cell_type(&cells[0][couples[i][0]][couples[i][1]], BAWANA_BONUS | CELL_BAWANA);
    }
    //next 2 cells will set to food poison
    set_cell_type(&cells[0][couples[4][0]][couples[4][1]], BAWANA_FOOD_POISONING | CELL_BAWANA);
    set_cell_type(&cells[0][couples[5][0]][couples[5][1]], BAWANA_FOOD_POISONING | CELL_BAWANA);
    //next 2 cells will set to disoriented
    set_cell_type(&cells[0][couples[6][0]][couples[6][1]], BAWANA_DISORIENT | CELL_BAWANA);
    set_cell_type(&cells[0][couples[7][0]][couples[7][1]], BAWANA_DISORIENT | CELL_BAWANA);
    //next 2 cells will set to triggered
    set_cell_type(&cells[0][couples[8][0]][couples[8][1]], BAWANA_TRIGGER | CELL_BAWANA);
    set_cell_type(&cells[0][couples[9][0]][couples[9][1]], BAWANA_TRIGGER | CELL_BAWANA);
    //next 2 cells will set to happy
    set_cell_type(&cells[0][couples[10][0]][couples[10][1]], BAWANA_HAPPY | CELL_BAWANA);
    set_cell_type(&cells[0][couples[11][0]][couples[11][1]], BAWANA_HAPPY | CELL_BAWANA);
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
                            set_cell_type(cell, CELL_NORMAL_CONSUMABLE);
                            break;
                        case 2:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '+';
                            cell->data.mpdata.value = (rand() % 2) + 1;
                            set_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                        case 3:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '+';
                            cell->data.mpdata.value = (rand() % 3) + 3;
                            set_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                        case 4:
                            cell->data.has_movementpoint = true;
                            cell->data.mpdata.factor = '*';
                            cell->data.mpdata.value = (rand() % 2) + 2;
                            set_cell_type(cell, CELL_NORMAL_BONUS);
                            break;
                    }
                }
            }
        }
    }
}

/* ======================== READ AND LOAD GAME DATA ======================== */

/*
    Read and validate input data from seed.txt flag.txt, staris.txt, poles.txt, and walls.txt
    Validate and load game data.
*/

char* get_current_datetime() 
{
    time_t rawtime;
    time(&rawtime);
    
    struct tm* timeinfo = localtime(&rawtime);
 
    if (time_buffer == NULL) return "[GetCurrentDateTimeFailed]"; 

    strftime(time_buffer, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return time_buffer;
}

void log_file_error(const FileErrors error, const char filename[])
{
    char file_loc[50];
    snprintf(file_loc, sizeof(file_loc), "%-35s", filename);

    switch(error)
    {
        case FILE_NOT_OPEN:
            fprintf(log_fp, "[ERROR] %-25s | %-40s | %s | %s\n",
                    "FILE_NOT_OPEN",
                    "Cannot open file",
                    file_loc,
                    get_current_datetime());
            break;
        case EMPTY_FILE:
            fprintf(log_fp, "[ERROR] %-25s | %-40s | %s | %s\n",
                    "EMPTY_FILE",
                    "File is empty, no data to process",
                    file_loc,
                    get_current_datetime());
            break;
    }
}

void log_file_error_line(const FileErrors error, const char filename[], const int line_number)
{
    char file_loc[50];
    snprintf(file_loc, sizeof(file_loc), "%-25s, line %-3d", filename, line_number);

    switch(error)
    {
        case INVALID_NUM_DIGITS:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "INVALID_NUM_DIGITS",
                    "Invalid number of digits, line skipped",
                    file_loc,
                    get_current_datetime());
            break;
        case NO_DIGITS:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "NO_DIGITS",
                    "No digits found, line skipped",
                    file_loc,
                    get_current_datetime());
            break;
        case INVALID_FORMAT_LINE:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "INVALID_FORMAT_LINE",
                    "Invalid format, line skipped",
                    file_loc,
                    get_current_datetime());
            break;
        default:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "UNKNOWN_FILE_ERROR",
                    "Unhandled file error",
                    file_loc,
                    get_current_datetime());
    }
}

void log_range_error(const RangeError error, const char filename[], const int line_number)
{
    char file_loc[50];
    snprintf(file_loc, sizeof(file_loc), "%-25s, line %-3d", filename, line_number);

    switch(error)
    {
        case FLOOR:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "FLOOR",
                    "Range error on floor",
                    file_loc,
                    get_current_datetime());
            break;
        case START_FLOOR:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "START_FLOOR",
                    "Range error on start floor",
                    file_loc,
                    get_current_datetime());
            break;
        case END_FLOOR:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "END_FLOOR",
                    "Range error on end floor",
                    file_loc,
                    get_current_datetime());
            break;
        case START_WIDTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "START_WIDTH",
                    "Range error on start width",
                    file_loc,
                    get_current_datetime());
            break;
        case START_LENGTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "START_LENGTH",
                    "Range error on start length",
                    file_loc,
                    get_current_datetime());
            break;
        case END_WIDTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "END_WIDTH",
                    "Range error on end width",
                    file_loc,
                    get_current_datetime());
            break;
        case END_LENGTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "END_LENGTH",
                    "Range error on end length",
                    file_loc,
                    get_current_datetime());
            break;
        case WIDTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "WIDTH",
                    "Range error on width",
                    file_loc,
                    get_current_datetime());
            break;
        case LENGTH:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "LENGTH",
                    "Range error on length",
                    file_loc,
                    get_current_datetime());
            break;
        default:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n",
                    "UNKNOWN_RANGE_ERROR",
                    "Unhandled range error",
                    file_loc,
                    get_current_datetime());
    }
}

void log_logic_error(const LogicError error, const char filename[], const int line_number)
{
    char file_loc[50];
    snprintf(file_loc, sizeof(file_loc), "%-25s, line %-3d", filename, line_number);

    switch(error)
    {
        case INVALID_FLAG_POSITION:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "INVALID_FLAG_POSITION", "FLAG position is invalid. Generate new flag", file_loc, get_current_datetime());
            break;
        case INVALID_POLE_DEFINITION:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "INVALID_POLE_DEFINITION", "Pole exit must be lower than entrance", file_loc, get_current_datetime());
            break;
        case POLE_ENTRANCE_INVALID: 
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","POLE_ENTRANCE_INVALID", "Pole entrance is invalid", file_loc, get_current_datetime());
            break;
        case POLE_EXIT_INVALID:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "POLE_EXIT_INVALID", "Pole exit is invalid", file_loc, get_current_datetime());
            break;
        case POLE_INTERCEPTION_INVALID:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "POLE_INTERCEPTION_INVALID", "Pole intercepts another object", file_loc, get_current_datetime());
            break;
        case INVALID_STAIR_DEFINITION:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "INVALID_STAIR_DEFINITION", "Stair definition is inconsistent", file_loc, get_current_datetime());
            break;
        case STAIR_START_INVALID:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "STAIR_START_INVALID", "Stair start position is invalid", file_loc,get_current_datetime());
            break;
        case STAIR_END_INVALID:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n", "STAIR_END_INVALID","Stair end position is invalid",file_loc,get_current_datetime());
            break;
        case STAIR_START_FULL:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","STAIR_START_FULL","Stair start cell already occupied",file_loc,get_current_datetime());
            break;
        case STAIR_END_FULL:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","STAIR_END_FULL","Stair end cell already occupied",file_loc,get_current_datetime());
            break;
        case INVALID_WALL_POSITION:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","INVALID_WALL_POSITION","Wall placed in invalid position",file_loc,get_current_datetime());
            break;
        case SKIP_WALL_CELL:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","SKIP_WALL_CELL","Skipped cell due to wall placement",file_loc,get_current_datetime());
            break;
        case DIAGONAL_WALL:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","DIAGONAL_WALL","Diagonal wall placement not allowed",file_loc,get_current_datetime());
            break;
        case FLAG_REACH_CHECK:
            fprintf(log_fp, "[INFO ] %-25s | %-40s | %57s\n", "FLAG_REACH_CHECK", "Flag is reachable", get_current_datetime());
            break;
        default:
            fprintf(log_fp, "[WARN ] %-25s | %-40s | %s | %s\n","UNKNOWN_LOGIC_ERROR","Unknown logic error",file_loc,get_current_datetime());
    }
}

void setup_logfile()
{
    log_fp = fopen("log.txt", "w");
    if(log_fp == NULL) perror("log.txt: warning: file open failed [LOG_CRSH]\n");
    fclose(log_fp);

    log_fp = fopen("log.txt", "a");
    if(log_fp == NULL) perror("log.txt: warning: file open failed [LOG_CRSH]\n");
}

unsigned int get_random_seed() 
{
    static bool seeded = false;     //seed for just once
    if (!seeded) {
        srand((unsigned int)time(NULL));   
        seeded = true;
    }
    unsigned int random_seed = ((unsigned int)rand() << 16) | ((unsigned int)rand() & 0xFFFF);
    return random_seed;
}

unsigned int get_seed()
{
    setup_logfile();

    FILE* seed_fp = fopen("seed.txt", "r");
    if(seed_fp == NULL)
    {
        log_file_error(FILE_NOT_OPEN, "seed.txt");
        return get_random_seed();   
    }
    if(fgetc(seed_fp) == EOF)
    {
        log_file_error(EMPTY_FILE, "seed.txt");
        return get_random_seed();
    }

    unsigned int value;
    int matched = fscanf(seed_fp, "%u", &value);

    if (matched != 1)   // If no unsigned int was found
    {
        log_file_error_line(INVALID_FORMAT_LINE, "seed.txt", 1);
        fclose(seed_fp);
        return get_random_seed();
    }
    
    int ch;
    while ((ch = fgetc(seed_fp)) != EOF) { 
        if (!isspace(ch)) 
        {  
            log_file_error_line(INVALID_FORMAT_LINE, "seed.txt", 1);
            fclose(seed_fp);
            return get_random_seed();
        }
    }
    fclose(seed_fp);
    return value;
}

Cell* generate_custom_flag() {
    int floor, width, length;

    while (1) {
        floor = rand() % NUM_OF_FLOORS;      // 0 to NUM_OF_FLOORS-1
        width = rand() % MAX_WIDTH;          // 0 to MAX_WIDTH-1
        length = rand() % MAX_LENGTH;        // 0 to MAX_LENGTH-1

        Cell* c = &cells[floor][width][length];

        // Check validity and not starting area / bawana
        if (c->is_valid && !check_cell_types(c, CELL_STARTING_AREA | CELL_BAWANA | BAWANA_ENTRANCE)) {
            add_cell_type(c, CELL_FLAG);
            return c;  // return the new valid flag cell
        }
    }
}

void load_flag_or_generate(const int* const digits, const char filename[], const int num_lines)
{
    int floor = digits[0];
    int width_num = digits[1];
    int length_num = digits[2];
    if( floor < 0 || floor >= NUM_OF_FLOORS) 
    {
        log_range_error(FLOOR, filename, num_lines); 
        return;
    }
    if( width_num < 0 || width_num >= MAX_WIDTH ) 
    {
        log_range_error(WIDTH, filename, num_lines); 
        return;
    }
    if( length_num < 0 || length_num >= MAX_LENGTH ) 
    {
        log_range_error(LENGTH, filename, num_lines); 
        return;
    }
    //Init flag
    if(cells[floor][width_num][length_num].is_valid && !check_cell_types(&cells[floor][width_num][length_num], CELL_STARTING_AREA | CELL_BAWANA | BAWANA_ENTRANCE) )
    {
        cell_flag = &cells[floor][width_num][length_num];
    }
    else 
    {
        log_logic_error(INVALID_FLAG_POSITION, filename, num_lines);
        cell_flag = generate_custom_flag();
    }

    while(!check_flag_reachability_for_all_players(cell_flag))
    {
        cell_flag = generate_custom_flag();
    }
    log_logic_error(FLAG_REACH_CHECK, filename, num_lines);
    add_cell_type(cell_flag, CELL_FLAG);
    flag_loaded = true;
}

void load_poles(const int* const digits, const char filename[], const int num_lines)
{
    int exit_floor = digits[0], enter_floor = digits[1];
    int width_num = digits[2], length_num = digits[3];

    if( exit_floor!=1 && exit_floor!=0 ) 
    {
        log_range_error(END_FLOOR, filename, num_lines); 
        return;
    }
    if( enter_floor!=1 && enter_floor!=2 ) 
    {
        log_range_error(START_FLOOR, filename, num_lines); 
        return;
    }
    if( width_num<0 || width_num>=MAX_WIDTH ) 
    {
        log_range_error(WIDTH, filename, num_lines); 
        return;
    }
    if( length_num<0 || length_num>=MAX_LENGTH ) 
    {
        log_range_error(LENGTH, filename, num_lines); 
        return;
    }
    //Handle conditions, return _numif a condition is false, if get to the last statement add the pole
    if(exit_floor>=enter_floor)
    {
        log_logic_error(INVALID_POLE_DEFINITION, filename, num_lines);
        return;
    }
    if(cells[enter_floor][width_num][length_num].is_valid==false)
    {
        log_logic_error(POLE_ENTRANCE_INVALID, filename, num_lines);
        return;
    }
    if(cells[exit_floor][width_num][length_num].is_valid==false)
    {
        log_logic_error(POLE_EXIT_INVALID, filename, num_lines);
        return;
    }

    if(exit_floor==0 && enter_floor==2 && cells[1][width_num][length_num].is_valid == false)
    {
        log_logic_error(POLE_INTERCEPTION_INVALID, filename, num_lines);
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

void load_stairs(const int* digits, const char filename[], const int num_lines)
{
    int start_floor = digits[0], start_width_num = digits[1], start_length_num = digits[2];
    int end_floor = digits[3], end_width_num = digits[4], end_length_num = digits[5];

    if( start_floor<0 || start_floor>1 ) 
    {
        log_range_error(START_FLOOR, filename, num_lines); 
        return;
    }
    if( start_width_num<0 || start_width_num>=MAX_WIDTH ) 
    {
        log_range_error(START_WIDTH, filename, num_lines); 
        return;
    }
    if( start_length_num<0 || start_length_num>=MAX_LENGTH ) 
    {
        log_range_error(START_LENGTH, filename, num_lines); 
        return;
    }
    if( end_floor!=1 && end_floor!=2 ) 
    {
        log_range_error(END_FLOOR, filename, num_lines); 
        return;
    }
    if( end_width_num<0 || end_width_num>=MAX_WIDTH ) 
    {
        log_range_error(END_WIDTH, filename, num_lines); 
        return;
    }
    if( end_length_num<0 || end_length_num>=MAX_LENGTH ) 
    {
        log_range_error(END_LENGTH, filename, num_lines); 
        return;
    }

    Cell* stair_start_cell = &cells[start_floor][start_width_num][start_length_num];
    Cell* stair_end_cell = &cells[end_floor][end_width_num][end_length_num];
    
    if(start_floor >= end_floor)
    {
        log_logic_error(INVALID_STAIR_DEFINITION, filename, num_lines);
        return;
    }
    if(stair_start_cell->is_valid==false)
    {
        log_logic_error(STAIR_START_INVALID, filename, num_lines);
        return;
    }
    if(stair_end_cell->is_valid==false)
    {
        log_logic_error(STAIR_END_INVALID, filename, num_lines);
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
        stair_start_cell->data.stairs[(stair_start_cell->data.stair_count)++] = new_stair;
        stair_end_cell->data.stairs[(stair_end_cell->data.stair_count)++] = new_stair;

        add_cell_type(stair_start_cell, CELL_STAIR_START);

        add_cell_type(stair_end_cell, CELL_STAIR_END);

        if(stair_start_cell->floor == 0 && stair_end_cell->floor == 2)
        {
            int middle_w = (stair_start_cell->width + stair_end_cell->width) / 2;
            int middle_l = (stair_start_cell->length + stair_end_cell->length) / 2;
            if(cells[1][middle_l][middle_w].is_valid)
            {
                cells[1][middle_l][middle_w].celltypes = CELL_WALL;
            }
        }
    }
    else
    {
        if(stair_start_cell->data.stair_count == 2)
        {
            log_logic_error(STAIR_START_FULL, filename, num_lines);
            return;
        }
        else
        {
            log_logic_error(STAIR_END_FULL, filename, num_lines);
            return;
        }
    }
}

int check_wall_init_conditions(const Cell* cell)
{
    if(cell->is_valid == true && 
        !check_cell_types(cell, CELL_FLAG | CELL_STAIR_START | CELL_STAIR_END | CELL_POLE_ENTER | CELL_POLE_EXIT | CELL_BAWANA | BAWANA_ENTRANCE | CELL_STARTING_AREA))
    {
        return 1;
    }
    else return 0;
}

void load_walls(const int* digits,const char filename[], const int num_lines)
{
    int floor = digits[0], start_width_num = digits[1], start_length_num = digits[2];
    int end_width_num = digits[3], end_length_num = digits[4];

    if( floor<0 || floor>2 ) 
    {
        log_range_error(START_FLOOR, filename, num_lines); 
        return;
    }
    if( start_width_num<0 || start_width_num>=MAX_WIDTH ) 
    {
        log_range_error(START_WIDTH, filename, num_lines); 
        return;
    }
    if( start_length_num<0 || start_length_num>=MAX_LENGTH ) 
    {
        log_range_error(START_LENGTH, filename, num_lines); 
        return;
    }
    if( end_width_num<0 || end_width_num>=MAX_WIDTH ) 
    {
        log_range_error(END_WIDTH, filename, num_lines); 
        return;
    }
    if( end_length_num<0 || end_length_num>=MAX_LENGTH ) 
    {
        log_range_error(END_LENGTH, filename, num_lines); 
        return;
    }
    //Handle conditions
    if(start_width_num == end_width_num)
    {
        if(start_length_num == end_length_num)  //single cell wall
        {
            if(check_wall_init_conditions(&cells[floor][start_width_num][start_length_num]))
            {
                set_cell_type(&cells[floor][start_width_num][start_length_num], CELL_WALL);
            }
            else 
            {
                log_logic_error(INVALID_WALL_POSITION, filename, num_lines);
                return;
            }
        }
        else    //horizontal wall
        {
            if(start_length_num > end_length_num)   
            {
                for(int length = start_length_num; length >= end_length_num; length--)   
                {
                    if(check_wall_init_conditions(&cells[floor][start_width_num][length]))
                    {
                        set_cell_type(&cells[floor][start_width_num][length], CELL_WALL);
                    }
                    else 
                    {
                        log_logic_error(SKIP_WALL_CELL, filename, num_lines);
                        break; //stop this wall, but keep loading the other walls
                    }
                }
            }
            else
            {
                for(int length = start_length_num; length <= end_length_num; length++)  
                {
                    if(check_wall_init_conditions(&cells[floor][start_width_num][length]))
                    {
                        set_cell_type(&cells[floor][start_width_num][length], CELL_WALL);
                    }
                    else 
                    {
                        log_logic_error(SKIP_WALL_CELL, filename, num_lines);
                        break; //stop this wall, but keep loading the other walls
                    }
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
                        set_cell_type(&cells[floor][width][start_length_num], CELL_WALL);
                    }
                    else 
                    {
                        log_logic_error(SKIP_WALL_CELL, filename, num_lines);
                        break; //stop this wall, but keep loading the other walls
                    }
                }
            }
            else
            {
                for(int width = start_width_num; width <= end_width_num; width++)   
                {
                    if(check_wall_init_conditions(&cells[floor][width][start_length_num]))
                    {
                        set_cell_type(&cells[floor][width][start_length_num], CELL_WALL);
                    }
                    else 
                    {
                        log_logic_error(SKIP_WALL_CELL, filename, num_lines);
                        break; //stop this wall, but keep loading the other walls
                    }
                }
            }
        }
        else    //diagonal wall [NOT ALLOWED]
        {   
            log_logic_error(DIAGONAL_WALL, filename, num_lines);
            return;
        }    
    }
}

void check_digits(int* digits, int count, char filename[], int num_lines)
{
    switch(count)
    {
        case 6:
            load_stairs(digits, filename, num_lines);
            break;
        case 5:
            load_walls(digits, filename, num_lines);
            break;
        case 4:
            load_poles(digits, filename, num_lines);
            break;
        case 3:
            if(!flag_loaded) load_flag_or_generate(digits, filename, num_lines); return;
        default:
            log_file_error_line(INVALID_NUM_DIGITS, filename, num_lines);
    }
}

int parse_and_process_line(const char* line, char filename[], int num_lines)
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

        check_digits(digits_array, count, filename, num_lines);
    }
    else
    {
        log_file_error_line(NO_DIGITS, filename, num_lines);
    }
    
    return 1;
}

int validate_file(char filename[])
{
    FILE* dataf = fopen(filename, "r");
    if(dataf == NULL)  
    {
        log_file_error(FILE_NOT_OPEN, filename);
        exit(1);
        return 0;
    }

    int ch = fgetc(dataf);
    if (ch == EOF) 
    {
        log_file_error(EMPTY_FILE, filename);
        return 0;
    }

    rewind(dataf);  

    char buffer[1024]; 
    int line_num = 1;

    while(fgets(buffer, sizeof(buffer), dataf))     
    {
        buffer[strcspn(buffer, "\n")] = '\0';       
        if(!parse_and_process_line(buffer, filename, line_num))
        {
            log_file_error_line(INVALID_FORMAT_LINE, filename, line_num);
        }
        line_num++;
    }
    fclose(dataf);
    return 1;
}

void read_data()
{
    char* files[] = {"poles.txt", "stairs.txt", "walls.txt", "flag.txt"};

    for(int index = 0; index < 4; index++)
    {
        validate_file(files[index]);
    }
}

/* ================ UTILITY FUNCTIONS FOR PLAYER MOVE ================ */

/*
    Functions that support player movement functions and functions that 
    support player movement.
*/

char* get_player_dir_string(PlayerDirection dir)
{
    switch (dir)
    {
    case NORTH: return "North";
    case SOUTH: return "South";
    case EAST: return "East";
    case WEST: return "West";
    default: return "Unknown";
    }
}

char* get_bawana_cell_type(CellType celltype)
{
    if (celltype & BAWANA_BONUS)          return "Bonus";
    if (celltype & BAWANA_FOOD_POISONING) return "Food Poison";
    if (celltype & BAWANA_DISORIENT)      return "Disorient";
    if (celltype & BAWANA_TRIGGER)        return "Trigger";
    if (celltype & BAWANA_HAPPY)          return "Happy";

    return "Unknown";
}

void print_output(MoveResult result, Player* p, int path_cost, Cell* trigger_cell, Cell* dest_cell)
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
            printf("Player %c rolls and %d on the movement dice and cannot move in the %s. Player remains at [%d, %d, %d]\n", p->name, p->move_value, get_player_dir_string(p->direction), p->player_pos->floor, p->player_pos->width, p->player_pos->length);
            return;
        case COMPLETE_MOVE:
            printf("Player %c moved %d that cost %+d movement points and is left with %d and is moving in the %s.\n", p->name, p->move_value, path_cost, p->mp_score, get_player_dir_string(p->direction));
            return;
        case START_MOVE:
            printf("Player %c rolls and %d on the movement dice and moves %s by %d cells and is now at [%d, %d, %d].\n", p->name, p->move_value, get_player_dir_string(p->direction), p->move_value, p->player_pos->floor, p->player_pos->width, p->player_pos->length);
            return;
        case START_MOVE_WITH_DIRECTION:
            printf("Player %c rolls and %d on the movement dice and %s on the direction dice, changes direction to %s and moves %d cells and is now at [%d, %d, %d].\n", p->name, p->move_value, get_player_dir_string(p->direction), get_player_dir_string(p->direction), p->move_value, p->player_pos->floor, p->player_pos->width, p->player_pos->length);
            return;
        case TAKE_POLE:
            printf("Player %c lands on [%d, %d, %d] which is a pole cell. Player %c slides down an now placed at [%d, %d] in floor %d.\n", p->name, trigger_cell->floor, trigger_cell->width, trigger_cell->length, p->name, dest_cell->width, dest_cell->length, dest_cell->floor);
            return;
        case TAKE_STAIR:
            printf("Player %c lands on [%d, %d, %d] which is a stair cell. Player %c takes the stair and now placed at [%d, %d] in floor %d.\n", p->name, trigger_cell->floor, trigger_cell->width, trigger_cell->length, p->name, dest_cell->width, dest_cell->length, dest_cell->floor);
            return;
        case PLAYER_WIN:
            printf("Player %c lands on [%d, %d, %d] which is the flag cell. Player %c won.\n", p->name, trigger_cell->floor, trigger_cell->width, trigger_cell->length, p->name);
            return;
        case TO_BAWANA:
            printf("Player %c movement points are depleted and requires replenishment. Transporting to Bawana.\n", p->name);
            printf("Player %c is place on a %s cell and effects take place.\n", p->name, get_bawana_cell_type(p->player_pos->celltypes));
            return;
        case TO_BAWANA_BONUS:
            printf("Player %c eats from Bawana and earns %d movement points and is placed at the entrance of Bawana.\n", p->name, p->mp_score);
            return;
        case TO_BAWANA_FP:
            printf("Player %c eats from Bawana and have a bad case of food poisoning. Will need three rounds to recover.\n", p->name);
            return;
        case TO_BAWANA_DISORIENT:
            printf("Player %c eats from Bawana and is disoriented and is placed at the entrance of Bawana with 50 movement points.\n", p->name);
            return;
        case TO_BAWANA_TRIGGER:
            printf("Player %c eats from Bawana and is triggered due to bad quality of food. Player %c is placed at the entrance of Bawana with 50 movement points.\n", p->name, p->name);
            return;
        case TO_BAWANA_HAPPY:
            printf("Player %c eats from Bawana and is happy. Player %c is placed at the entrance of Bawana with 200 movement points.\n", p->name, p->name);
            return;
        default:
            printf("[PrintOutputError] Please contact developers! %d\n", result);
            exit(1);
    }
}

PlayerDirection generate_random_player_direction()
{
    switch(rand() % 4)
    {
        case 0: return NORTH;
        case 1: return SOUTH;
        case 2: return EAST;
        case 3: return WEST;
    }
}

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

/* ======================== EVENT HANDLING FUNCTIONS ======================== */
   
/*
    Manages the in-game event log for player actions. Each event that occurs during
    a player's move is stored in a non-circular queue, following LIFO policy 
    to provide a step-by-step trace of gameplay actions if in a complete move.
*/

bool event_list_full()
{
    return ((rear + 1) % MAX_EVENTS_PER_EACH_MOVE) == front;
}

bool event_list_empty()
{
    return front == -1;
}

MoveResult peek_last_event()
{
    return event_list[rear].event;
}

void reset_event_list()
{
    front = rear = -1;
}

void add_event(MoveResult event_type, Cell* trigger_cell, Cell* dest_cell)
{
    if(event_list_full())
    {
        printf("EVENT_LST_FULL_ERR (rear=%d): Please Contact support@example.com\n", rear);
        exit(1);
    }
    rear = (rear + 1) % MAX_EVENTS_PER_EACH_MOVE;
    if(event_list_empty()) front = 0;
    event_list[rear].event = event_type;     
    event_list[rear].data.trigger_cell = trigger_cell;
    event_list[rear].data.dest_cell = dest_cell;
}

void print_event_list(Player* plyr, int path_cost)
{
    if(event_list_empty())
    {
        printf("EVNT_LST_EMPTY_ERR : Please contact support@example.com\n");
        return;
    }

    int i = front;
    while (true) 
    {
        print_output(event_list[i].event, plyr, path_cost, event_list[i].data.trigger_cell, event_list[i].data.dest_cell);
        if (i == rear) break;
        i = (i + 1) % MAX_EVENTS_PER_EACH_MOVE;
    }
    reset_event_list();
}

/* ================ SUPPORT FUNCTIONS FOR HANDLER FUNCTIONS ================ */
   
/*
    Handle player movement logic, updating player position,
    and validating moves based on player states and environment constraints.
*/

HandlerResult handle_stair_start(Player* plyr, int* path_cost);
HandlerResult handle_stair_end(Player* plyr, int* path_cost);
HandlerResult handle_pole_enter(Player* plyr, int* paht_cost);
static const CellAction actions[];

HandlerResult check_landed_cell(Player* plyr, int* path_cost)
{
    if(check_cell_types(plyr->player_pos, CELL_FLAG))
    {
        return WIN_GAME;
    }
    else if(check_cell_types(plyr->player_pos, CELL_BAWANA))
    {
        return PLACED_ON_BAWANA;
    }
    else if(check_cell_types(plyr->player_pos, CELL_STAIR_START))
    {
        handle_stair_start(plyr, path_cost);
    }
    else if(check_cell_types(plyr->player_pos, CELL_STAIR_END))
    {
        handle_stair_end(plyr, path_cost);
    }
    else if(check_cell_types(plyr->player_pos, CELL_POLE_ENTER))
    {
        handle_pole_enter(plyr, path_cost);
    }
    else
    {
        return CONTINUE_STEP;
    }
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

/* ========================== CELL HANDLERS ========================== */
/* 
    Handle player interactions with specific cell types. 
*/

void player_to_bawana(Player* plyr);

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

HandlerResult handle_stair(Player* plyr, StairDirection wrong_dir, Cell* stair0_dest_cell, Cell* stair1_dest_cell, int* path_cost, StairCheckType check_type)
{
    Cell* cell = plyr->player_pos;
    if (cell->data.stair_count == 1)
    {
        if (cell->data.stairs[0]->direction != wrong_dir &&
            (check_type == CHECK_START_CELL ? cell->data.stairs[0]->start_cell : cell->data.stairs[0]->end_cell) == cell)
        {
            add_event(TAKE_STAIR, plyr->player_pos, stair0_dest_cell);
            plyr->player_pos = stair0_dest_cell;
            return check_landed_cell(plyr, path_cost);
        }
        else
        {
            return ABORT_MOVE;
        }
    }
    else
    {
        bool stair0_matches = (check_type == CHECK_START_CELL ? cell->data.stairs[0]->start_cell : cell->data.stairs[0]->end_cell) == cell;
        bool stair1_matches = (check_type == CHECK_START_CELL ? cell->data.stairs[1]->start_cell : cell->data.stairs[1]->end_cell) == cell;

        if (stair0_matches && stair1_matches)
        {
            if (cell->data.stairs[0]->direction != wrong_dir && cell->data.stairs[1]->direction != wrong_dir)
            {
                Cell* temp = plyr->player_pos;
                plyr->player_pos = pick_stair_heuristic(stair0_dest_cell, stair1_dest_cell, cell_flag);
                add_event(TAKE_STAIR, temp, plyr->player_pos);
                return check_landed_cell(plyr, path_cost);
            }
            else if (cell->data.stairs[0]->direction != wrong_dir)
            {
                add_event(TAKE_STAIR, plyr->player_pos, stair0_dest_cell);
                plyr->player_pos = stair0_dest_cell;
                return check_landed_cell(plyr, path_cost);
            }
            else if (cell->data.stairs[1]->direction != wrong_dir)
            {
                add_event(TAKE_STAIR, plyr->player_pos, stair1_dest_cell);
                plyr->player_pos = stair1_dest_cell;
                return check_landed_cell(plyr, path_cost);
            }
            else
            {
                return ABORT_MOVE;
            }
        }
        else if (stair0_matches && cell->data.stairs[0]->direction != wrong_dir)
        {
            add_event(TAKE_STAIR, plyr->player_pos, stair0_dest_cell);
            plyr->player_pos = stair0_dest_cell;
            return check_landed_cell(plyr, path_cost);
        }
        else if (stair1_matches && cell->data.stairs[1]->direction != wrong_dir)
        {
            add_event(TAKE_STAIR, plyr->player_pos, stair1_dest_cell);
            plyr->player_pos = stair1_dest_cell;
            return check_landed_cell(plyr, path_cost);
        }
        else
        {
            return ABORT_MOVE;
        }
    }
}

HandlerResult handle_stair_start(Player* plyr, int* path_cost)
{
    if (plyr->player_pos->data.stair_count == 1 || plyr->player_pos->data.stairs[1] == NULL) 
    {
        return handle_stair( 
            plyr,
            DOWN,   //wrong direction
            plyr->player_pos->data.stairs[0]->end_cell,
            NULL,    // second stair doesn’t exist
            path_cost,  CHECK_START_CELL
        );
    }

    // Two stairs
    return handle_stair(
        plyr,
        DOWN,   //wrong direction
        plyr->player_pos->data.stairs[0]->end_cell,
        plyr->player_pos->data.stairs[1]->end_cell,
        path_cost, CHECK_START_CELL
    );
}

HandlerResult handle_stair_end(Player* plyr, int* path_cost)
{
    if (plyr->player_pos->data.stair_count == 1 || plyr->player_pos->data.stairs[1] == NULL) 
    {
        return handle_stair(plyr,
            UP, //wrong direction
            plyr->player_pos->data.stairs[0]->start_cell,
            NULL,  // second stair doesn’t exist
            path_cost, CHECK_END_CELL
        );
    }

    // Two stairs
    return handle_stair(
        plyr,
        UP, //wrong direction
        plyr->player_pos->data.stairs[0]->start_cell,
        plyr->player_pos->data.stairs[1]->start_cell,
        path_cost, CHECK_END_CELL
    );
}

HandlerResult handle_pole_enter(Player* plyr, int* path_cost)
{
    add_event(TAKE_POLE, plyr->player_pos, plyr->player_pos->data.pole_data.dest_cell);
    plyr->player_pos = plyr->player_pos->data.pole_data.dest_cell;
    return check_landed_cell(plyr, path_cost);
}

HandlerResult handle_no_special_effect_cell(Player* plyr, int* ignore_val)
{
    return CONTINUE_STEP;
}

HandlerResult handle_starting_area(Player* plyr, int* ignore_val)
{
    MoveResult prev_move = peek_last_event();
    if(prev_move == TAKE_STAIR || prev_move == TAKE_POLE)
    {
       plyr->player_pos = plyr->starting_cell;
       plyr->player_state = INACTIVE; 
       plyr->direction = plyr->init_dir;
    }
    return ABORT_MOVE;
}

HandlerResult handle_flag(Player* plyr, int* ignore_val)
{
    return WIN_GAME;
}

static const CellAction actions[] = {                           
    { CELL_FLAG,                handle_flag             },                  
    { CELL_STAIR_START,         handle_stair_start      },
    { CELL_STAIR_END,           handle_stair_end        },
    { CELL_POLE_ENTER,          handle_pole_enter       },
    { CELL_NORMAL_BONUS,        handle_normal_bonus     },
    { CELL_NORMAL_CONSUMABLE,   handle_normal_consumable},
    { BAWANA_ENTRANCE,          handle_no_special_effect_cell   },
    { CELL_POLE_EXIT,           handle_no_special_effect_cell   },
    { CELL_WALL,                handle_wall                     },
    { CELL_STARTING_AREA,       handle_starting_area            },
    { CELL_NONE,                handle_no_special_effect_cell   },
};

/* ======================== PLAYER MOVEMENT FUNCTIONS ======================== */
   
/*
    Handle player movement logic, updating player position,
    and validating moves based on player states and environment constraints.
*/

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
            length++; //move left
            break;
        case WEST:
            length--; //move right
            break;
    }

    if(width < 0)
    {
        return NORTH_BOUND;   
    }
    else if(width >= MAX_WIDTH)
    {
        return SOUTH_BOUND;
    }
    else if(length < 0)
    {
        return WEST_BOUND;
    }
    else if(length >= MAX_LENGTH)
    {
        return EAST_BOUND;
    }
    
    plyr->player_pos = &cells[floor][width][length];
    return NO_BOUNDS;
}

/* Apply the effects to the player based on his cell position and print output */
void player_to_bawana(Player* plyr)
{
    if(check_cell_types(plyr->player_pos, BAWANA_BONUS))
    {
        plyr->player_state = ACTIVE;
        plyr->mp_score = (rand() % 91) + 10;
        plyr->player_pos = bawana_entrance_cell_ptr;
        plyr->direction = NORTH;
        print_output(TO_BAWANA_BONUS, plyr, 0, NULL, NULL);
    }
    else if(check_cell_types(plyr->player_pos, BAWANA_FOOD_POISONING))
    {
        print_output(TO_BAWANA_FP, plyr, 0, NULL, NULL);
        plyr->player_state = FOOD_POISONED;
        plyr->effect_turns = 3;
    }   
    else if(check_cell_types(plyr->player_pos, BAWANA_DISORIENT))
    {
        print_output(TO_BAWANA_DISORIENT, plyr, 0, NULL, NULL);
        plyr->player_state = DISORIENTED;
        plyr->mp_score = 50;
        plyr->player_pos = bawana_entrance_cell_ptr;
        plyr->direction = NORTH;
        plyr->effect_turns = 4;
    }
    else if(check_cell_types(plyr->player_pos, BAWANA_TRIGGER))
    {
        print_output(TO_BAWANA_TRIGGER, plyr, 0, NULL, NULL);
        plyr->player_state = TRIGGERED;
        plyr->mp_score = 50;
        plyr->player_pos = bawana_entrance_cell_ptr;
        plyr->direction = NORTH;
    }
    else if(check_cell_types(plyr->player_pos, BAWANA_HAPPY))
    {
        print_output(TO_BAWANA_HAPPY, plyr, 0, NULL, NULL);
        plyr->player_state = ACTIVE;    
        plyr->mp_score = 200;
        plyr->player_pos = bawana_entrance_cell_ptr;
        plyr->direction = NORTH;
    }
}

void capture_player(int current_player_index)
{
    for(int p_index = 0; p_index < NUM_OF_PLAYERS; p_index++)
    {
        if(p_index == current_player_index) continue;
        
        if(players[current_player_index].player_pos == players[p_index].player_pos)
        {
            players[p_index].player_pos = players[p_index].starting_cell;
            players[p_index].player_state = INACTIVE;
            players[p_index].direction = players[p_index].init_dir;
            players[p_index].effect_turns = 0;
            printf("Player %c capture Player %c. Player %c is placed at the [%d, %d, %d] in starting area and direction is changed to %s.\n", 
                players[current_player_index].name, players[p_index].name, players[p_index].name, players[p_index].player_pos->floor,
                players[p_index].player_pos->width, players[p_index].player_pos->length, get_player_dir_string(players[p_index].direction));
        }
    }
}

void process_movement_blocked(Player* plyr, Cell* position_before_move)
{
    plyr->player_pos = position_before_move;
    print_output(MOVEMENT_BLOCKED, plyr, 0, NULL, NULL);
    plyr->mp_score -= 2;
    plyr->move_value = 0;
    print_output(COMPLETE_MOVE, plyr, -2, NULL, NULL);
}

void handle_cell_traversal(Player* plyr, PlayerDirection move_direction,int player_index, int move_factor)
{
    Cell* position_before_move = plyr->player_pos;
    int path_cost = 0;
    
    for(int step = 0; step < (plyr->move_value * move_factor); step++)
    {
        if(move_one_cell(plyr, move_direction) != NO_BOUNDS)
        {
            process_movement_blocked(plyr, position_before_move);
            goto partial_move;    //terminates step by exiting the for loop
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
                        break;  //take the next step
                    }
                    else if(step_result == ABORT_MOVE)
                    {
                        process_movement_blocked(plyr, position_before_move);
                        goto partial_move;  
                    }
                    else if(step_result == WIN_GAME)    
                    {
                        game_on = false;
                        add_event(PLAYER_WIN, plyr->player_pos, NULL);
                        print_event_list(plyr, path_cost);
                        return;
                    }
                    else    //PLACED_ON_BAWANA
                    {
                        print_event_list(plyr, path_cost);
                        player_to_bawana(plyr);
                        return;
                    }
                }   
            }
        }
        else
        {
            process_movement_blocked(plyr, position_before_move);
            goto partial_move;
        }
    }    
    capture_player(player_index);
    plyr->mp_score += path_cost;
    add_event(COMPLETE_MOVE, NULL, NULL);
    print_event_list(plyr, path_cost);

    partial_move:;
    plyr->throw_count++;

    if(plyr->mp_score <= 0)
    {
        plyr->player_pos = &cells[0][(rand() % 3) + 7][(rand() % 4) + 21];  //place player on a random cell in bawana
        print_output(TO_BAWANA, plyr, 0, NULL, NULL);
        player_to_bawana(plyr);
    }
}

void handle_player_state_movement(Player* plyr, int player_index)
{
    /* Start the Move by rolling the dice and adding the event to the queue */
    plyr->move_value = movement_dice();
    printf("\n");

    if(plyr->throw_count % 4 ==0 && plyr->throw_count > 0)
    {
        plyr->direction = direction_dice(plyr);
        add_event(START_MOVE_WITH_DIRECTION, NULL, NULL);
    }
    else add_event(START_MOVE, NULL, NULL);

    /* Handle the player move according to the player state */
    if(plyr->player_state == ACTIVE)
    {
        handle_cell_traversal(plyr, plyr->direction, player_index, 1);
    }
    else
    {
        if(plyr->player_state == INACTIVE)
        {
            if(plyr->move_value == 6)
            {
                plyr->player_pos = plyr->first_cell;
                plyr->player_state = ACTIVE;
                print_output(EXIT_STARTING_AREA, plyr, 0, NULL, NULL);
            }
            else
            {
                print_output(REMAIN_AT_SPAWN, plyr, 0, NULL, NULL);
            }
        } 
        else
        { 
            if(plyr->player_state == FOOD_POISONED)
            {
                plyr->throw_count++;
                if(plyr->effect_turns == 0)
                {
                    plyr->player_pos = &cells[0][(rand() % 3) + 7][(rand() % 4) + 21];
                    printf("Player %c is now fit to proceed form the food poisoning episode and now placed on a %s and the effects take place.\n", plyr->name, get_bawana_cell_type(plyr->player_pos->celltypes));
                    player_to_bawana(plyr);
                    reset_event_list();
                    return;
                }
                printf("Player %c is still food poisoned and misses the turn.\n", plyr->name);
                plyr->effect_turns--;
            }
            else
            {
                if(plyr->player_state == DISORIENTED)
                {
                    plyr->throw_count++;
                    if(plyr->effect_turns == 0)
                    {
                        plyr->player_state = ACTIVE;
                        printf("Player %c has recovered from disorientation.\n", plyr->name);
                        handle_cell_traversal(plyr, plyr->direction, player_index, 1);
                        reset_event_list();
                        return;
                    }
                    plyr->effect_turns--;
                    PlayerDirection rand_dir = generate_random_player_direction();
                    handle_cell_traversal(plyr, rand_dir, player_index, 1);
                    printf("Player %c rolls and %d on the movement dice and is disoriented and move in the %s and moves %d cells and is placed at the [%d, %d, %d].\n", plyr->name, plyr->move_value, get_player_dir_string(rand_dir), plyr->move_value, plyr->player_pos->floor, plyr->player_pos->width, plyr->player_pos->length);
                }
                else 
                {
                    if(plyr->player_state == TRIGGERED)
                    {
                        handle_cell_traversal(plyr, plyr->direction, player_index, 2);
                        printf("Player %c is triggered and rolls and %d on the movement dice and move in the %s and moves %d cells and is placed at the [%d, %d, %d].\n",  plyr->name, plyr->move_value, get_player_dir_string(plyr->direction), (plyr->move_value)*2, plyr->player_pos->floor, plyr->player_pos->width, plyr->player_pos->length);
                    }
                    else
                    {
                        printf("PLYR_STATE_MOVE_ERR : Please Contact support@example.com\n");
                    }
                }
            }
        }
    }
    reset_event_list();
}

/* ======================== FLAG REACHABILITY CHECK ======================== */

/*
    Performs BFS from the flag cell to determine if all players' starting
    positions are reachable. Considers valid cells, walls, one-way poles,
    and bidirectional stairs (ignores stairs starting in starting area cells).
*/

bool is_qfull() { return (rearq + 1) % MAX_SIZE == frontq; }
bool is_qempty() { return frontq == -1; }

void enqueue(Cell* cell) 
{
    if (is_qfull()) return;
    if (is_qempty()) frontq = rearq = 0;
    else rearq = (rearq + 1) % MAX_SIZE;
    queue[rearq] = cell;
}

Cell* dequeue() 
{
    if (is_qempty()) return NULL;
    Cell* cell = queue[frontq];
    if (frontq == rearq) frontq = rearq = -1;
    else frontq = (frontq + 1) % MAX_SIZE;
    return cell;
}

bool in_bounds(int f, int w, int l) 
{
    return f >= 0 && f < NUM_OF_FLOORS &&
           w >= 0 && w < MAX_WIDTH &&
           l >= 0 && l < MAX_LENGTH;
}

bool check_flag_reachability_for_all_players(Cell* flag_cell) 
{
    frontq = rearq = -1;
    memset(visited, 0, sizeof(visited));

    enqueue(flag_cell);
    visited[flag_cell->floor][flag_cell->width][flag_cell->length] = true;

    // Track which players' starting cells have been reached
    bool reached_players[NUM_OF_PLAYERS];
    for (int i = 0; i < NUM_OF_PLAYERS; i++) reached_players[i] = false;

    while (!is_qempty()) 
    {
        Cell* cell = dequeue();

        // Check if this cell is any player's starting cell
        for (int i = 0; i < NUM_OF_PLAYERS; i++) {
            PlayerStart ps = player_starts[i];
            if (!reached_players[i] &&
                ps.floor == cell->floor &&
                ps.width == cell->width &&
                ps.length == cell->length) 
            {
                reached_players[i] = true;
            }
        }

        // Check if all players reached
        bool all_reached = true;
        for (int i = 0; i < NUM_OF_PLAYERS; i++) 
        {
            if (!reached_players[i]) 
            {
                all_reached = false;
                break;
            }
        }
        if (all_reached) return true;

        // --- 1. Normal moves ---
        int df[4] = {0, 0, 0, 0};
        int dw[4] = {-1, 1, 0, 0};
        int dl[4] = {0, 0, -1, 1};

        for (int d = 0; d < 4; d++) 
        {
            int nf = cell->floor + df[d];
            int nw = cell->width + dw[d];
            int nl = cell->length + dl[d];

            if (!in_bounds(nf, nw, nl)) continue;
            if (visited[nf][nw][nl]) continue;

            Cell* neighbor = &cells[nf][nw][nl];
            if (!neighbor->is_valid) continue;
            if (neighbor->celltypes & CELL_WALL) continue;

            enqueue(neighbor);
            visited[nf][nw][nl] = true;
        }

        // --- 2. Stairs (bidirectional) ---
        for (int i = 0; i < cell->data.stair_count; i++) 
        {
            Stair* stair = cell->data.stairs[i];
                if (!stair) continue;

            // Skip stairs that originate in a starting area cell
            if (cell->celltypes & CELL_STARTING_AREA)
                continue;

            // Upward move
            Cell* dest_up = stair->end_cell;
            if (!visited[dest_up->floor][dest_up->width][dest_up->length]) 
            {
                enqueue(dest_up);
                visited[dest_up->floor][dest_up->width][dest_up->length] = true;
            }

            // Downward move
            Cell* dest_down = stair->start_cell;
            if (!visited[dest_down->floor][dest_down->width][dest_down->length]) 
            {
                enqueue(dest_down);
                visited[dest_down->floor][dest_down->width][dest_down->length] = true;
            }
        }


        // --- 3. Poles (one-way only) ---
        if (cell->celltypes & CELL_POLE_ENTER) 
        {
            Cell* dest = cell->data.pole_data.dest_cell;
            if (dest && !visited[dest->floor][dest->width][dest->length]) 
            {
                enqueue(dest);
                visited[dest->floor][dest->width][dest->length] = true;
            }
        }
    }

    // If BFS finishes and not all players reached
    return false;
}


/* ======================== GAMEPLAY FUNCTIONS ======================== */
   
/*
    Functions that control overall game flow and manage stair direction logic.
*/

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

void play_game()
{
    time_buffer = (char*)malloc(20 * sizeof(char));
    stairs_array = malloc(stair_array_capacity * sizeof(Stair*));
    
    srand(get_seed());
    init_cells();
    init_players();
    init_bawana();
    read_data();
    set_normal_cells();
    
    int player_index = 0;
    while(game_on)
    {
        if(player_index == 0)
        {
            game_round++;
            printf("\n---------- GAME ROUND: %d----------\n", game_round);
        }
        if(game_round % GAME_SETTING == 0) change_stair_direction();
        
        handle_player_state_movement(&players[player_index], player_index);
        
        // get the player index for the next player 
        player_index = (player_index + 1) % NUM_OF_PLAYERS;
    }
    fclose(log_fp);
    free(time_buffer);

    for(int i = 0; i < stair_count; i++) free(stairs_array[i]);
    free(stairs_array);
}
