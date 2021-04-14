#ifndef _BUFFER_TYPE_H_
#define _BUFFER_TYPE_H_

typedef struct _entry
{
    uint8_t state;
    struct _entry *next;
} BUFFER_ENTRY;

typedef struct _circular_buffer
{
    BUFFER_ENTRY *head;
    BUFFER_ENTRY *tail;
    int entries;

} CIRCULAR_BUFFER;

typedef struct _thread_args
{
    int new_fd;
    pthread_t led_thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    const char *test_txt; // test string
    char *s; // server IP address
    int condition;
    CIRCULAR_BUFFER *led_buf;

} THREAD_ARGS;

#endif // _BUFFER_TYPE_H_
