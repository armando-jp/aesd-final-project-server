#ifndef _BUFFER_TYPE_H_
#define _BUFFER_TYPE_H_

#define SERVER_DEBUG //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef SERVER_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "custom application: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

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
    int thread_is_running;
    CIRCULAR_BUFFER *led_buf;
    uint32_t speed;

} THREAD_ARGS;

#endif // _BUFFER_TYPE_H_
