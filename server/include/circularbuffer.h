#ifndef _CIRCULARBUFFER_H_
#define _CIRCULARBUFFER_H_

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

int get_buf_entries(CIRCULAR_BUFFER *buf)
{
    return buf->entries;
}

void add_entry(CIRCULAR_BUFFER *buf, uint8_t state)
{
    // create new entry and initialize
    BUFFER_ENTRY *new_entry = NULL;
    new_entry = malloc(sizeof(BUFFER_ENTRY));
    new_entry->state = state;
    new_entry->next = NULL;

    // check if this will be the first entry
    if(buf->entries == 0)
    {
        buf->head = new_entry;
        buf->tail = new_entry;
    }
    else
    {
        // obtain the current last entry
        BUFFER_ENTRY *last_entry = NULL;
        last_entry = buf->tail;
        // last entry should point to the new entry
        last_entry->next = new_entry;
        // update the tail pointer to point to new_entry
        buf->tail = new_entry;
    }
    buf->entries++;

}

void print_buffer(CIRCULAR_BUFFER *buf)
{
    int i = 0;
    BUFFER_ENTRY *p = NULL;
    p = buf->head;

    printf("print: size of buffer: %d\n", buf->entries);
    while(p != NULL)
    {
        printf("entry %d: %d\n", i, p->state);
        i++;
        p = p->next;
    }
}

void clear_buffer(CIRCULAR_BUFFER *buf)
{
    int i = 0;
    BUFFER_ENTRY *p_next = NULL;
    BUFFER_ENTRY *p = NULL;
    p = buf->head;

    printf("clear: size of buffer: %d\n", buf->entries);

    while(p != NULL)
    {
        p_next = p->next;
        free(p);
        p = p_next;

        printf("removed entry %d\n", i);
        i++;
    }

    buf->head = NULL;
    buf->tail = NULL;
    buf->entries = 0;

}

void init_buffer(CIRCULAR_BUFFER *buf)
{
    buf->head = NULL;
    buf->tail = NULL;
    buf->entries = 0;
}

void test_buffer()
{
    CIRCULAR_BUFFER test_buf;
    init_buffer(&test_buf);
    add_entry(&test_buf, 4);
    add_entry(&test_buf, 214);
    add_entry(&test_buf, 132);
    add_entry(&test_buf, 94);
    print_buffer(&test_buf);
    clear_buffer(&test_buf);
    print_buffer(&test_buf);
    add_entry(&test_buf, 212);
    add_entry(&test_buf, 123);
    add_entry(&test_buf, 222);
    print_buffer(&test_buf);
    clear_buffer(&test_buf);

}

#endif // _CIRCULARBUFFER_H_
