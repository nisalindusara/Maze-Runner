#include "game.h"

#define MAX_LINE_LEN 256

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

/*
    read input data, validate each line while updating the relevant cells
*/

typedef enum {INVALID_NUM_DIGITS, NO_DIGITS, FILE_NOT_OPEN, EMPTY_FILE, INVALID_FORMAT_LINE} FILE_ERRORS;

void print_file_error(FILE_ERRORS error, char filename[], FILE *logfile)
{
    switch(error)
    {
        case FILE_NOT_OPEN:fprintf(logfile, "[ERROR] Cannot open (file: %s)", filename);break;
        case EMPTY_FILE:fprintf(logfile, "[ERROR] File is empty (file: %s). No data to process.\n", filename);break;
        default: fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");break;
    }
}

void print_file_error_line(FILE_ERRORS error, char filename[], FILE *logfile, int line_number)
{
    switch (error)
    {
        case INVALID_NUM_DIGITS:fprintf(logfile, "[WARN] Invalid number of digits (file %s, line %d). Line skipped.\n", filename, line_number);break;
        case NO_DIGITS:fprintf(logfile, "[WARN] No digits found (file %s, line %d). Line skipped\n", filename, line_number);break;
        case INVALID_FORMAT_LINE:fprintf(logfile, "[WARN] Invalid format (file %s, line %d). Line skipped.\n", filename, line_number);break;
        default: fprintf(logfile, "[WARN] Unhandled file error detected. Execution may be unstable.\n");break;
    }
}

void check_digits(int* digits, int count)
{
    switch(count)
    {
        case 6:
            printf("Will load stairs\n");
            //load_stairs()
            break;
        case 5:
            printf("Will load walls\n");
            //load_walls()
            break;
        case 4:
            printf("Will load poles\n");
            //load_poles()
            break;
        default:
            printf("Need to handle the error of invalid number of digits in a line\n");
            //handle invalid number of digits
    }
}

int parse_and_process_line(const char* line)
{
    const char* p =  line;
    
    while(isspace(*p)) p++;     //Removes whitespaces at the start of the line
    
    if(*p != '[') return 0;     //Must start with '['
    
    p++;
    
    int count = 0;
    int digits_array[100];
    
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
        
        if(count >= 100) return 0;
        digits_array[count++] = num;
        
        while(isspace(*p)) p++;
        
        if(*p == ',') p++;
        else if(*p != ']') return 0;    //Must close with ']'
    }

    if(*p != ']') return 0;
    
    p++;
    
    while(isspace(*p)) p++;
    
    if(*p != '\0') return 0;

    if(count > 0)
    {
        check_digits(digits_array, count);
    }
    else
    {
        printf("Hanlde the error of no digits within the []\n");
        //handle no digit error
    }
    
    return 1;
}

int read_data(char filename[])
{
    FILE* logf = fopen(filename, "a");
    if(logf == NULL)    //File not opening error for log file
    {
        perror("log.txt file can't be opened.\nThe game will try to continue without log.txt");
    }
    FILE* dataf = fopen(filename, "r");
    if(dataf == NULL)   //File not opening error for the filename
    {
        fprintf(logf, "The %s file can't be opened.\n", filename);
        exit(1);
    }

    int ch = fgetc(dataf);      //check if the file is empty
    if (ch == EOF) 
    {
        fprintf(logf, "The %s file is empty. No data to read.\n", filename);
        exit(1);
    }

    rewind(dataf);   //Reset file pointer to back to start

    char buffer[1024];  //to hold a each line read from the file
    int line_num = 1;

    while(fgets(buffer, sizeof(buffer), dataf))     //reads one line from dataf to the buffer at most sizeof(buffer)-1(for null termination), loop continues until fgets() returns Null which means EOF or error
    {
        buffer[strcspn(buffer, "\n")] = '\0';       //finds the index of the first '\n' in the line and replace it with '\0'. Removes the trailing new line from the line so "123\n" becomes "123"
        if(!parse_and_process_line(buffer))
        {
            printf("Invalidly formatted line printed\n");
            //handle invalid line
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
{}