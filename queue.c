#include <assert.h>
#include <stdlib.h>

#include "queue.h"

void
enqueue(Queue* q, Thread* t)
{
    assert(t);
    assert(q);

    if (q->head)  // append to end
    {
        assert(q->last);
        q->last->next = t;
    }
    else  // set t as head
    {
        assert(!q->last);
        q->head = t;
    }

    q->last = t;
}

Thread*
dequeue(Queue* q)
{
    assert(q);
    Thread* r = q->head;
    q->head = q->head->next;
    if (!q->head) q->last = NULL;
    return r;
}
