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
    {"C", "\tclear\r\n", CMD_CLEAR}, //C
    {"S", "\tstart\r\n", CMD_START}, //S
    {"T", "\tstop\r\n", CMD_STOP},    //T
    {"A", "\tappend\r\n", CMD_APPEND}, //A
    {"D", "\tspeed\r\n", CMD_SPEED},     //D
    {"P", "\tprint\r\n", CMD_PRINT},
};

const char *welcome_msg = "Welcome to LAN Lights";
const char *confirmation = "Message received";

// returns the command number specified by the buffer by looking at the first character
// if the command is invalid, -1 is returned.
int get_msg_type(char *buf, int size)
{
    char msg_type;
    if(buf == NULL)
    {
        return -1;
    }

    msg_type = *buf; // get the first character
    for(int i = 0; i < NUM_CMDS; i++)
    {
        if(msg_type == *(cli_cmds[i].name))
            return  i;
    }

    return -1;
}

int process_msg(char *in_buf, int size, THREAD_ARGS *args)
{
    int rv = -1;
    int payload_size;
    char *payload = NULL;

    // check that input buffer is not empty
    if(in_buf == NULL)
    {
        return -1;
    }

    // find the command number
    for(int i = 0; i < NUM_CMDS; i++)
    {
        if(in_buf[0] == *(cli_cmds[i].name))
        {
            rv = i+1;
            break;
        }
    }

    // printf("rv: %d\n", rv);
    switch(rv)
    {
        case CMD_CLEAR:
            // stop the LED pthread
            // empty the contents of the LED cbuffer
            clear_buffer(args->led_buf);
            printf("Buffer content cleared\n");
            break;

        case CMD_START:
            // check if the LED buffer is empty, if not empty then...
            // spawn a new pthread LED_RUN which handles driving the LED
    pthread_mutex_lock(&args->mutex);
            if(get_buf_entries(args->led_buf))
            {
                printf("starting LED\n");
                pthread_create( &(args->led_thread_id), NULL, led_fnct, (void*) args);
            }
            else
            {
                printf("cmd buf is empty, not going to start LED\n");
            }
    pthread_mutex_unlock(&args->mutex);
            break;

        case CMD_STOP:
            // stop the LED pthread
            printf("stopping LED\n");
            (args->condition) = 1;
            pthread_join(args->led_thread_id, NULL);
            (args->condition) = 0;
            printf("stopping LED success\n");
            break;

        case CMD_APPEND:
            // append the desired LED states to the current LED buffer.
            // note that this LED buffer is not the one being read directly from by the
            // pthread LED_RUN. LED RUN has a copy of the buffer that is generated
            // when spawned.

    pthread_mutex_lock(&args->mutex);
            // get payload
            payload = in_buf+1; // get pointer to 2nd element in buffer
            payload_size = size-1; // subtract one because we removed the first byte

            if(!is_binary_str(payload, payload_size))
            {
                pthread_mutex_unlock(&args->mutex);
                printf("message payload contains invaild binary string");
                break;
            }
            // do loop to insert payload into buffer
            for(int i = 0; i < payload_size-1; i++) // subtract one because of '\n'
            {
                if(*(payload+i) == '0')
                    add_entry(args->led_buf, 0);
                else
                    add_entry(args->led_buf, 1);
            }
    pthread_mutex_unlock(&args->mutex);
            printf("Content appended\n");
            break;

        case CMD_SPEED:
    pthread_mutex_lock(&args->mutex);
            // update the speed at which the LED state changes in the pthread LED_RUN
            payload = in_buf+1; // get pointer to 2nd element in buffer
            payload_size = size-1; // subtract one because we removed the first byte

            args->speed = strtol(payload, NULL, 10);
            printf("updating LED speed to %d\n", args->speed);
    pthread_mutex_unlock(&args->mutex);
            break;

        case CMD_PRINT:
            print_buffer(args->led_buf);
            break;

        default:
            printf("invalid command found\n");
            return -1;
    }
    return 0;

}

#endif // _CLI_H_
