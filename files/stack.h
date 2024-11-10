// stack

#ifndef H_PCT_STACK
#define H_PCT_STACK

#include "block.h" // [M[ IGNORE ]M]
#include "cursor.h" // [M[ IGNORE ]M]
#include "chain.h" // [M[ IGNORE ]M]

typedef struct _Stack {
    struct _Chain;
} Stack;

Stack *Stack_new(bool isRetain) {
    return (Stack *)_Chain_new(isRetain, sizeof(Stack), PCT_OBJ_STACK);
}

void Stack_print(Stack *this) {
    _Chain_print((Chain *)this, "STACK");
}

void Stack_push(Stack *this, void *data) {
    Chain_push_to_tail((Chain *)this, data);
}

void *Stack_pop(Stack *this) {
    return Chain_pop_from_tail((Chain *)this);
}

void Stack_clear(Stack *this) {
    Chain_clear_from_tail((Chain *)this);
}

void Stack_free(Stack *this) {
    Chain_free_from_tail((Chain *)this);
}

void Stack_RESTE(Stack *this) {
    Chain_RESTE_TO_TAIL((Chain *)this);
}

void *Stack_NEXT(Stack *this) {
    return Chain_LAST((Chain *)this);
}

void *Stack_LAST(Stack *this) {
    return Chain_NEXT((Chain *)this);
}

Cursor *Stack_reset(Stack *this) {
    return Chain_reset_to_tail((Chain *)this);
}

void *Stack_next(Stack *this, Cursor *cursor) {
    return Chain_last((Chain *)this, cursor);
}

void *Stack_last(Stack *this, Cursor *cursor) {
    return Chain_next((Chain *)this, cursor);
}

Stack *Stack_clone(Stack *this) {
    Stack *stack = Stack_new(this->retain);
    _Chain_sync((Chain *)this, (Chain *)stack);
    return stack;
}

void Stack_reverse(Stack *this) {
    Chain_reverse((Chain *)this);
}

#define STACK_FOREACH_FUNC CHAIN_FOREACH_FUNC

void Stack_foreachItem(Stack *this, STACK_FOREACH_FUNC func, void *arg) {
    Chain_foreach_from_tail((Chain *)this, func, arg);
}

#endif
