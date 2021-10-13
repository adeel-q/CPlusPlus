#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define QUEUE_MAX_SIZE 4

typedef struct queue_struct
{
    size_t length;
    int data[QUEUE_MAX_SIZE];
    int* head;
    int* tail;
} queue_t;

void initQueue(queue_t * queue)
{
    queue->length = 0;
    queue->head = &queue->data[0];
    queue->tail = &queue->data[0];
    for (int i = 0; i < QUEUE_MAX_SIZE; i++)
    {
        queue->data[i] = 0x00;
    }
}

// Return -1 if queue full
int put(queue_t * queue, int element)
{
    if (queue->length == QUEUE_MAX_SIZE)
    {
        printf("queue full!\n");
        return -1;
    }
    // Write to tail ptr
    *(queue->tail) = element;
    // Incrememnt length
    queue->length += 1;
    // Increment write tail or wrap around
    if (queue->tail == &queue->data[QUEUE_MAX_SIZE-1])
    {
        queue->tail = &queue->data[0];
        printf("writing last slot, wrapping around\n");
    }
    else
    {
        queue->tail++;
    }
    return 1;
}


// Always writes
int put_circ(queue_t * queue, int element)
{
    if (queue->length == QUEUE_MAX_SIZE)
    {
        // return -1;
        printf("overwritting...\n");
    }
    // Write to tail ptr
    *(queue->tail) = element;
    // Incrememnt length
    queue->length += 1;
    // Increment write tail or wrap around
    if (queue->tail == &queue->data[QUEUE_MAX_SIZE-1])
    {
        queue->tail = &queue->data[0];
    }
    else
    {
        queue->tail++;
    }
    return 1;
}

int get(queue_t * queue)
{
    if (queue->length == 0)
    {
        printf("nothing to read\n!");
        return 0;
    }

    int retval = 0;
    // Read 
    retval = *(queue->head);
    printf("pop %d\n", retval);
    // Decrement length
    queue->length -=1;
    // Incrememnt read head and wrap around
    if (queue->head == &queue->data[QUEUE_MAX_SIZE-1])
    {
        printf("read head back to 0\n");
        queue->head = &queue->data[0];
    }
    else
    {
        queue->head++; // Seek to last element
    }
    return retval;
}

// Print the queue data from head to tail
void printQueue(queue_t * q)
{
    printf("length=%d\n", q->length);
    printf("data [ ");
    for (int i = 0; i < QUEUE_MAX_SIZE; i++)
    {
        if ((&q->data[i] == q->tail) && ((&q->data[i] == q->head)))
        {
            printf("head\\tail(%d) ", q->data[i]);
        }
        else if (&q->data[i] == q->head)
        {
            printf("head(%d) ", q->data[i]);
        }
        else if (&q->data[i] == q->tail)
        {
            printf("tail(%d) ", q->data[i]);
        }
        else 
        {
            printf("%d, ", q->data[i]);
        }
    }
    printf(" ] \n\n");
}

int main(void)
{
    queue_t q;
    initQueue(&q);

    put(&q,-1);
    printQueue(&q);
    
    put(&q,1);
    printQueue(&q);

    put(&q,2);
    printQueue(&q);

    put(&q,3);
    printQueue(&q);

    
    put(&q,4);
    printQueue(&q);

    put_circ(&q, 4);
    printQueue(&q);

    int var;
    var = get(&q);
    printQueue(&q);

    var = get(&q);
    printQueue(&q);

    var = get(&q);
    printQueue(&q);

    put_circ(&q, 5);
    printQueue(&q);

    return 0;
}