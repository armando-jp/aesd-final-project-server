#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#include "gpio.h"
#include "cli.h"
#include "circularbuffer.h"

#define PORT "3490"
#define MAXINBUF 100
#define MAXOUTBUF 100

#define BACKLOG 10 //how many pending connections the queue will hold

typedef struct _thread_args
{
    int new_fd;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    const char *test_txt; // test string
    char *s; // server IP address
    int condition;
    CIRCULAR_BUFFER *led_buf;

} THREAD_ARGS;

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
        //printf("%c\n", *p);
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

void * listener_fnct (void *thread_args)
{
    THREAD_ARGS *args = (THREAD_ARGS *) thread_args;

    // allocate the buffer memory and initialize with zeros
    char in_buf[MAXINBUF];
    memset(in_buf, 0, MAXINBUF);
    int count = 0;
    int rv = 0;

    printf("test message: %s\n", args->test_txt);
    if (send(args->new_fd, args->test_txt, strlen(args->test_txt), 0) == -1)
    {
        perror("send");
    }

    while(1)
    {
        do
        {
            printf("\nwaiting for message...\n");
            rv = recv(args->new_fd, in_buf, MAXINBUF, 0);
            printf("got something\n");
            if(rv == -1)
            {
                perror("recv");
                exit(1);
            }
            if(rv == 0)
            {
                printf("Remote client has closed connection\n");
                exit(1);
            }
            count += rv;
        } while(!newline_found(in_buf, count));

        rv = is_binary_str(in_buf, count);
        if(rv)
        {
            char *p = in_buf;

            // lock access because we are accessing shared data
            pthread_mutex_lock(&args->mutex);
            // add entries to buffer
            for(int i = 0; i < count-1; i++) // subtract one because of '\n'
            {
                if(*(p+i) == '0')
                    add_entry(args->led_buf, 0);
                else
                    add_entry(args->led_buf, 1);
            }
            pthread_mutex_unlock(&args->mutex);
            //printf("message added to buf\n");
            print_buffer(args->led_buf);
        }
        else
        {
            printf("message not a valid binary string\n");
        }

        //printf("newline was found!\n");
        printf("message received from %s: %s", args->s, in_buf);
        memset(in_buf, 0, MAXINBUF);
        count = 0;
    }
    close(args->new_fd);
    exit(0);

}

void * sender_fnct (void *thread_args)
{
    THREAD_ARGS *args = (THREAD_ARGS *) thread_args;

    // allocate the buffer memory and initialize with zeros
    char out_buf[MAXOUTBUF];
    memset(out_buf, 0, MAXOUTBUF);
    // int count = 0;
    // int rv = 0;

    while(1)
    {
        // wait for flag to change
        pthread_mutex_lock(&args->mutex);
        printf("sender: waiting for listener thread\n");
        while(!args->condition)
        {
            printf("sender: waiting for listener thread\n");
            if(pthread_cond_wait(&args->cond, &args->mutex) != 0)
            {
                perror("pthread wait:");
            }
        }

        printf("sending: %s", confirmation);
        if (send(args->new_fd, confirmation, strlen(confirmation), 0) == -1)
        {
            perror("send");
        }
        args->condition = 0;
        pthread_mutex_unlock(&args->mutex);
        memset(out_buf, 0, MAXOUTBUF);
    }
    close(args->new_fd);
    exit(0);

}

int main(void)
{
    int sockfd, new_fd; // listen on sockfd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    //struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    // pthread related variables
    pthread_t listener_thread;
    // pthread_t sender_thread;
    int iret1;
    // int iret2;

    // create the main LED buffer
    CIRCULAR_BUFFER led_buf;
    init_buffer(&led_buf);

    // initialize the shared thread variable
    THREAD_ARGS thread_args;
    memset(&thread_args, 0, sizeof(THREAD_ARGS));
    pthread_mutex_init(&thread_args.mutex, NULL);
    pthread_cond_init(&thread_args.cond, NULL);
    thread_args.test_txt = welcome_msg;
    thread_args.condition = 0;
    thread_args.led_buf = &led_buf;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use own IP

    //initialize a linked list of addrinfo structures (servinfo),
    //for each network address that matches node and service args.
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        // create an end point for communication
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        // allow re-use of address if previously left connected
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
            sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        // associate the address to a file descriptor, sockfd
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // we are finished with this structure

    if(p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // wait for incoming connections
    if(listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("server waiting for connections...\n");
    while(1)
    {

        // store the size of the data structure used to store the caller's info
        sin_size = sizeof their_addr;
        // connect to pending request
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if(new_fd == -1)
        {
            perror("accept");
            continue;
        }
        thread_args.new_fd = new_fd;

        // network to presentation conversion. store result string in s.
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        thread_args.s = s;

        printf("server: got connection from %s\n", s);

        // create a listener and sender thread
        iret1 = pthread_create( &listener_thread, NULL, listener_fnct, (void*) &thread_args);
        printf("iret1: %d\n", iret1);
        // iret2 = pthread_create( &sender_thread, NULL, sender_fnct, (void*) &thread_args);
        // printf("iret2: %d\n", iret2);

        pthread_join(listener_thread, NULL);
        // pthread_join(sender_thread, NULL);

        // this is the child process
        // if(!fork())
        // {
        //     // allocate the buffer memory and initialize with zeros
        //     char in_buf[MAXINBUF];
        //     memset(in_buf, 0, MAXINBUF);
        //     int count = 0;
        //     rv = 0;
        //
        //     // child does not need the listener
        //     close(sockfd);
        //     if (send(new_fd, welcome_msg, strlen(welcome_msg), 0) == -1)
        //     {
        //         perror("send");
        //     }
        //
        //     // main program loop
        //     while(1)
        //     {
        //         do
        //         {
        //             printf("\nwaiting for message...\n");
        //             rv = recv(new_fd, in_buf, MAXINBUF, 0);
        //             if(rv == -1)
        //             {
        //                 perror("recv");
        //                 exit(1);
        //             }
        //             if(rv == 0)
        //             {
        //                 printf("Remote client has closed connection\n");
        //                 exit(1);
        //             }
        //             count += rv;
        //         } while(!newline_found(in_buf, count));
        //
        //         //printf("newline was found!\n");
        //         printf("message received from %s: %s", s, in_buf);
        //         memset(in_buf, 0, MAXINBUF);
        //     }
        //     close(new_fd);
        //     exit(0);
        // }
    }

    // never executes
    return 0;
}
