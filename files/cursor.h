// cursor

#ifndef H_PCT_CURSOR
#define H_PCT_CURSOR

typedef struct _Cursor {
    struct _Object;
    void *target;
} Cursor;

Cursor *Cursor_new(void *target)
{
    Cursor *cursor = (Cursor *)pct_mallloc(sizeof(Cursor));
    Object_init(cursor, PCT_OBJ_CURSOR);
    cursor->target = target;
    return cursor;
}

void Cursor_set(Cursor *this, void *target)
{
    this->target = target;
}

void *Cursor_get(Cursor *this)
{
    return this->target;
}

void Cursor_print(Cursor *this)
{
    printf("[(CURSOR) => p:%s, t:%s]\n", this, this->target);
}

void Cursor_free(Cursor *this)
{
    this->target = NULL;
    Object_free(this);
}

#endif
