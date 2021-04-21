#ifndef _UTILS_H_
#define _UTILS_H_

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// scans the input buffer for '\n' terminator
int newline_found(void* buf, int len)
{
    char *p = (char *) buf;
    for(; p != NULL; p++)
    {
        //PDEBUG("%c\n", *p);
        if(*p == '\n')
        {
            return 1;
        }
    }
    return 0;
}

// scans the buffer and determines if it is a string binary representation
// ex. buf = '100110011' returns 1
// ex. buf = '110022001' returns 0
int is_binary_str(char *buf, int size)
{
    int i = 0;
    while(i < size)
    {
        if( (*(buf+i) != '0' && *(buf+i) != '1') )
        {
            if( (*(buf+i)) == '\n')
            {
                return 1;
            }

            return 0;
        }
        i++;
    }
    return 0;
}

int create_daemon(int argc, char *argv[], int arg_sockfd)
{
    // check if the first argument matches -d.
    // IF true, fork the process
    //      The parent process will receive the new child PID and should exit().
    //      The child shall continue execution as normal.
    // return 0 if daemon created successfully, -1 otherwise.
    int rv;
    if ( argc > 1 )
    {
        for (int i = 0; i < argc; i++)
        {
            PDEBUG("got the argument: %s\n", argv[i]);
        }

        if( strcmp(argv[1], "-d") == 0 )
        {
            pid_t pid;
            pid = fork();

            // an error occured
            if(pid == -1)
            {
                PDEBUG("Daemonization failed!\n");
                return -1;
            }

            // success: let the parent terminate
            if(pid != 0)
            {
                exit(EXIT_SUCCESS);
            }

            // create new session and process group; child becomes leader
            if (setsid() == -1)
            {
                perror("setsid");
                return -1;
            }

            // fork a second time
            pid = fork();

            // an error occured
            if(pid == -1)
            {
                PDEBUG("Daemonization failed!\n");
                return -1;
            }

            // success: let the parent terminate
            if(pid != 0)
            {
                exit(EXIT_SUCCESS);
            }

            // set new file permissions
            umask(0);

            // set the working directory to the root directory
            if ( chdir("/") == -1)
            {
                perror("chdir");
                return -1;
            }

            //close all open files
            for (int i =  sysconf(_SC_OPEN_MAX); i >= 0; i--)
            {
                if(i == arg_sockfd)
                    continue;
                close(i);
            }

            //openlog("aesdsocket", LOG_NDELAY, LOG_DAEMON);
            //syslog(LOG_DEBUG, "Process is now a daemon\n");
            //PDEBUG("Process is now a daemon!\n");
            //openlog("aesdsocket")

            // redirect fd's 0, 1, 2 to /dev/null
            open ("/dev/null", O_RDWR); // stdin
            rv = dup(0);                     // stdout
            if(rv == -1)
            {
                perror("dup");
            }
            rv = dup(0);                     // stderror
            if(rv == -1)
            {
                perror("dup");
            }

        }
        else
        {
            return -1;
        }

        return 0;
    }
    return -1;

}

#endif // _UTILS_H_
