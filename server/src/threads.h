#ifndef _THREADS_H_
#define _THREADS_H_

#define MAXINBUF 100
#define MAXOUTBUF 100

void * listener_fnct (void *thread_args)
{
    THREAD_ARGS *args = (THREAD_ARGS *) thread_args;
    // allocate the buffer memory and initialize with zeros
    char in_buf[MAXINBUF];
    char out_buf[MAXOUTBUF];
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
            PDEBUG("\nwaiting for message...\n");
            rv = recv(args->new_fd, in_buf, MAXINBUF, 0);
            if(rv == -1)
            {
                perror("recv");
                pthread_exit(NULL);
            }
            if(rv == 0)
            {
                PDEBUG("Remote client has closed connection\n\n");
                pthread_exit(NULL);
            }
            count += rv;
        } while(!newline_found(in_buf, count));

        PDEBUG("message received from %s: %s", args->s, in_buf);
        process_msg(in_buf, out_buf, rv, args);
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
    args->thread_is_running = 1;
    PDEBUG("led thread: number of entries = %d\n", entries);
    fd = open(PATH, O_WRONLY);
    BUFFER_ENTRY *led_entry = NULL;
    led_entry = (args->led_buf->head);

    print_buffer(args->led_buf);

    while(1)
    {
        // check here for termination signal
        pthread_mutex_lock(&args->mutex);
        PDEBUG("entry %d from thread = %d\n", i, (led_entry)->state);
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
            PDEBUG("led_thread: got termination signal, goodbye!\n");
            args->thread_is_running = 0;
            close(fd);
            pthread_exit(NULL);
        }

    }
}

#endif // _THREADS_H_
