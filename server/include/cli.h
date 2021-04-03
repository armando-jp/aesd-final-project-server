#ifndef _CLI_H_
#define _CLI_H_

#define NUM_CMDS 5

#define CMD_CLEAR 1
#define CMD_START 2
#define CMD_STOP 3
#define CMD_APPEND 4
#define CMD_SPEED 5

typedef struct _cli
{
    char *name;
    char *usage;
    uint16_t cmd;

} CLICMDS;

static const
CLICMDS cli[NUM_CMDS] =
{
    {"clear", "\tclear\r\n", CMD_CLEAR},
    {"start", "\tstart\r\n", CMD_START},
    {"stop", "\tstop\r\n", CMD_STOP},
    {"append", "\tappend\r\n", CMD_APPEND},
    {"speed", "\tspeed\r\n", CMD_SPEED}
};

const char *welcome_msg = "Welcome to LAN Lights";
const char *confirmation = "Message received";


#endif // _CLI_H_
