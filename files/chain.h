// chain

#ifndef H_PCT_CHAIN
#define H_PCT_CHAIN

#include "block.h" // [M[ IGNORE ]M]
#include "cursor.h" // [M[ IGNORE ]M]

typedef struct _Chain {
    struct _Object;
    int size;
    Block *head;
    Block *tail;
    Cursor *cursor;
    bool retain;
} Chain;

Chain *_Chain_new(bool isRetain, int size, char typ) {
    Chain *chain = (Chain *)pct_mallloc(size);
    Object_init(chain, typ);
    chain->size = 0;
    chain->head = NULL;
    chain->tail = NULL;
    chain->cursor = Cursor_new(NULL);
    chain->retain = isRetain;
    return chain;
}

Chain *Chain_new(bool isRetain) {
    return _Chain_new(isRetain, sizeof(Chain), PCT_OBJ_CHAIN);
}

void _Chain_print(Chain *this, char *flag)
{
    printf("[(%s_START) => address:%d]\n", flag, this);
    Block *current = this->head;
    while (current != NULL)
    {
        Block_print(current);
        current = current->next;
    }
    printf("[(%s_END) => address:%d]\n", flag, this);
}

void Chain_print(Chain *this) {
    _Chain_print(this, "CHAIN");
}

void Chain_push_to_head(Chain *this, void *data)
{
    this->size++;
    if (this->retain) Object_retain(data);
    Block *block = Block_new(data);
    if (this->head != NULL) {
        Block_prepend(this->head, block);
    } else {
        this->tail = block;
    }
    this->head = block;
}

void Chain_push_to_tail(Chain *this, void *data)
{
    this->size++;
    if (this->retain) Object_retain(data);
    Block *block = Block_new(data);
    if (this->tail != NULL) {
        Block_append(this->tail, block);
    } else {
        this->head = block;
    }
    this->tail = block;
}

void *Chain_pop_from_head(Chain *this)
{
    if (this->head == NULL)
    {
        this->size = 0;
        return NULL;
    }
    else
    {
        void *data = this->head->data;
        Block *head = this->head;
        if (this->head == this->tail)
        {
            this->size = 0;
            this->head = NULL;
            this->tail = NULL;
        }
        else
        {
            this->size--;
            this->head = this->head->next;
            this->head->last = NULL;
        }
        if (this->retain) Object_release(head->data);
        Object_release(head);
        return data;
    }
}

void *Chain_pop_from_tail(Chain *this)
{
    if (this->tail == NULL)
    {
        this->size = 0;
        return NULL;
    }
    else
    {
        void *data = this->tail->data;
        Block *tail = this->tail;
        if (this->tail == this->head)
        {
            this->size = 0;
            this->head = NULL;
            this->tail = NULL;
        }
        else
        {
            this->size--;
            this->tail = this->tail->last;
            this->tail->next = NULL;
        }
        if (this->retain) Object_release(tail->data);
        Object_release(tail);
        return data;
    }
}

void Chain_clear_from_head(Chain *this)
{
    void *data = Chain_pop_from_head(this);
    while (data != NULL)
    {
        data = Chain_pop_from_head(this);
    }
}

void Chain_clear_from_tail(Chain *this)
{
    void *data = Chain_pop_from_tail(this);
    while (data != NULL)
    {
        data = Chain_pop_from_tail(this);
    }
}

void Chain_free_from_head(Chain *this)
{
    Block *head = this->head;
    while (head != NULL)
    {
        this->head = head->next;
        if (this->retain) Object_release(head->data);
        Object_release(head);
        head = this->head;
    }
    Object_free(this->cursor);
    Object_free(this);
}

void Chain_free_from_tail(Chain *this)
{
    Block *tail = this->tail;
    while (tail != NULL)
    {
        this->tail = tail->last;
        if (this->retain) Object_release(tail->data);
        Object_release(tail);
        tail = this->tail;
    }
    Object_free(this->cursor);
    Object_free(this);
}

void Chain_free(Chain *this) {
    Chain_free_from_tail(this);
}

void Chain_RESTE_TO_HEAD(Chain *this) {
    Cursor_set(this->cursor, this->head);
}

void Chain_RESTE_TO_TAIL(Chain *this) {
    Cursor_set(this->cursor, this->tail);
}

void *Chain_NEXT(Chain *this) {
    Block *temp = Cursor_get(this->cursor);
    if (temp == NULL) return NULL;
    Cursor_set(this->cursor, temp->next);
    return temp->data;
}

void *Chain_LAST(Chain *this) {
    Block *temp = Cursor_get(this->cursor);
    if (temp == NULL) return NULL;
    Cursor_set(this->cursor, temp->last);
    return temp->data;
}

Cursor *Chain_reset_to_head(Chain *this)
{
    return Cursor_new(this->head);
}

Cursor *Chain_reset_to_tail(Chain *this)
{
    return Cursor_new(this->tail);
}

void *Chain_next(Chain *this, Cursor *cursor)
{
    Block *temp = Cursor_get(cursor);
    if (temp == NULL) return NULL;
    Cursor_set(cursor, temp->next);
    return temp->data;
}

void *Chain_last(Chain *this, Cursor *cursor)
{
    Block *temp = Cursor_get(cursor);
    if (temp == NULL) return NULL;
    Cursor_set(cursor, temp->last);
    return temp->data;
}

void _Chain_sync(Chain *this, Chain *to) {
    Chain_RESTE_TO_HEAD(this);
    void *data = Chain_NEXT(this);
    while (data != NULL)
    {
        Chain_push_to_tail(to, data);
        data = Chain_NEXT(this);
    }
}

Chain *Chain_clone(Chain *this) {
    Chain *chain = _Chain_new(this->retain, sizeof(Chain), this->objType);
    _Chain_sync(this, chain);
    return chain;
}

void Chain_reverse(Chain *this)
{
    Chain *chain = Chain_clone(this);
    chain->retain = false;
    Chain_clear_from_head(this);
    Chain_RESTE_TO_TAIL(chain);
    void *data = Chain_LAST(chain);
    while (data != NULL)
    {
        Chain_push_to_tail(this, data);
        data = Chain_LAST(chain);
    }
    Chain_RESTE_TO_HEAD(chain);
    Object_release(chain);
}

typedef void (*CHAIN_FOREACH_FUNC)(void *, void *);

void Chain_foreach_from_head(Chain *this, CHAIN_FOREACH_FUNC func, void *arg) {
    Cursor *cursor = Chain_reset_to_head(this);
    void *ptr = NULL;
    while ((ptr = Chain_next(this, cursor)) != NULL) {
        func(ptr, arg);
    }
    Cursor_free(cursor);
}

void Chain_foreach_from_tail(Chain *this, CHAIN_FOREACH_FUNC func, void *arg) {
    Cursor *cursor = Chain_reset_to_tail(this);
    void *ptr = NULL;
    while ((ptr = Chain_last(this, cursor)) != NULL) {
        func(ptr, arg);
    }
    Cursor_free(cursor);
}

#endif
