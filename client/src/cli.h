#ifndef _CLI_H_
#define _CLI_H_

#define NUM_CMDS 6

#define CMD_CLEAR 1
#define CMD_START 2
#define CMD_STOP 3
#define CMD_APPEND 4
#define CMD_SPEED 5
#define CMD_PRINT 6

extern void *led_fnct(void *thread_args);

typedef struct _cli
{
    char *name;
    char *usage;
    uint16_t cmd;

} CLICMDS;

static const
CLICMDS cli_cmds[NUM_CMDS] =
{
    {"clear", "\tclear\r\n", CMD_CLEAR}, //C
    {"start", "\tstart\r\n", CMD_START}, //S
    {"stop", "\tstop\r\n", CMD_STOP},    //T
    {"append", "\tappend [binary string]\r\n", CMD_APPEND}, //A
    {"speed", "\tspeed [useconds]\r\n", CMD_SPEED},     //D
    {"print", "\tprint\r\n", CMD_PRINT},
};

// returns the command number specified by the buffer by looking at the first character
// if the command is invalid, -1 is returned.
int get_msg_type(char *buf, int size)
{
    if(buf == NULL)
    {
        return -1;
    }

    for(int i = 0; i < NUM_CMDS; i++)
    {
        //printf("cmd = %s\t buf = %s\n", cli_cmds[i].name, buf);
        if( strcmp((cli_cmds[i].name), buf) == 0 )
        {
            return  i+1;
        }
    }

    return -1;
}

// scans buffer for newline, if found, replaces with null terminator
// return 0 if no newline was found, returns 1 if newline was found and replaced
// returns -1 if error with buffer argument
int replace_newline(char *buf, int size)
{
    char *p = NULL;
    if(buf == NULL)
    {
        return -1;
    }
    p = buf;
    for(int i = 0; i < size; i++)
    {
        if(*(p+i) == '\n')
        {
            *(p+i) = 0;
            return 1;
        }
    }

    return 0;


}

void print_menu()
{
    int i = 0;
    for(; i < NUM_CMDS; i++)
    {
        printf("%d. %s", i+1, cli_cmds[i].usage);
    }
}

//
void process_cmd(char* in_buf, char* out_buf)
{
    int rv = -1;
    int i = 0;
    char *tok1 = NULL;
    char *tok2 = NULL;
    const char delim[2] = " ";

    if(in_buf == NULL)
    {
        return;
    }
    memset(out_buf, 0, MAXOUTBUF);

    tok1 = strtok(in_buf, delim); // get first token
    tok2 = strtok(NULL, delim); // get second token

    //printf("tok1 = %s\t tok2 = %s\n", tok1, tok2);

    replace_newline(tok1, strlen(tok1)); // remove newline from token if necessary
    rv = get_msg_type(tok1, strlen(tok1)); // get msg type

    switch(rv)
    {
        case CMD_START:
            printf("Got START!\n");
            out_buf[0] = 'S';
            out_buf[1] = '\n';
            break;

        case CMD_CLEAR:
            printf("Got CLEAR!\n");
            out_buf[0] = 'C';
            out_buf[1] = '\n';
            break;

        case CMD_APPEND:
            printf("Got APPEND!\n");
            printf("WITH ARG = %s\n", tok2);

            //build payload
            out_buf[0] = 'A';
            for(; i < strlen(tok2); i++)
            {
                out_buf[i+1] = *(tok2+i);
            }

            break;

        case CMD_STOP:
            printf("Got STOP!\n");
            out_buf[0] = 'T';
            out_buf[1] = '\n';
            break;

        case CMD_SPEED:
            printf("Got SPEED!\n");
            printf("WITH ARG = %s\n", tok2);

            //build payload
            out_buf[0] = 'D';
            for(; i < strlen(tok2); i++)
            {
                out_buf[i+1] = *(tok2+i);
            }

            break;

        case CMD_PRINT:
            printf("Got PRINT!\n");
            out_buf[0] = 'P';
            out_buf[1] = '\n';
            break;

        default:
            printf("invalid command\n");
            print_menu();
            break;
    }
    // printf("command found: %d\n", cmd);
    // printf("buffer contains: %s\n", buf);
    // printf("buffer size: %d\n", size);

}
#endif // _CLI_H_
