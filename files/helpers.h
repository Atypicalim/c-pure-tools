// helpers

#ifndef H_PCT_HELPERS
#define H_PCT_HELPERS

#include "header.h"  // [M[ IGNORE ]M]

void pct_print_some_object()
{
    printf("test...\n");
}

// Object_initByType
// Object_printByType
// Object_freeByType
void pct_object_free(void *_this)
{
    if (_this == NULL) tools_error("null pointer to object free");
    Object *this = _this;
    int type = this->objType;
    if (type == PCT_OBJ_OBJECT) return Object_free(this);
    if (type == PCT_OBJ_STRING) return String_free((String *)this);
    if (type == PCT_OBJ_ARRAY) return Array_free((Array *)this);
    if (type == PCT_OBJ_CURSOR) return Cursor_free((Cursor *)this);
    if (type == PCT_OBJ_CHAIN) return Chain_free((Chain *)this);
    if (type == PCT_OBJ_STACK) return Stack_free((Stack *)this);
    if (type == PCT_OBJ_QUEUE) return Queue_free((Queue *)this);
    if (type == PCT_OBJ_HASHKEY) return Hashkey_free((Hashkey *)this);
    if (type == PCT_OBJ_HASHMAP) return Hashmap_free((Hashmap *)this);
    if (type == PCT_OBJ_FOLIAGE) return Foliage_free((Foliage *)this);
    if (type == PCT_OBJ_BLOCK) return Block_free((Block *)this);
    Object_free(this);
}

void pct_object_print(void *_this)
{
    if (_this == NULL) tools_error("null pointer to object print");
    Object *this = _this;
    int type = this->objType;
    int count = this->gcCount;
    printf("<Object t:%c c:%i p:%p>\n", type, count, this);
}

void helpers_free(void *pointer) {
    if (pointer != NULL) pct_free(pointer);
}

char *system_execute(char *msg, ...) {
    va_list lst;
    va_start(lst, msg);
    char *cmd = _tools_format(msg, lst);

    FILE *file;
    if ((file = popen(cmd, "r")) == NULL) {
        pct_free(cmd);
        return NULL;
    }
    int BUFSIZE = 1024;
    char buf[BUFSIZE];
    String *out = String_new();
    char *temp1;
    char *temp2;
    while (fgets(buf, BUFSIZE, file) != NULL) {
        String_appendArr(out, buf);
    }
    pclose(file);
    char *text = String_dump(out);
    Object_release(out);
    return text;
}

char *system_scanf() {
    char value[1024];
    scanf(" %[^\n]", value);
    String *str = String_new();
    String_appendArr(str, value);
    char *data = String_dump(str);
    Object_release(str);
    return data;
}

#endif
