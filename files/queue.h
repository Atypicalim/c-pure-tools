// queue

#ifndef H_PCT_QUEUE
#define H_PCT_QUEUE

#include "block.h" // [M[ IGNORE ]M]
#include "cursor.h" // [M[ IGNORE ]M]
#include "chain.h" // [M[ IGNORE ]M]

typedef struct _Queue {
    struct _Chain;
} Queue;

Queue *Queue_new(bool isRetain) {
    return (Queue *)_Chain_new(isRetain, sizeof(Queue), PCT_OBJ_QUEUE);
}

void Queue_print(Queue *this) {
    _Chain_print((Chain *)this, "QUEUE");
}

void Queue_push(Queue *this, void *data) {
    Chain_push_to_tail((Chain *)this, data);
}

void *Queue_pop(Queue *this) {
    return Chain_pop_from_head((Chain *)this);
}

void Queue_clear(Queue *this) {
    Chain_clear_from_head((Chain *)this);
}

void Queue_free(Queue *this) {
    Chain_free_from_head((Chain *)this);
}

void Queue_RESTE(Queue *this) {
    Chain_RESTE_TO_HEAD((Chain *)this);
}

void *Queue_NEXT(Queue *this) {
    return Chain_NEXT((Chain *)this);
}

void *Queue_LAST(Queue *this) {
    return Chain_LAST((Chain *)this);
}

Cursor *Queue_reset(Queue *this) {
    return Chain_reset_to_head((Chain *)this);
}

void *Queue_next(Queue *this, Cursor *cursor) {
    return Chain_next((Chain *)this, cursor);
}

void *Queue_last(Queue *this, Cursor *cursor) {
    return Chain_last((Chain *)this, cursor);
}

Queue *Queue_clone(Queue *this) {
    Queue *queue = Queue_new(this->retain);
    _Chain_sync((Chain *)this, (Chain *)queue);
    return queue;
}

void Queue_reverse(Queue *this) {
    Chain_reverse((Chain *)this);
}

#define QUEUE_FOREACH_FUNC CHAIN_FOREACH_FUNC

void Queue_foreachItem(Queue *this, QUEUE_FOREACH_FUNC func, void *arg) {
    Chain_foreach_from_head((Chain *)this, func, arg);
}

#endif
