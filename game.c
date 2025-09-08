#include "game.h"

#define MAX_LINE_LEN 256

Player players[NUM_OF_PLAYERS];
Cell cells[NUM_OF_FLOORS][MAX_WIDTH][MAX_LENGTH];

int game_round = 0;

void mark_cells(int floor, int start_w, int end_w, int start_l, int end_l, bool is_valid, CellType type) {
    for(int w = start_w; w <= end_w; w++)
        for(int l = start_l; l <= end_l; l++) {
            cells[floor][w][l].is_valid = is_valid;
            if(cells[floor][w][l].is_valid) cells[floor][w][l].celltype = type;
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
            }
        }
    }
    mark_cells(0, 6, 9, 8, 16, true, STARTING_AREA);
    mark_cells(1, 0, 5, 8, 16, false, NONE);
    mark_cells(2, 6, 9, 0, 7, false, NONE);
    mark_cells(2, 6, 9, 17, 24, false, NONE);
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

/*
    read input data, validate each line while updating the relevant cells
*/

void print_file_error(FileErrors error, char filename[], FILE *logfile)
{
    switch(error)
    {
        case FILE_NOT_OPEN:fprintf(logfile, "[ERROR] Cannot open (file: %s)\n", filename);break;
        case EMPTY_FILE:fprintf(logfile, "[ERROR] File is empty (file: %s). No data to process.\n", filename);break;
        default: fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");break;
    }
}

void print_file_error_line(FileErrors error, char filename[], FILE *logfile, int line_number)
{
    switch (error)
    {
        case INVALID_NUM_DIGITS:fprintf(logfile, "[WARN] Invalid number of digits (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case NO_DIGITS:fprintf(logfile, "[WARN] No digits found (file %s, line %d). Line skipped\n", filename, line_number);break;
        case INVALID_FORMAT_LINE:fprintf(logfile, "[WARN] Invalid format (file %s, line %d). Line skipped.\n", filename, line_number);break;
        default: fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");break;
    }
}

void print_range_error(RangeError error, char filename[], FILE* logfile, int line_number)
{
    switch(error)
    {
        case START_FLOOR:fprintf(logfile, "[WARN] Range error on start floor (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case END_FLOOR:fprintf(logfile, "[WARN] Range error on end floor (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case START_WIDTH:fprintf(logfile, "[WARN] Range error on start width (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case START_LENGTH:fprintf(logfile, "[WARN] Range error on start length (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case END_WIDTH:fprintf(logfile, "[WARN] Range error on end width (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case END_LENGTH:fprintf(logfile, "[WARN] Range error on end length (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case WIDTH:fprintf(logfile, "[WARN] Range error on width (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case LENGTH:fprintf(logfile, "[WARN] Range error on length (file %s, line %d). Line skipped.\n", filename, line_number);break;
        default: fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");break;
    }
}

void load_stairs(int* digits, char filename[], FILE* logfile, int num_lines)
{
    if( *(digits)<0 || *(digits)>1 ) print_range_error(START_FLOOR, filename, logfile, num_lines); return;
    if( *(digits + 1)<0 || *(digits + 1)>=MAX_WIDTH ) print_range_error(START_WIDTH, filename, logfile, num_lines); return;
    if( *(digits + 2)<0 || *(digits + 2)>=MAX_LENGTH ) print_range_error(START_LENGTH, filename, logfile, num_lines); return;
    if( *(digits + 3)!=1 || *(digits + 3)!=2 ) print_range_error(END_FLOOR, filename, logfile, num_lines); return;
    if( *(digits + 4)<0 || *(digits + 4)>=MAX_WIDTH ) print_range_error(END_WIDTH, filename, logfile, num_lines); return;
    if( *(digits + 5)<0 || *(digits + 5)>=MAX_LENGTH ) print_range_error(END_LENGTH, filename, logfile, num_lines); return;

    //Handle conditions, return if a condition is false, if get to the last statement add the stair
}
void load_walls(int* digits, char filename[], FILE* logfile, int num_lines)
{
    if( *(digits)<0 || *(digits)>2 ) print_range_error(START_FLOOR, filename, logfile, num_lines); return;
    if( *(digits + 1)<0 || *(digits + 1)>=MAX_WIDTH ) print_range_error(START_WIDTH, filename, logfile, num_lines); return;
    if( *(digits + 2)<0 || *(digits + 2)>=MAX_LENGTH ) print_range_error(START_LENGTH, filename, logfile, num_lines); return;
    if( *(digits + 3)<0 || *(digits + 1)>=MAX_WIDTH ) print_range_error(END_WIDTH, filename, logfile, num_lines); return;
    if( *(digits + 4)<0 || *(digits + 2)>=MAX_LENGTH ) print_range_error(END_LENGTH, filename, logfile, num_lines); return;

    //Handle conditions, return if a condition is false, if get to the last statement add the walls
}
void load_poles(int* digits, char filename[], FILE* logfile, int num_lines)
{
    if( *(digits)!=1 || *(digits)!=0 ) print_range_error(START_FLOOR, filename, logfile, num_lines); return;
    if( *(digits + 1)!=1 || *(digits + 1)!=2 ) print_range_error(END_FLOOR, filename, logfile, num_lines); return;
    if( *(digits + 2)<0 || *(digits + 2)>=MAX_WIDTH ) print_range_error(WIDTH, filename, logfile, num_lines); return;
    if( *(digits + 3)<0 || *(digits + 3)>=MAX_LENGTH ) print_range_error(WIDTH, filename, logfile, num_lines); return;

    //Handle conditions, return if a condition is false, if get to the last statement add the pole
}

void check_digits(int* digits, int count, char filename[], FILE* logfile, int num_lines)
{
    switch(count)
    {
        case 6:
            printf("Will load stairs\n");
            load_stairs(digits, filename, logfile, num_lines);
            break;
        case 5:
            printf("Will load walls\n");
            load_walls(digits, filename, logfile, num_lines);
            break;
        case 4:
            printf("Will load poles\n");
            load_poles(digits, filename, logfile, num_lines);
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

//for compilation purpose only
int main()
{
    init_cells();
    init_players();
    read_data("poles.txt");
    read_data("stairs.txt");
    read_data("walls.txt");
    printf("No errors so far!\n");
}