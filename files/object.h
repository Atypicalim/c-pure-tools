
#ifndef H_PCT_UG_OBJECT
#define H_PCT_UG_OBJECT

void pct_object_free(void *object);
void pct_object_print(void *object);

typedef struct _Object {
    char objType;
    int gcCount;
    char gcMark;
    char gcFreeze;
    void* gcNext;
} Object;

void Object_init(void *_this, char _objType)
{
    Object *this = _this;
    this->objType = _objType;
    this->gcCount = 1;
    this->gcMark = 0;
    this->gcFreeze = true;
    this->gcNext = NULL;
    #ifdef H_PCT_OBJECT_CALLBACKS
    Object_initByType(this->objType, this);
    #endif
}

Object *Object_new()
{
    Object *object = (Object *)pct_mallloc(sizeof(Object));
    Object_init(object, PCT_OBJ_OBJECT);
    return object;
}


void Object_free(void *_this)
{
    Object *this = _this;
    pct_free(this);
}

void Object_retain(void *_this)
{
    if (_this == NULL) tools_error("null pointer to object retain");
    Object *this = _this;
    this->gcCount++;
}

void Object_release(void *_this)
{
    if (_this == NULL) tools_error("null pointer to object release");
    Object *this = _this;
    this->gcCount--;
    if (this->gcCount <= 0) {
        #ifdef H_PCT_OBJECT_CALLBACKS
        Object_freeByType(this->objType, this);
        #else
        pct_object_free(this);
        #endif
    }
}

void Object_print(void *_this)
{
    if (_this == NULL) tools_error("null pointer to object print");
    Object *this = _this;
    #ifdef H_PCT_OBJECT_CALLBACKS
    Object_printByType(this->objType, this);
    #else
    pct_object_print(this);
    #endif
}

#endif
