// mark-sweep gc

#ifndef H_PCT_GALLECTOR
#define H_PCT_GALLECTOR

typedef Object* (*GACHE_CREATE_FUNC)(void *);
typedef Object* (*GACHE_FREE_FUNC)(void *);

typedef struct _Gallector {
    struct _Object;
    Object* first;
    int numObjects;
    int maxObjects;
    size_t extra;
    GACHE_FREE_FUNC freeer;
} Gallector;

typedef struct _Gache {
    struct _Object;
    Object *gallector;
    size_t size;
    Object *cache;
    GACHE_CREATE_FUNC creator;
} Gache;

Gallector *Gallector_new(size_t extra, GACHE_FREE_FUNC freeer)
{
    Gallector *gallector = (Gallector *)pct_mallloc(sizeof(Gallector));
    Object_init(gallector, PCT_OBJ_GALLECTOR);
    gallector->first = NULL;
    gallector->numObjects = 0;
    gallector->maxObjects = 50;
    gallector->extra = extra;
    gallector->freeer = freeer;
    return gallector;
}

Gache *Gallector_cache(Gallector *this, size_t size, GACHE_CREATE_FUNC func)
{
    Gache *gache = (Gache *)pct_mallloc(sizeof(Gache));
    Object_init(gache, PCT_OBJ_OBJECT);
    gache->gallector = (Object *)this;
    gache->size = size;
    gache->cache = NULL;
    gache->creator = func;
    return gache;
}

Object *Gache_get(Gache *this, bool freeze)
{
    Object *object = this->cache;
    if (object) {
        this->cache = object->gcNext;
    } else {
        object = this->creator(this);
    }
    object->gcCount = 0;
    object->gcNext = NULL;
    // 
    Gallector *gallector = (Gallector *)this->gallector;
    object->gcFreeze = freeze;
    if (!freeze) {
        object->gcNext = gallector->first;
        gallector->first = object;
        gallector->numObjects++;
    }
    // 
    return object;
}

bool Gache_return(Gache *this, Object *object) {
    if (this->cache) {
        int count = (this->cache)->gcCount;
        if (count >= this->size) {
            return false;
        }
        object->gcCount = count + 1;
        object->gcNext = this->cache;
    }
    this->cache = object;
    return true;
}

int Gallector_sweep(Gallector *this)
{
    int bgnCount = this->numObjects;
    Object* previous = NULL;
    Object* object = this->first;
    while (object) {
        if (object->gcMark || object->gcFreeze) {
            object->gcMark = 0;
            previous = object;
            object = object->gcNext;
            continue;
        }
        //
        Object* unreached = object;
        object = unreached->gcNext;
        if (previous != NULL) {
            previous->gcNext = object;
        } else {
            this->first = object;
        }
        //
        unreached->gcNext = NULL;
        unreached->gcCount = 0;
        this->freeer(unreached);
        this->numObjects--;
    }
    //
    int endCount = this->numObjects;
    int delCount = bgnCount - endCount;
    this->maxObjects = endCount * 2 + this->extra;
    return delCount;
}

void Gallector_print(Gallector *this)
{
    printf("[(GALLECTOR) => p:%s, s:%d]\n", this, this->numObjects);
}

void Gallector_free(Gallector *this)
{
    this->first = NULL;
    Object_free(this);
}

// 
// Gallector *gallector = Gallector_new(1000, _freeFunc);
// Gache *gache = Gallector_cache(gallector, 100, _createFunc);
// void *obj = Gache_get(gache, true);
// obj->gcFreeze = true; // no gc
// obj->gcMark = 1;
// Gallector_sweep(gallector);
// 

#endif
