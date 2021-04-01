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

#include "gpio.h"
#include "cli.h"

#define PORT "3490"

#define BACKLOG 10 //how many pending connections the queue will hold

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

        // network to presentation conversion. store result string in s.
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

        printf("server: got connection from %s", s);

        // this is the child process
        if(!fork())
        {
            // child does not need the listener
            close(sockfd);
            if (send(new_fd, welcome_msg, strlen(welcome_msg), 0) == -1)
            {
                perror("send");
            }
            // TODO: Implement while loop which waits for commands.
            // while(1)
            // {
            //     //
            // }
            close(new_fd);
            exit(0);
        }
    }

    // never executes
    return 0;
}
