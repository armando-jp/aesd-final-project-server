#ifndef _THREADS_H_
#define _THREADS_H_

#define MAXINBUF 100
#define MAXOUTBUF 100

void * listener_fnct (void *thread_args)
{
    THREAD_ARGS *args = (THREAD_ARGS *) thread_args;
    // allocate the buffer memory and initialize with zeros
    char in_buf[MAXINBUF];
    memset(in_buf, 0, MAXINBUF);
    int count = 0;
    int rv = 0;

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
            // printf("got something\n");
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

        printf("message received from %s: %s", args->s, in_buf);
        process_msg(in_buf, rv, args);
        memset(in_buf, 0, MAXINBUF);
        count = 0;
    }
    close(args->new_fd);
    exit(0);

}

void *led_fnct(void *thread_args)
{
    THREAD_ARGS *args = (THREAD_ARGS *) thread_args;
    int i = 0;
    char c;
    int fd;
    int entries = get_buf_entries(args->led_buf);
    printf("led thread: number of entries = %d\n", entries);
    fd = open(PATH, O_WRONLY);
    BUFFER_ENTRY *led_entry = NULL;
    led_entry = (args->led_buf->head);

    print_buffer(args->led_buf);

    // struct timeval tv = { .tv_sec = 1print_buffer(args->led_buf);,
    //                       .tv_usec = 0 };

    while(1)
    {
        // check here for termination signal
        pthread_mutex_lock(&args->mutex);
        printf("entry %d from thread = %d\n", i, (led_entry)->state);
        c = ((led_entry)->state) + '0';
        write(fd, &c, 1);
        pthread_mutex_unlock(&args->mutex);
        usleep(args->speed);

        i = (i + 1) % entries;
        if(led_entry->next == NULL)
        {
            led_entry = args->led_buf->head;
        }
        else
        {
            led_entry = led_entry->next;
        }

        // check for termination signal
        if((args->condition))
        {
            printf("led_thread: got termination signal, goodbye!\n");
            close(fd);
            pthread_exit(NULL);
        }

    }
}

#endif // _THREADS_H_
