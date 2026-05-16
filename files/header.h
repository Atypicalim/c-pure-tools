// pure c tools

#ifndef H_PCT_PURE_C_TOOLS
#define H_PCT_PURE_C_TOOLS



#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>

#include <math.h>
#include <time.h>
#include <setjmp.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

// object types
#define PCT_OBJ_OBJECT 'O'
#define PCT_OBJ_GALLECTOR 'G'
#define PCT_OBJ_STRING 'S'
#define PCT_OBJ_ARRAY 'A'
#define PCT_OBJ_CURSOR 'U'
#define PCT_OBJ_CHAIN 'C'
#define PCT_OBJ_STACK 'S'
#define PCT_OBJ_QUEUE 'Q'
#define PCT_OBJ_HASHKEY 'h'
#define PCT_OBJ_HASHMAP 'H'
#define PCT_OBJ_FOLIAGE 'F'
#define PCT_OBJ_BLOCK 'B'
#define PCT_OBJ_TIMER 'T'

void *pct_mallloc(size_t size)
{
    return malloc(size);
}

void *pct_realloc(void *object, size_t size)
{
    return realloc(object, size);
}

void pct_free(void *object)
{
    free(object);
}

#if defined(_WIN32) || defined(_WIN64)
    #define popen _popen
    #define pclose _pclose
#endif


#define PCT_LAMBDA(ret, body) ({ ret __fn__ body &__fn__; })
#ifndef LAMBDA
#define LAMBDA PCT_LAMBDA
#endif

#define PCT_FUNCTION(ret, name, ...) ret (*name)(__VA_ARGS__)
#ifndef FUNCTION
#define FUNCTION PCT_FUNCTION
#endif

// \033[1;31mThis is red text.\033[0m\n
#define __PCT_COLOR_TAG_BEGIN "\033[1;"
#define PCT_COLOR_TAG_GREEN __PCT_COLOR_TAG_BEGIN "32m"
#define PCT_COLOR_TAG_BLUE __PCT_COLOR_TAG_BEGIN "32m"
#define PCT_COLOR_TAG_YELLOW __PCT_COLOR_TAG_BEGIN "33m"
#define PCT_COLOR_TAG_RED __PCT_COLOR_TAG_BEGIN "31m"
#define PCT_COLOR_TAG_END "\033[0m"

char PCT_TAG_DEBUG[] = "[DEBUG]";
char PCT_TAG_INFO[]  = "[INFO ]";
char PCT_TAG_WARN[]  = "[WARN ]";
char PCT_TAG_ERROR[] = "[ERROR]";

#endif
