// hashmap

#ifndef H_PCT_HASHMAP
#define H_PCT_HASHMAP

#include "header.h"  // [M[ IGNORE ]M]
#include "hashkey.h" // [M[ IGNORE ]M]

#define HASHMAP_DEFAULT_CAPACITY 4096

typedef struct _Hashmap {
    struct _Object;
    int size;
    bool retain;
    Hashkey *bucket[HASHMAP_DEFAULT_CAPACITY];
} Hashmap;

Hashmap* Hashmap_new(bool isRetainValue) {
    Hashmap *map = (Hashmap *)pct_mallloc(sizeof(Hashmap));
    Object_init(map, PCT_OBJ_HASHMAP);
    map->retain = isRetainValue;
    map->size = HASHMAP_DEFAULT_CAPACITY;
    for (int i = 0; i < map->size; ++i ) map->bucket[i] = NULL;
    return map;
}


void Hashmap_clear(Hashmap *this) {
    Hashkey *ptr;
    Hashkey *tmp;
    for (int i = 0; i < this->size; ++i) {
        ptr = this->bucket[i];
        while (ptr != NULL) {
            tmp = ptr;
            ptr = ptr->next;
            if (this->retain) {
                Object_release(tmp->value);
            }
            Object_release(tmp);
        }
        this->bucket[i] = NULL;
    }
}

// TODO: release removed value
void Hashmap_free(Hashmap *this) {
    assert(this != NULL);
    Hashmap_clear(this);
    Object_free(this);
}

typedef bool (*HASHMAP_SET_FUNC)(void *, void *);

void *Hashmap_setByCheck(Hashmap *this, char *_key, void *value, HASHMAP_SET_FUNC func) {
    assert(this != NULL);
    assert(_key != NULL);
    assert(value != NULL);
    String *key = String_format(_key);
    int pos = String_hash(key) % this->size;
    // new position
    Hashkey *ptr = this->bucket[pos];
    if (ptr == NULL) {
        if (func(NULL, value)) {
            this->bucket[pos] = Hashkey_new(key, value);
            if (this->retain) Object_retain(value);
        }
        Object_release(key);
        return NULL;
    }
    // replace old
    void *tmp = NULL;
    void *rpl = NULL;
    while (ptr != NULL) {
        if (String_equal(key, ptr->key)) {
            tmp = ptr->value;
            if (func(tmp, value)) {
                rpl = tmp;
                Hashkey_set(ptr, value);
                if (this->retain) {
                    Object_retain(value);
                    Object_release(rpl);
                }
            }
            break;
        }
        ptr = ptr->next;
    }
    if (rpl != NULL) {
        Object_release(key);
        return rpl;
    }
    // prepend position
    if (func(NULL, value)) {
        Hashkey *pnode = Hashkey_new(key, value);
        pnode->next = this->bucket[pos];
        this->bucket[pos] = pnode;
        if (this->retain) Object_retain(value);
    }
    Object_release(key);
    return NULL;
}

bool _hashmap_set_by_default(void *_old, void *_new) {
    return true;
}

void *Hashmap_set(Hashmap *this, char *_key, void *value) {
    return Hashmap_setByCheck(this, _key, value, _hashmap_set_by_default);
}

void *Hashmap_get(Hashmap *this, char *_key) {
    assert(this != NULL);
    assert(_key != NULL);
    String *key = String_format(_key);
    int pos = String_hash(key) % this->size;
    //
    Hashkey *ptr = this->bucket[pos];
    while (ptr != NULL) {
        if (String_equal(key, ptr->key)) {
            Object_release(key);
            return ptr->value;
        }
        ptr = ptr->next;
    }
    Object_release(key);
    return NULL;
}

// TODO: release removed value
void *Hashmap_del(Hashmap *this, char *_key) {
    assert(this != NULL);
    assert(_key != NULL);
    String *key = String_format(_key);
    int pos = String_hash(key) % this->size;
    //
    void *tmp = NULL;
    Hashkey *ptr = this->bucket[pos];
    if (ptr == NULL) {
        Object_release(key);
        return NULL;
    }
    Hashkey *pre = NULL;
    while (ptr != NULL) {
        if (String_equal(key, ptr->key)) {
            if (pre == NULL) {
                this->bucket[pos] = NULL;
            } else {
                pre->next = ptr->next;
            }
            tmp = ptr->value;
            if (this->retain) {
                Object_release(ptr->value);
            }
            Object_release(ptr);
            break;
        }
        pre = ptr;
        ptr = ptr->next;
    }
    Object_release(key);
    return tmp;
}

typedef void (*HASHMAP_FOREACH_FUNC)(Hashkey *, void *);

void Hashmap_foreachItem(Hashmap *this, HASHMAP_FOREACH_FUNC func, void *arg) {
    Hashkey *ptr;
    for (int i = 0; i < this->size; ++i) {
        ptr = this->bucket[i];
        while (ptr != NULL) {
            func(ptr, arg);
            ptr = ptr->next;
        }
    }
}

void _hashmap_copy_to_other(Hashkey *hashkey, void *other) {
    String *key = hashkey->key;
    void *val = hashkey->value;
    char *_key = String_get(key);
    Hashmap_set(other, _key, val);
}

void Hashmap_copyTo(Hashmap *this, Hashmap *other)
{
    Hashmap_foreachItem(this, _hashmap_copy_to_other, other);
}

char *Hashmap_toString(Hashmap *this)
{
    return tools_format("[Hashmap => p:%p s:%i]", this, this->size);
}

#endif
