#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#include "buffer_type.h"
#include "gpio.h"
#include "utils.h"
#include "circularbuffer.h"
#include "cli.h"
#include "threads.h"

#define PORT "3490"

#define BACKLOG 10 //how many pending connections the queue will hold


int main(int argc, char *argv[])
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
    // int iret1;
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

    // check if the daemon option was requested
    create_daemon(argc, argv, sockfd);

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

    PDEBUG("server waiting for connections...\n");
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

        PDEBUG("server: got connection from %s\n", s);

        // create a listener and sender thread
        pthread_create( &listener_thread, NULL, listener_fnct, (void*) &thread_args);

        pthread_join(listener_thread, NULL);
    }

    // never executes
    return 0;
}
