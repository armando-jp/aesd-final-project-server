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

#define PORT "3490"
#define MAXOUTBUF 100
#define MAXINBUF 100

#define MAXDATASIZE 100

#include "utils.h"
#include "cli.h"

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

void *listener_fnct(void *sockfd)
{
    int *sockfd_arg = (int*)sockfd;
    // allocate the buffer memory and initialize with zeros
    char in_buf[MAXINBUF];
    memset(in_buf, 0, MAXINBUF);
    int count = 0;
    int rv = 0;
    int i = 0;

    while(1)
    {
        do
        {
            rv = recv(*sockfd_arg, in_buf, MAXINBUF, 0);
            if(rv == -1)
            {
                perror("recv");
                pthread_exit(NULL);
            }
            if(rv == 0)
            {
                printf("Remote client has closed connection\n\n");
                pthread_exit(NULL);
            }
            count += rv;
        } while(!newline_found(in_buf, count));

        printf("LED States\n");
        while(*(in_buf+i) != '\n')
        {
            printf("State %d = %c\n", i, *(in_buf+i));
            i++;
        }

        //printf("message received from %d: %s", *sockfd_arg, in_buf);
        memset(in_buf, 0, MAXINBUF);
        count = 0;
        i = 0;
    }
    // should never get to this point
    close(*sockfd_arg);
    exit(0);

}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    char in_buf[MAXINBUF];
    char out_buf[MAXOUTBUF];
    //char raw_buf[MAXOUTBUF];, 0, MAXINBUF);
    int rv = 0;

    struct addrinfo hints, *servinfo, *p;
    char s[INET6_ADDRSTRLEN];

    pthread_t listener_thread;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client [hostname/ip]\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //initialize a linked list of addrinfo structures (servinfo),
    //for each network address that matches node and service args.
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0)
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
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if(p == NULL)
    {
        fprintf(stderr, "client failed to connect");
        exit(2);
    }

    // network to presentation conversion. store result string in s.
    inet_ntop(p->ai_family,
        get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // we are finished with this structure

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
    {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n", buf);

    // create a pthread for receiving messages and displaying to user
    pthread_create( &listener_thread, NULL, listener_fnct, (void*) &sockfd);


    // this process will send commands to server
    while(1)
    {
        printf("> ");
        fgets(in_buf, MAXOUTBUF, stdin);
        process_cmd(in_buf, out_buf);

        //printf("sending: %s of size %ld\n", out_buf, strlen(out_buf));
        // for(int i = 0; i < strlen(out_buf); i++)
        // {
        //     printf("%d\n", *(out_buf+i));
        // }
        if (send(sockfd, out_buf, strlen(out_buf), 0) == -1)
        {
            perror("send");
        }

        PDEBUG("sending message complete\n");
        memset(out_buf, 0, MAXOUTBUF);
        memset(in_buf, 0, MAXOUTBUF);
    }

    close(sockfd);

    // never executes
    return 0;
}
