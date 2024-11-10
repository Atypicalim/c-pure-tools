
// ./files/header.h

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
#include <unistd.h>
#include <setjmp.h>
#include <sys/stat.h>
#include <sys/time.h>

// object types
#define PCT_OBJ_OBJECT 'O'
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


// ./files/log.h

// log

// https://github.com/rxi/log.c

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>

#define PCT_LOG_VERSION "0.1.0"

#define MAX_CALLBACKS 32

typedef struct {
  int level;
  int line;
  const char *file;
  const char *fmt;
  va_list args;
  struct tm *time;
  void *target;
} log_Event;

typedef void (log_Func)(log_Event *ev);

static struct {
  bool quiet;
  int level;
  int color;
  FILE *file;
  log_Func *callbacks;
} L;

enum { PCT_LOG_DEBUG, PCT_LOG_INFO, PCT_LOG_WARN, PCT_LOG_ERROR };

static const char *level_strings[] = {
  PCT_TAG_DEBUG, PCT_TAG_INFO, PCT_TAG_WARN, PCT_TAG_ERROR
};
static const char *level_colors[] = {
  "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m"
};

////////////////////////////////////////////////////////////////////////////////

static void _log_stdio_callback(log_Event *ev) {
  char buf[64];
  buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
  if (L.color) {
    fprintf(
      ev->target,
      "%s %s%-2s\x1b[0m \x1b[90m%s:%03d:\x1b[0m ",
      buf, level_colors[ev->level], level_strings[ev->level],
      ev->file, ev->line
    );
  } else {
    fprintf(
      ev->target,
      "%s %-2s %s:%d: ",
      buf, level_strings[ev->level], ev->file, ev->line
    );
  }
}


static void _log_file_callback(log_Event *ev) {
  if (!L.file) return;
  char buf[128];
  buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
  fprintf(
    ev->target,
    "%s %-2s %s:%d: ",
    buf, level_strings[ev->level], ev->file, ev->line
  );
}

static void _log_fill_args(log_Event *ev) {
  vfprintf(ev->target, ev->fmt, ev->args);
  fprintf(ev->target, "\n");
  fflush(ev->target);
}

static void init_event(log_Event *ev, void *target) {
  if (!ev->time) {
    time_t t = time(NULL);
    ev->time = localtime(&t);
  }
  ev->target = target;
}

void __pct_log(int level, const char *file, int line, const char *fmt, ...) {
  log_Event ev = {
    .fmt   = fmt,
    .file  = file,
    .line  = line,
    .level = level,
  };
  //
  if (!L.quiet && level >= L.level) {
    va_start(ev.args, fmt);
    //
    init_event(&ev, stderr);
    _log_stdio_callback(&ev);
    _log_fill_args(&ev);
    //
    init_event(&ev, L.file);
    _log_file_callback(&ev);
    _log_fill_args(&ev);
    //
    va_end(ev.args);
  }

    log_Func *func = L.callbacks;
    if (func != NULL && level >= L.level) {
      init_event(&ev, NULL);
      va_start(ev.args, fmt);
      func(&ev);
      va_end(ev.args);
    }

}

////////////////////////////////////////////////////////////////////////////////

#define log_debug(...) __pct_log(PCT_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  __pct_log(PCT_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  __pct_log(PCT_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) __pct_log(PCT_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////

void log_set_quiet(bool enable) {
  L.quiet = enable;
}

void log_set_level(int level) {
  L.level = level;
}

void log_set_color(bool enabled) {
  L.color = enabled;
}

int log_set_file(char *path) {
  if (!path) {
    L.file = NULL;
  } else {
    L.file = fopen(path, "w");
  }
}

int log_set_func(log_Func *func) {
  L.callbacks = *func;
}


// ./files/tools.h

// tools

#ifndef H_PCT_TOOLS
#define H_PCT_TOOLS


// os type
#define PLATFORM_WINDOWS "PLATFORM_WINDOWS"
#define PLATFORM_APPLE "PLATFORM_APPLE"
#define PLATFORM_LINUX "PLATFORM_LINUX"
#define PLATFORM_UNKNOWN "PLATFORM_UNKNOWN"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define PLATFORM_NAME PLATFORM_WINDOWS
    #define IS_WINDOWS
#elif __APPLE__
    #define PLATFORM_NAME PLATFORM_APPLE
    #define IS_APPLE
#elif __linux__ || __unix || __unix__
    #define PLATFORM_NAME PLATFORM_LINUX
    #define IS_LINUX
#else
    #define PLATFORM_NAME PLATFORM_UNKNOWN
    #define IS_UNKNOWM
#endif

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int pct_cstr_starts_with(const char *s, const char *test)
{
    return strncmp(s, test, strlen(test)) == 0;
}

/* return 0 for match, nonzero for no match */
int pct_cstr_ends_with(const char *s, const char *test)
{
    size_t slen = strlen(s);
    size_t tlen = strlen(test);
    if (tlen > slen) return 1;
    return strcmp(s + slen - tlen, test);
}


void tools_error(const char* msg, ...) {
    va_list lst;
    va_start(lst, msg);
    printf("%s%s%s => ", PCT_COLOR_TAG_RED, PCT_TAG_ERROR, PCT_COLOR_TAG_END);
    vfprintf(stdout, msg, lst);
    printf("\n");
    va_end(lst);
    exit(1);
}

void tools_assert(bool value, const char *msg, ...)
{
    if (value == true) return;
    va_list lst;
    va_start(lst, msg);
    printf("%s%s%s => ", PCT_COLOR_TAG_RED, PCT_TAG_ERROR, PCT_COLOR_TAG_END);
    vfprintf(stdout, msg, lst);
    printf("\n");
    va_end(lst);
    exit(1);
}

void tools_warn(const char* msg, ...) {
    va_list lst;
    va_start(lst, msg);
    printf("%s%s%s => ", PCT_COLOR_TAG_YELLOW, PCT_TAG_WARN, PCT_COLOR_TAG_END);
    vfprintf(stdout, msg, lst);
    printf("\n");
    va_end(lst);
}

void tools_log(const char* msg, ...) {
    va_list lst;
    va_start(lst, msg);
    printf("%s%s%s => ", PCT_COLOR_TAG_GREEN, PCT_TAG_INFO, PCT_COLOR_TAG_END);
    vfprintf(stdout, msg, lst);
    printf("\n");
    va_end(lst);
}

void tools_debug(const char* msg, ...) {
    va_list lst;
    va_start(lst, msg);
    printf("%s%s%s => ", PCT_COLOR_TAG_BLUE, PCT_TAG_DEBUG, PCT_COLOR_TAG_END);
    vfprintf(stdout, msg, lst);
    printf("\n");
    va_end(lst);
}

char *_tools_format(char *msg, va_list lst) {
    va_list lstCopy;
    va_copy(lstCopy, lst);
    int bufsz = vsnprintf(NULL, 0, msg, lst);
    char* text = malloc(bufsz + 1);
    vsnprintf(text, bufsz + 1, msg, lstCopy);
    va_end(lst);
    va_end(lstCopy);
    return text;
}

char *tools_format(char *msg, ...)
{
    va_list lst;
    va_start(lst, msg);
    _tools_format(msg, lst);
}

void tools_set_env(char *name, char *value) {
    char *text = tools_format("%s=%s", name, value);
    putenv(text);
    pct_free(text);
}

char *tools_get_env(char *name) {
    return getenv(name);
}

int char_to_int(char c)
{
    int i = c - '0';
    if (i >= 49) i-= 39;
    if (i < 0 || i > 15) return 0;
    return i;
}

char *arr_to_str(char arr[], int len)
{
    char* ptr = (char *)malloc(len * sizeof(char) + 1);
    for (int i = 0; i < len; i++)
    {
        ptr[i] = arr[i];
    }
    ptr[len] = '\0';
    return ptr;
}

int color_hex_to_int(char *str)
{
    int len = strlen(str);
    int result = 0;
    int number;
    for (int i = len - 1; i >= 0; i--)
    {
        number = char_to_int(str[len - i - 1]);
        result = result + number * pow(16, i);
    }
    return result;
}

void color_hex_to_rgba(unsigned int hex, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a) {
    *a = (hex >> 24) & 0xFF;
    *r = (hex >> 16) & 0xFF;
    *g = (hex >> 8) & 0xFF;
    *b = hex & 0xFF;
}

unsigned int color_rgba_to_hex(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | (unsigned int)b;
}

char* color_hex_to_str(unsigned int hex) {
    char* buffer = (char*)malloc(10 * sizeof(char));
    snprintf(buffer, 10, "#%08X", hex);
    return buffer;
}

unsigned int color_str_to_hex(const char* hex) {
    if (hex[0] == '#') hex++;
    return (unsigned int)strtol(hex, NULL, 16);
}

int num_random(int from, int to)
{
    int big = to > from ? to : from;
    int small = to > from ? from : to;
    int count = big - small + 1;
    int num = rand() % count;
    int r = small + num;
    return r;
}

// limit as : 0 ~ size (-size ~ -1)
void limit_range(int size, bool swapXY, int *_from, int *_to)
{
    int from = *_from;
    int to = *_to;
    if (from < 0) from = MAX(0, size + from);
    if (to < 0) to = MAX(0, size + to);
    from = MAX(0, MIN(size - 1, from));
    to = MAX(0, MIN(size - 1, to));
    if (swapXY && from > to) {
        int tmp = from;
        from = to;
        to = tmp;
    }
    *_from = from;
    *_to = to; 
}

bool file_write(char *path, char *data)
{
    if (data == NULL) return false;
    FILE *fp = fopen(path, "ab");
    if (fp == NULL) return false;
    fputs(data, fp);
    fclose(fp);
    return true;
}

char *file_read(char *path)
{
    char *text;
    FILE *file = fopen(path, "rb");
    if (file == NULL) return NULL;
    fseek(file, 0, SEEK_END);
    long lSize = ftell(file);
    text = (char *)malloc(lSize + 1);
    rewind(file);
    fread(text, sizeof(char), lSize, file);
    text[lSize] = '\0';
    fclose(file);
    return text;
}

void file_fetch(char *path, char **_text, int *_size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) return;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    char *text = (char *)malloc(size + 1);
    rewind(file);
    fread(text, sizeof(char), size, file);
    text[size] = '\0';
    fclose(file);
    *_text = text;
    *_size = size;
}

bool file_copy(char *path, char *to)
{
    int BUF_SIZE = 1024;
    FILE *src, *dst;
    size_t in, out;
    src = fopen(path, "rb");
    dst = fopen(to, "wb");
    if (src == NULL || dst == NULL) {
        if (src != NULL) return fclose(src);
        if (dst != NULL) return fclose(dst);
        return false;
    }
    char *buf = (char*) malloc(BUF_SIZE * sizeof(char));
    while (1) {
        in = fread(buf, sizeof(char), BUF_SIZE, src);
        if (0 == in) break;
        out = fwrite(buf, sizeof(char), in, dst);
        if (0 == out) break;
    }
    fclose(src);
    fclose(dst);
    free(buf);
    return true;
}

int file_rename(char *path, char *to)
{
    return rename(path, to);
}

int file_remove(char *path)
{
    return unlink(path);
}

bool file_exist(char *path)
{
    return access(path, F_OK) == 0;
}

int file_mkdir(const char* name)
{
    #ifdef __linux__
        return mkdir(name, 777); /* Or what parameter you need here ... */
    #else
        return mkdir(name);
    #endif
}

bool file_is_file(char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool file_is_directory(char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

int file_create_directory(char *path)
{
    char tmp[1024];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp),"%s",path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++)
    {
        if (*p != '/') continue;
        *p = 0;
        file_mkdir(tmp);
        *p = '/';
    }
    return file_mkdir(tmp);
}

#endif


// ./files/object.h


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


// ./files/cstring.h


// HEADER ---------------------------------------------------------------------

#ifndef STDSTRING_H
#define STDSTRING_H

#include <stdarg.h>
#include <stdint.h>
#include <wchar.h>
#include <stdbool.h>
#include <math.h>  /* remember to compile with -lm */
#include <setjmp.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#ifndef AVA
   // code annonations
   #define HEAP           /* heap pointer. must free() after use */
   #define TEMP           /* temporary stack pointer. do not free() after use */
   #define INOUT          /* both input and output argument */
   #define OPTIONAL       /* optional argument */
   // mem wrappers
   #define REALLOC(p, sz) realloc((p), (sz))
   #define MALLOC(sz)     REALLOC(0, (sz))
   #define CALLOC(n, m)   memset(MALLOC((n)*(m)), 0, (n)*(m))
   #define FREE(p)        REALLOC(p, 0)
   #define STRDUP(s)      strdup(s)
   // compiler stuff
   #ifdef _MSC_VER
   #define builtin(x)     __declspec(x)
   #else
   #define builtin(x)     __##x
   #endif
   // raii perf/profiler
   #define $
#endif

// IMPLEMENTATION -------------------------------------------------------------

// stricmp() is common, but not standard, so I provide my own
static int istrcmp(const char *p, const char *q) {
    for(; tolower(p[0]) == tolower(q[0]) && p[0]; p++, q++);
    return tolower(p[0]) - tolower(q[0]);
}

// ## rob pike's regexp match (apparently public domain).
// [ref] https://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
// @todo: evaluate @kokke/tiny-regex-c instead.

int regstar(const char *string, const char *re, int c);

int reghere(const char *string, const char *re) {
    if( re[0] == '\0' ) return 1;
    if( re[1] == '*' ) return regstar(string, re+2, re[0]);
    if( re[0] == '$' && re[1] == '\0' ) return *string == '\0';
    if( *string!='\0' && (re[0]=='?' || re[0]==*string) ) return reghere(string+1, re+1);
    return 0;
}

int regstar(const char *string, const char *re, int c) {
    do { /* a * matches zero or more instances */
        if( reghere(string, re) ) return 1;
    } while( *string != '\0' && (*string++ == c || c == '?') );
    return 0;
}

int strregex(const char *string, const char *re) {
    if( re[0] == '^' ) return reghere(string, re+1);
    do { /* must look even if string is empty */
        if( reghere(string, re) ) return 1;
    } while (*string++ != '\0');
    return 0;
}

bool strmatch( const char *text, const char *pattern ) { $
    if( *pattern=='\0' ) return !*text;
    if( *pattern=='*' )  return strmatch(text, pattern+1) || (*text && strmatch(text+1, pattern));
    if( *pattern=='?' )  return *text && (*text != '.') && strmatch(text+1, pattern+1);
    return (*text == *pattern) && strmatch(text+1, pattern+1);
}

// ## C-style formatting
// - rlyeh, public domain.

const char *strsub( const char *str, int pos ) { $
    int size = strlen(str);
    pos = pos && size ? (pos > 0 ? pos % size : size-1 + ((pos+1) % size)) : 0;
    return str + pos;
}

const char* strfindl(const char *text, const char *substring) { $
    const char *found = strstr( text, substring );
    return found ? found : text + strlen(text);
}
const char* strfindr(const char *text, const char *substring) { $
    char *found = 0;
    while(1) {
        char *found2 = strstr(text, substring);
        if( !found2 ) break;
        found = found2;
        text = found2 + 1;
    }
    return found ? found : text + strlen(text);
}

bool strbegin( const char *text, const char *substring ) { $
    // also, return strncmp(string, substr, strlen(substr)) == 0;
    int s1 = strlen(text), s2 = strlen(substring);
    return s1 >= s2 ? 0 == memcmp( &text[       0 ], substring, s2 ) : false;
}
bool strend( const char *text, const char *substring ) { $
    int s1 = strlen(text), s2 = strlen(substring);
    return s1 >= s2 ? 0 == memcmp( &text[ s1 - s2 ], substring, s2 ) : false;
}

bool streq( const char *string, const char *substr ) { $
    return !strcmp( string, substr );
}
bool streqi( const char *string, const char *substr ) { $
    while( *string && *substr ) {
        int eqi = (*string++ | 32) == (*substr++ | 32);
        if( !eqi ) return 0;
    }
    return *string == 0 && *substr == 0;
}

// ## string conversion unit utils
// - rlyeh, public domain.

int64_t strint( const char *string ) { $
    int64_t v = 0, n = 1;
    if( string ) {
        while( *string == '-' ) n *= -1, string++;
        while( *string >= '0' && *string <= '9') v = (v * 10) + *string++ - '0';
    }
    return n * v;
}

// ## string transform utils
// First four functions are based on code by Bob Stout (public domain).
// - rlyeh, public domain.

char *strlower(char *string) {
    if( string ) for( char *s = string; *s; ++s ) *s = tolower(*s);
    return string;
}
char *strupper(char *string) {
    if( string ) for( char *s = string; *s; ++s ) *s = toupper(*s);
    return string;
} 
char *strrev(char *string) {
    if(string && *string)
    for( char *p1 = string, *p2 = p1 + strlen(p1) - 1; p2 > p1; ++p1, --p2 ) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return string;
}
char *strdel(char *string, const char *substring) {
    if( string ) {
        char *p = strstr(string, substring);
        if( p ) {
            for( int len = strlen(substring); p[len] ; ++p ) {
                p[0] = p[len];
            }
            *p = 0;
        }
    }
    return string;
}

// ## string transform utils
// - rlyeh, public domain.

char* strtrimbff(char *string, const char *substring) { $
    char *found = strstr(string, substring);
    if( found ) {
        int L = strlen(substring);
        memmove(string, found+L, strlen(string) - L);
    }
    return string;
}
char* strtrimffe(char *string, const char *substring) { $
    ((char *)strfindl(string, substring))[0] = 0;
    return string;
}
char* strtrimblf(char *string, const char *substring) { $
    const char *found = strfindr(string, substring);
    int L = strlen(substring);
    memmove( string, found + L, strlen(found) - L + 1);
    return string;
}
char* strtrimlfe(char *string, const char *substring) { $
    ((char *)strfindr(string, substring))[0] = 0;
    return string;
}
char *strtrimws(char *str) {
    char *ibuf, *obuf;
    if( str ) {
        for( ibuf = obuf = str; *ibuf; ) {
            while( *ibuf && isspace(*ibuf)  )  (ibuf++);
            if(    *ibuf && obuf != str     ) *(obuf++) = ' ';
            while( *ibuf && !isspace(*ibuf) ) *(obuf++) = *(ibuf++);
        }
        *obuf = 0;
    }
    return str;
}

// ## replace substring in a string
// - rlyeh, public domain.


// ## split strings into tokens
// - rlyeh, public domain.

int strchop( const char *string, const char *delimiters, int avail, const char *tokens[] ) { $
    assert( avail >= 4 && 0 == ( avail % 2 ) );
    for( avail /= 2; *string && avail-- > 0; ) {
        int n = strcspn( string += strspn(string, delimiters), delimiters );
        *tokens++ = (*tokens++ = (const char *)(uintptr_t)n) ? string : "";
        string += n;
    }
    return *tokens++ = 0, *tokens = 0, avail > 0;
}

HEAP char **strsplit( const char *string, const char *delimiters ) { $
    int L = strlen(string), len = sizeof(char *) * (L/2+1+1), i = 0;
    char **res = (char **)CALLOC(1, len + L + 1 );
    char *buf = strcpy( (char *)res + len, string );
    for( char *token = strtok(buf, delimiters); token; token = strtok(NULL, delimiters) ) {
        res[i++] = token;
    }
    return res;
}

// ## string interning (quarks)
// - rlyeh, public domain.

static builtin(thread) int quarklen = 0, quarkcap = 0;
static builtin(thread) char *quarks = 0;

int strput( const char *string ) { $
    if( !quarks ) {
        // init buffer on first time
        quarks = (char*)REALLOC( quarks, (1+1) );
        quarkcap += 1;
        // copy null string
        quarks[0] = 0;
        quarklen += 1;
        quarkcap -= 1;
    }
    if( string && string[0] ) {
        int len = strlen(string)+1;
        if( quarkcap < len ) {
            int extend = (int)(len * 1.5f); // 2.33f
            // printf("alloc += %d\n", extend);
            quarks = (char*)REALLOC( quarks, quarklen + extend );
            quarkcap += extend;
        }
        memcpy( quarks + quarklen, string, len );
        quarklen += len;
        quarkcap -= len;
        // printf("%d/%d\n", quarklen, quarklen+quarkcap);
        return quarklen - len;
    }
    return 0;
}
const char *strget( int key ) {
    assert( quarks );
    return quarks + key;
}

// ## Fuzzy string matching
// Ideas from https://blog.forrestthewoods.com/reverse-engineering-sublime-text-s-fuzzy-match-4cffeed33fdb
// - rlyeh, public domain.


int strscore( const char *str1, const char *str2 ) {
    int score = 0, consecutive = 0, maxerrors = 0;
    while( *str1 && *str2 ) {
        int is_leading = (*str1 & 64) && !(str1[1] & 64);
        if( (*str1 & ~32) == (*str2 & ~32) ) {
            int had_separator = (str1[-1] <= 32);
            int x = had_separator || is_leading ? 10 : consecutive * 5;
            consecutive = 1;
            score += x;
            ++str2;
        } else {
            int x = -1, y = is_leading * -3;
            consecutive = 0;
            score += x;
            maxerrors += y;
        }
        ++str1;
    }
    return score + (maxerrors < -9 ? -9 : maxerrors);
}

const char *strfuzzy( const char *str, int num, const char *words[] ) {
    int scoremax = 0;
    const char *best = 0;
    for( int i = 0; i < num; ++i ) {
        int score = strscore( words[i], str );
        int record = ( score >= scoremax );
        int draw = ( score == scoremax );
        if( record ) {
            scoremax = score;
            if( !draw ) best = words[i];
            else best = best && strlen(best) < strlen(words[i]) ? best : words[i];
        }
    }
    return best ? best : "";
}

// ## utf8 and unicode
// Based on code by @ddiakopoulos (unlicensed).
// Based on code by @nothings (public domain).
// - rlyeh, public domain.

uint32_t strutf32(INOUT const char **p) { $
    if( (**p & 0x80) == 0x00 ) {
        int a = *((*p)++);
        return a;
    }
    if( (**p & 0xe0) == 0xc0 ) {
        int a = *((*p)++) & 0x1f;
        int b = *((*p)++) & 0x3f;
        return (a << 6) | b;
    }
    if( (**p & 0xf0) == 0xe0 ) {
        int a = *((*p)++) & 0x0f;
        int b = *((*p)++) & 0x3f;
        int c = *((*p)++) & 0x3f;
        return (a << 12) | (b << 6) | c;
    }
    if( (**p & 0xf8) == 0xf0 ) {
        int a = *((*p)++) & 0x07;
        int b = *((*p)++) & 0x3f;
        int c = *((*p)++) & 0x3f;
        int d = *((*p)++) & 0x3f;
        return (a << 18) | (b << 12) | (c << 8) | d;
    }
    return 0;
}
static builtin(thread) uint8_t utf[8] = {0};
TEMP char *strutf8(uint32_t cp) { $
    int n = 0;
    /**/ if( cp <        0x80 ) n = 1;
    else if( cp <       0x800 ) n = 2;
    else if( cp <     0x10000 ) n = 3;
    else if( cp <    0x200000 ) n = 4;
    else if( cp <   0x4000000 ) n = 5;
    else if( cp <= 0x7fffffff ) n = 6;
    switch (n) {
        case 6: utf[5] = 0x80 | (cp & 0x3f); cp = (cp >> 6) | 0x4000000;
        case 5: utf[4] = 0x80 | (cp & 0x3f); cp = (cp >> 6) | 0x200000;
        case 4: utf[3] = 0x80 | (cp & 0x3f); cp = (cp >> 6) | 0x10000;
        case 3: utf[2] = 0x80 | (cp & 0x3f); cp = (cp >> 6) | 0x800;
        case 2: utf[1] = 0x80 | (cp & 0x3f); cp = (cp >> 6) | 0xc0;
        case 1: utf[0] = cp; default:;
    }
    return utf[n] = '\0', (char *)utf;
}

wchar_t *strwiden_(wchar_t *buffer, const char *ostr, int n) { $
   const uint8_t *str = (const uint8_t *) ostr;
   uint32_t c;
   int i=0;
   --n;
   while (*str) {
      if (i >= n)
         return NULL;
      if (!(*str & 0x80))
         buffer[i++] = *str++;
      else if ((*str & 0xe0) == 0xc0) {
         if (*str < 0xc2) return NULL;
         c = (*str++ & 0x1f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf0) == 0xe0) {
         if (*str == 0xe0 && (str[1] < 0xa0 || str[1] > 0xbf)) return NULL;
         if (*str == 0xed && str[1] > 0x9f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x0f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         buffer[i++] = c + (*str++ & 0x3f);
      } else if ((*str & 0xf8) == 0xf0) {
         if (*str > 0xf4) return NULL;
         if (*str == 0xf0 && (str[1] < 0x90 || str[1] > 0xbf)) return NULL;
         if (*str == 0xf4 && str[1] > 0x8f) return NULL; // str[1] < 0x80 is checked below
         c = (*str++ & 0x07) << 18;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 12;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f) << 6;
         if ((*str & 0xc0) != 0x80) return NULL;
         c += (*str++ & 0x3f);
         // utf-8 encodings of values used in surrogate pairs are invalid
         if ((c & 0xFFFFF800) == 0xD800) return NULL;
         if (c >= 0x10000) {
            c -= 0x10000;
            if (i + 2 > n) return NULL;
            buffer[i++] = 0xD800 | (0x3ff & (c >> 10));
            buffer[i++] = 0xDC00 | (0x3ff & (c      ));
         }
      } else
         return NULL;
   }
   buffer[i] = 0;
   return buffer;
}

char *strshorten_(char *buffer, const wchar_t *str, int n) { $
   int i=0;
   --n;
   while (*str) {
      if (*str < 0x80) {
         if (i+1 > n) return NULL;
         buffer[i++] = (char) *str++;
      } else if (*str < 0x800) {
         if (i+2 > n) return NULL;
         buffer[i++] = 0xc0 + (*str >> 6);
         buffer[i++] = 0x80 + (*str & 0x3f);
         str += 1;
      } else if (*str >= 0xd800 && *str < 0xdc00) {
         uint32_t c;
         if (i+4 > n) return NULL;
         c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
         buffer[i++] = 0xf0 + (c >> 18);
         buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
         buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
         buffer[i++] = 0x80 + ((c      ) & 0x3f);
         str += 2;
      } else if (*str >= 0xdc00 && *str < 0xe000) {
         return NULL;
      } else {
         if (i+3 > n) return NULL;
         buffer[i++] = 0xe0 + (*str >> 12);
         buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
         buffer[i++] = 0x80 + ((*str     ) & 0x3f);
         str += 1;
      }
   }
   buffer[i] = 0;
   return buffer;
}

TEMP wchar_t *strwiden(const char *str) {
   int len = strlen(str) * 6;
   wchar_t *buffer = (wchar_t*)MALLOC( len );
   return strwiden_(buffer, str, len);
}

TEMP char *strshorten(const wchar_t *str) {
   int len = wcslen(str) * 6;
   char *buffer = (char*)MALLOC( len );
   return strshorten_(buffer, str, len);
}

uint64_t strhash(const char *str) {
    uint64_t hash = 0;
    while( *str++ ) {
        hash = ( str[-1] ^ hash ) * 131ull;
    }
    return hash;
}

#endif // STDSTRING_H



// ./files/string.h

// string

#ifndef H_PCT_UG_STRING
#define H_PCT_UG_STRING

#define STRING_MIN_CAPACITY 128

typedef struct _String {
    struct _Object;
    int length;
    int capacity;
    char *data;
} String;

String *String_new()
{
    String *string = (String *)pct_mallloc(sizeof(String));
    Object_init(string, PCT_OBJ_STRING);
    string->length = 0;
    string->capacity = STRING_MIN_CAPACITY + 1;
    string->data = pct_mallloc(string->capacity);
    string->data[string->length] = '\0';
    return string;
}

void String_free(String *this)
{
    pct_free(this->data);
    Object_free(this);
}

void _string_check_capacity(String *this, int length)
{
    if (this->capacity >= length + 1) return;
    while (this->capacity < length + 1 && this->capacity * 2 < INT_MAX) {
        this->capacity *= 2;
        if (this->capacity <= 0) this->capacity--;
    }
    this->data = pct_realloc(this->data, this->capacity);
}

String *String_appendChar(String *this, char c)
{
    _string_check_capacity(this, this->length + 1);
    this->data[this->length] = c;
    this->length++;
    this->data[this->length] = '\0';
    return this;
}

String *String_prependChar(String *this, char c)
{
    _string_check_capacity(this, this->length + 1);
    memmove(this->data + 1, this->data, this->length);
    this->data[0] = c;
    this->length++;
    this->data[this->length] = '\0';
    return this;
}

String *String_appendArr(String *this, char arr[])
{
    if (arr == NULL) return this;
    int len = strlen(arr);
    if (len == 0) return this;
    _string_check_capacity(this, this->length + len);
    memmove(this->data + this->length, arr, len);
    this->length += len;
    this->data[this->length] = '\0';
    return this;
}

String *String_prependArr(String *this, char arr[])
{
    if (arr == NULL) return this;
    int len = strlen(arr);
    if (len == 0) return this;
    memmove(this->data + len, this->data, this->length);
    memmove(this->data, arr, len);
    this->length += len;
    this->data[this->length] = '\0';
    return this;
}

String *String_appendStr(String *this, char *str)
{
    if (str == NULL || *str == '\0') return this;
    int len = strlen(str);
    if (len == 0) return this;
    _string_check_capacity(this, this->length + len);
    memmove(this->data + this->length, str, len);
    this->length += len;
    this->data[this->length] = '\0';
    return this;
}

String *String_prependStr(String *this, char *str)
{
    if (str == NULL || *str == '\0') return this;
    int len = strlen(str);
    if (len == 0) return this;
    _string_check_capacity(this, this->length + len);
    memmove(this->data + len, this->data, this->length);
    memmove(this->data, str, len);
    this->length += len;
    this->data[this->length] = '\0';
    return this;
}

String *String_append(String *this, char *str)
{
    return String_appendStr(this, str);
}

String *String_prepend(String *this, char *str)
{
    return String_prependStr(this, str);
}

String *String_insert(String *this, int at, String *that)
{
    if (at < 0 || at > this->length || that == NULL || that->length <= 0) return this;
    _string_check_capacity(this, this->length + that->length);
    char *temp = pct_mallloc(this->length - at);
    memmove(temp, this->data + at, this->length - at);
    memmove(this->data + at, that->data, that->length);
    memmove(this->data + at + that->length, temp, this->length - at);
    pct_free(temp);
    this->length += that->length;
    this->data[this->length] = '\0';
    return this;
}

String *String_delete(String *this, int from, int to)
{
    if (this->length <= 0) return this;
    limit_range(this->length, true, &from, &to);
    int len = this->length;
    char *dst = this->data + from;
    char *src = this->data + to + 1;
    size_t cnt = this->length - to - 1;
    memmove(dst, src, cnt);
    this->length -= (to - from + 1);
    this->data[this->length] = '\0';
    _string_check_capacity(this, this->length + 1);
    return this;
}

String *String_deleteStarting(String *this, int to)
{
    return String_delete(this, 0, to);
}

String *String_deleteEnding(String *this, int from)
{
    return String_delete(this, from, this->length);
}

String *String_clear(String *this)
{
    this->length = 0;
    this->data[this->length] = '\0';
    return this;
}

unsigned long String_hash(String *this)
{
    char *str = this->data;
    unsigned long hash = 5381;
    int c;
    while (c = *str++) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

int String_findNext(String *this, int from, char *target)
{
    int len = strlen(target);
    if (this->length <= 0 || from < 0 || len <= 0) return -1;
    char *ptr = strstr(this->data + from, target);
    if (ptr == NULL) return -1;
    int pos = ptr - this->data;
    return pos;
}

int String_findLast(String *this, int to, char *target)
{
    int len = strlen(target);
    if (this->length <= 0 || to > this->length || len <= 0) return -1;
    int containIndex = to - len;
    int nextIndex = this->length - 1;
    int foundIndex = -1;
    char *ptr = NULL;
    while (nextIndex >= 0) {
        if ((ptr = strstr(this->data + nextIndex, target)) != NULL && (foundIndex = ptr - this->data) <= containIndex) return foundIndex;
        nextIndex--;
    }
    return -1;
}

// [N, index1, index2, ... indexN]
int *String_findAll(String *this, char *target)
{
    int size = 128;
    int *result = (int *)pct_mallloc(sizeof(int) * size);
    for (size_t i = 0; i < size; i++) {
        result[i] = -1;
    }
    int foundIndex = -1;
    int nextIndex = 0;
    int lastRecorded = -1;
    while ((foundIndex = String_findNext(this, nextIndex, target)) >= 0) {
        nextIndex++;
        if (foundIndex != lastRecorded) {
            result[0]++;
            if (result[0] > size - 1) {
                size <<= 2;
                result = (int *)pct_realloc(result, size);
            }
            result[result[0]] = foundIndex;
            lastRecorded = foundIndex;
        }
    }
    return result;
}

int String_capacity(String *this)
{
    return this->capacity;
}

int String_length(String *this)
{
    return this->length;
}

char *String_get(String *this)
{
    return this->data;
}

String *String_set(String *this, char *str)
{
    String_delete(this, 0, -1);
    String_appendStr(this, str);
    return this;
}

void String_print(String *this)
{
    printf("<STRING:%d,%lu,%s>\n", this->length, String_hash(this), this->data);
}

char *String_dump(String *this)
{
    char *s = pct_mallloc(this->length + 1);
    memcpy(s, this->data, this->length + 1);
    return s;
}

String *String_clone(String *this)
{
    String *s = String_new();
    String_appendStr(s, String_get(this));
    return s;
}

String *String_format(char *template, ...)
{
    va_list lst;
    va_list lstCopy;
    va_start(lst, template);
    va_copy(lstCopy, lst);
    int bufsz = vsnprintf(NULL, 0, template, lst);
    char *t = pct_mallloc(bufsz + 1);
    vsnprintf(t, bufsz + 1, template, lstCopy);
    va_end(lst);
    va_end(lstCopy);
    String *s = String_new();
    String_appendStr(s, t);
    pct_free(t);
    return s;
}

String* String_repeat(String *this, int count)
{
    if (count <= 0 || this->length <= 0) return this;
    char *data = String_dump(this);
    while (--count > 0) {
        String_appendStr(this, data);
    }
    pct_free(data);
    return this;
}

String *String_subString(String *this, int from, int to)
{
    limit_range(this->length, true, &from, &to);
    String *s = String_new();
    int len = to - from + 1;
    char *temp = pct_mallloc(len + 1);
    memmove(temp, this->data + from, len);
    temp[len] = '\0';
    String_appendStr(s, temp);
    pct_free(temp);
    return s; 
}

int String_compare(String *this, String *that)
{
    return strcmp(this->data, that->data);
}

bool String_equal(String *this, String *that)
{
    return String_compare(this, that) == 0;
}

char String_getChar(String *this, int index)
{
    if (index < 0 || index >= this->length) return '\0';
    return this->data[index];
}

String *String_setChar(String *this, int index, char c)
{
    if (index < 0 || index >= this->length || c == '\0') return this;
    this->data[index] = c;
    return this;
}

String *String_reverse(String *this)
{
    int left = 0;
    int right = this->length - 1;
    char temp;
    while(left < right) {
        temp = this->data[left];
        this->data[left] = this->data[right];
        this->data[right] = temp;
        left++;
        right--;
    }
}

String *String_upper(String *this)
{
    int index = 0;
    while(index < this->length) this->data[index++] = toupper(this->data[index]);
}

String *String_lower(String *this)
{
    int index = 0;
    while(index < this->length) this->data[index++] = tolower(this->data[index]);
}

bool String_startsWith(String *this, char *target)
{
    return String_findNext(this, 0, target) == 0;
}

bool String_endsWith(String *this, char *target)
{
    return String_findLast(this, -1, target) == this->length - strlen(target);
}

bool String_contains(String *this, char *target)
{
    return String_findNext(this, 0, target) >= 0;
}

String *String_replace(String *this, char *target, char *relacement, int from, int to, int count)
{
    limit_range(this->length, true, &from, &to);
    int thisLen = String_length(this);
    int targetLen = strlen(target);
    int replacementLen = strlen(relacement);
    if (targetLen <= 0 || count == 0) return this;
    int findIndex = from;
    int fromIndex = 0;
    int foundIndex = -1;
    int replaceCount = 0;
    String *result = String_new();
    String *temp = NULL;
    while((foundIndex = String_findNext(this, findIndex, target)) >= 0) {
        if (foundIndex >= to) break;
        temp = String_subString(this, fromIndex, foundIndex - 1);
        String_append(result, String_get(temp));
        String_free(temp);
        String_append(result, relacement);
        fromIndex = foundIndex + targetLen;
        findIndex = foundIndex + targetLen;
        replaceCount++;
        if (count > 0 && replaceCount >= count) break;
    }
    temp = String_subString(this, fromIndex, this->length - 1);
    String_append(result, String_get(temp));
    String_free(temp);
    String_set(this, String_get(result));
    String_free(result);
    return this;
}

String *_string_trim_both(String *this, bool isLeft, bool isRight)
{
    if (this->length == 0) return this;
    if (isLeft) {
        int start = 0;
        while(start < this->length && isspace(this->data[start])) start++;
        if (start > 0)  String_deleteStarting(this, start);
    }
    if (isRight) {
        int end = this->length - 1;
        while(end >= 0 && isspace(this->data[end])) end--;
        if (end < this->length - 1) String_deleteEnding(this, end + 1);
    }
    return this;
}

String *String_trimLeft(String *this)
{
    return _string_trim_both(this, true, false);
}

String *String_trimRight(String *this)
{
    return _string_trim_both(this, false, true);
}

String *String_trim(String *this)
{
    return _string_trim_both(this, true, true);
}

#endif


// ./files/cursor.h

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


// ./files/hashkey.h

// Hashkey

#ifndef H_PCT_HASHKEY
#define H_PCT_HASHKEY

typedef struct _Hashkey {
    struct _Object;
    String *key;
    void *value;
    struct _Hashkey *next;
} Hashkey;


Hashkey *Hashkey_set(Hashkey *this, void *value)
{
    this->value = value;
    return this;
}

Hashkey *Hashkey_new(String *key, void *value)
{
    Hashkey *hashkey = (Hashkey *)pct_mallloc(sizeof(Hashkey));
    hashkey->key = key;
    hashkey->value = value;
    hashkey->next = NULL;
    Object_retain(key);
    Object_init(hashkey, PCT_OBJ_HASHKEY);
    return hashkey;
}

void Hashkey_free(void *_this)
{
    Hashkey *this = _this;
    Object_release(this->key);
    this->key = NULL;
    this->value = NULL;
    this->next = NULL;
    Object_free(this);
}

#endif


// ./files/hashmap.h

// hashmap

#ifndef H_PCT_HASHMAP
#define H_PCT_HASHMAP


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


// ./files/foliage.h

// token

#ifndef H_PCT_FOLIAGE
#define H_PCT_FOLIAGE

typedef struct _Foliage {
    struct _Object;
    void *data;
    void *left;
    void *right;
} Foliage;

void Foliage_init(void *_foliage, void *data)
{
    Foliage *foliage = _foliage;
    foliage->data = data;
    foliage->left = NULL;
    foliage->right = NULL;
}

Foliage *Foliage_new(void *data)
{
    Foliage *foliage = (Foliage *)pct_mallloc(sizeof(Foliage));
    Object_init(foliage, PCT_OBJ_FOLIAGE);
    Foliage_init(foliage, data);
    return foliage;
}

void Foliage_print(void *_this)
{
    if (_this == NULL)
    {
        printf("[(Foliage) => NULL]\n");
    }
    else
    {
        Foliage *this = _this;
        printf("[(Foliage) => address:%d, data:%d]\n", this, this->data);
    }
}

void Foliage_free(Foliage *this)
{
    this->left = NULL;
    this->right = NULL;
    if (this->data != NULL)
    {
        pct_free(this->data);
        this->data = NULL;
    }
    Object_free(this);
}

#endif


// ./files/block.h

// token

#ifndef H_PCT_BLOCK
#define H_PCT_BLOCK

typedef struct _Block {
    struct _Object;
    void *data;
    void *next;
    void *last;
} Block;

void Block_init(void *_block, void *data)
{
    Block *block = _block;
    block->data = data;
    block->next = NULL;
    block->last = NULL;
}

Block *Block_new(void *data)
{
    Block *block = (Block *)pct_mallloc(sizeof(Block));
    Object_init(block, PCT_OBJ_BLOCK);
    Block_init(block, data);
    return block;
}

void Block_print(void *_this)
{
    if (_this == NULL)
    {
        printf("[(Block) => NULL]\n");
    }
    else
    {
        Block *this = _this;
        printf("[(Block) => address:%d, data:%d]\n", this, this->data);
    }
}

void Block_link(void *_first, void *_second)
{
    Block *first = _first;
    Block *second = _second;
    first->next = second;
    second->last = first;
}

void Block_append(void *_this, void *_other)
{
    Block_link(_this, _other);
}

void Block_prepend(void *_this, void *_other)
{
    Block_link(_other, _this);
}

void Block_remove(void *_this)
{
    Block *this = _this;
        Block *next = this->next;
        Block *last = this->last;
    if (next != NULL && last != NULL)
    {
        last->next = next;
        next->last = last;
    }
    else if (next != NULL)
    {
        next->last = NULL;
    }
    else if (last != NULL)
    {
        last->next = NULL;
    }
    this->next = NULL;
    this->last = NULL;
}

void *Block_next(void *_this)
{
    Block *this = _this;
    return this->next;
}

void *Block_last(void *_this)
{
    Block *this = _this;
    return this->last;
}

void Block_free(void *_this)
{
    Block *this = _this;
    Block *tmpNext = this->next;
    Block *tmpLast =this->last;
    if (tmpNext != NULL) tmpNext->last = NULL;
    if (tmpLast != NULL) tmpLast->next = NULL;
    this->next = NULL;
    this->last = NULL;
    tmpNext = NULL;
    tmpLast = NULL;
    this->data = NULL;
    Object_free(this);
}

#endif


// ./files/chain.h

// chain

#ifndef H_PCT_CHAIN
#define H_PCT_CHAIN


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


// ./files/queue.h

// queue

#ifndef H_PCT_QUEUE
#define H_PCT_QUEUE


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


// ./files/stack.h

// stack

#ifndef H_PCT_STACK
#define H_PCT_STACK


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


// ./files/array.h

// array

#ifndef H_PCT_ARRAY
#define H_PCT_ARRAY

#define ARRAY_DEFAULT_CAPACITY 1024
#define ARRAY_INVALID_INDEX 0

typedef int (* ArraySortFunction)(void const*, void const*);
typedef bool (* ArrayFindFunction)(void const*);

typedef struct _Array {
    struct _Object;
    void **elements;
    int length;
    bool retain;
    int capacity;
    bool nullable;
} Array;

Array *Array_new(bool isRetainValue)
{
    Array *array = (Array *)pct_mallloc(sizeof(Array));
    Object_init(array, PCT_OBJ_ARRAY);
    array->retain = isRetainValue;
    array->capacity = ARRAY_DEFAULT_CAPACITY;
    array->length = 0;
    array->nullable = false;
    array->elements = (void *)pct_mallloc(sizeof(void *) * array->capacity);
    for (int i = array->length; i < array->capacity; i++) array->elements[i] = NULL;
    return array;
}

void Array_clear(Array *this)
{
    for (int i = 0; i < this->length; i++) {
        if (this->retain) {
            Object_release(this->elements[i]);
        }
        this->elements[i] = NULL;
    }
    this->length = 0;
}

void Array_free(Array *this)
{
    Array_clear(this);
    pct_free(this->elements);
    this->elements = NULL;
    Object_free(this);
}

bool _array_check_resize(Array *this, int length)
{
    if (length <= this->capacity) return true;
    int capacity = this->capacity;
    while (length > capacity) capacity = capacity + ARRAY_DEFAULT_CAPACITY;
    void **elements = pct_realloc(this->elements, sizeof(void *) * capacity);
    if (elements == NULL) return false;
    this->capacity = capacity;
    this->elements = elements;
    for (int i = this->length; i < this->capacity; i++) this->elements[i] = NULL;
    return true;
}

bool Array_set(Array *this, int index, void *element)
{
    if (!this->nullable && element == NULL) return false;
    if (index < 0 || (!this->nullable && index >= this->length)) return false;
    int length = index < this->length ? this->length : index + 1;
    bool isOk = _array_check_resize(this, length);
    if (!isOk) return false;
    if (this->elements[index] != NULL) {
        if (this->retain) {
            Object_release(this->elements[index]);
        }
    }
    if (this->retain) {
        Object_retain(element);
    }
    this->length = length;
    this->elements[index] = element;
    return true;
}

void *Array_get(Array *this, int index)
{
    if (index < 0 || index >= this->length) return NULL;
    return this->elements[index];
}

void *Array_getLast(Array *this)
{
    return Array_get(this, this->length - 1);
}

void *Array_getFirst(Array *this)
{
    return Array_get(this, 0);
}

void *Array_del(Array *this, int index)
{
    if (index < 0 || index >= this->length) return NULL;
    void *item = this->elements[index];
    for(int i = index; i < this->length; i++)
    {
        this->elements[i] = i != (this->length - 1) ? this->elements[i + 1] : NULL;
    }
    this->elements[this->length] = NULL;
    this->length = this->length - 1;
    if (this->retain) {
        Object_release(item);
    }
    return item;
}

void *Array_delLast(Array *this)
{
    return Array_del(this, this->length - 1);
}

void *Array_delFirst(Array *this)
{
    return Array_del(this, 0);
}

bool _array_insert(Array *this, int index, void *element, bool isBefore)
{
    if (!this->nullable && element == NULL) return false;
    // validate
    if (index < 0) {
        index = 0;
        isBefore = true;
    } else if (index >= this->length) {
        index = this->length - 1;
        isBefore = false;
    }
    if (this->retain) {
        Object_retain(element);
    }
    // 
    if (this->length == 0)
    {
        this->elements[0] = element;
        this->length = 1;
        return true;
    }
    // resize
    bool isOk = _array_check_resize(this, this->length + 1);
    if (!isOk) return false;
    // insert
    int i = this->length;
    int to = isBefore ? index : index + 1;
    for (i = this->length - 1; i >= to; i--)
    {
        this->elements[i + 1] = this->elements[i];
    }
    this->elements[to] = element;
    this->length = this->length + 1;
    return true;
}

bool Array_insertBefore(Array *this, int at, void *element)
{
    return _array_insert(this, at, element, true);
}

bool Array_insertAfter(Array *this, int at, void *element)
{
    return _array_insert(this, at, element, false);
}

bool Array_prepend(Array *this, void *element)
{
    return Array_insertBefore(this, 0, element);
}

bool Array_append(Array *this, void *element)
{
    return Array_insertAfter(this, this->length - 1, element);
}

bool Array_push(Array *this, void *element)
{
    return Array_insertAfter(this, this->length - 1, element);
}

void *Array_pop(Array *this)
{
    return Array_del(this, this->length - 1);
}

bool Array_unshift(Array *this, void *element)
{
    return Array_insertBefore(this, 0, element);
}

void *Array_shift(Array *this)
{
    return Array_del(this, 0);
}

// int compare(const void *num1, const void *num2) { return 0; }
void Array_sort(Array *this, ArraySortFunction func)
{
    qsort(this->elements, this->length, sizeof(void *), func);
}

// int search(const void *num2) { return true; }
int Array_find(Array *this, int from, int to, bool isReverse, ArrayFindFunction func)
{
    if (from < 0 || to > this->length || from >= to) return -1;
    int var = isReverse ? -1 : 1;
    if (isReverse) {
        int temp = from;
        from = to - 1;
        to = temp + 1;
    }
    void *item;
    bool result;
    for (int i = from; i < to; i + var) {
        item = Array_get(this, i);
        result = func(item);
        if (result) return i;
    }
    return -1;
}

Array *Array_slice(Array *this, int from, int to)
{
    Array *other = Array_new(this->retain);
    if (from < 0 || to > this->length || from >= to) return other;
    for (int i = from; i < to; i++) Array_append(other, Array_get(this, i));
    return other;
}

typedef void (*ARRAY_FOREACH_FUNC)(int, void *, void *);

void Array_foreachItem(Array *this, ARRAY_FOREACH_FUNC func, void *arg) {
    for (int i = 0; i < this->length; i++) {
        void *ptr = Array_get(this, i);
        func(i, ptr, arg);
    }
}

void _array_copy_to_other(int i, void *item, void *other) {
    Array_set(other, i, item);
}

void Array_copyTo(Array *this, Array *other)
{
    Array_foreachItem(this, _array_copy_to_other, other);
}

char *Array_toString(Array *this)
{
    return tools_format("<Array p:%p s:%i>", this, this->length);
}

#endif


// ./files/time.h

// time

#include <time.h>
#include <sys/time.h>

double time_clock()
{
    clock_t clockTime = clock();
    double seconds = (double)clockTime / CLOCKS_PER_SEC;
    return seconds;
}

double time_second() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec) + (double)(tv.tv_usec / 1000000.0);
}

int time_zone() {
    time_t currentTime = time(NULL);
    struct tm *localTime = localtime(&currentTime);
    int hour1 = localTime->tm_hour;
    struct tm *globalTime = gmtime(&currentTime);
    int hour2 = globalTime->tm_hour;
    int distance = hour1 - hour2;
    return distance;
}

char *time_time()
{
    time_t _time = time(NULL);
    char *date = ctime(&_time);
    return date;
}

// "%Y-%m-%d %H:%M:%S"
int time_convert_to_seconds(char *str)
{
    if (strlen(str) != 19) return -1;
    int year = atoi(str + 0);
    int month = atoi(str + 5);
    int day = atoi(str + 8);
    int hour = atoi(str + 11);
    int minute = atoi(str + 14);
    int second = atoi(str + 17);
    struct tm info = {0};
    info.tm_year = year - 1900;
    info.tm_mon = month - 1;
    info.tm_mday = day;
    info.tm_hour = hour;
    info.tm_min = minute;
    info.tm_sec = second;
    info.tm_isdst = -1;
    time_t  result = mktime(&info);
    return (int) result;
}

// "%Y-%m-%d %H:%M:%S"
char *time_convert_from_seconds(int seconds, char *format)
{
    time_t currentTime = seconds >= 0 ? seconds : time(NULL);
    struct tm *localTime = localtime(&currentTime);
    const int size = 30;
    char *data = malloc(size);
    strftime(data, size, format, localTime);
    return data;
}



// ./files/timer.h

// timer

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

// #define PCT_TIMER_DEBUG

typedef double (*TIMER_FUNC)(void *);

typedef struct _Timer {
    struct _Object;
    double time;
    void *data;
    void *next;
    TIMER_FUNC func;
} Timer;

typedef double (*TIMER_CLEAN)(void *);
typedef double (*TIMER_EACH)(void *);
Timer *_timer_queue_head = NULL;

Timer *_timer_insert(Timer *timer, double seconds) {
    double current = time_second();
    timer->time = current + seconds;
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_insert: %f + %f = %f  %p", current, seconds, timer->time, timer);
    #endif
    //
    if (_timer_queue_head == NULL) {
        _timer_queue_head = timer;
    } else if (timer->time < _timer_queue_head->time) {
        timer->next = _timer_queue_head;
        _timer_queue_head = timer;
    } else {
        Timer *current = _timer_queue_head;
        while (current->next != NULL) {
            Timer *next = current->next;
            if (timer->time < next->time) break;
            current = next;
        }
        timer->next = current->next;
        current->next = timer;
    }
    //
    return timer;
}

bool _timer_execute() {
    // none
    Timer *timer = _timer_queue_head;
    if (timer == NULL) return true;
    // info
    double current = time_second();
    double nearest = timer->time;
    void *data = timer->data;
    TIMER_FUNC func = timer->func;
    // waiting
    if (nearest > current) return true;
    // remove
    Timer *next = timer->next;
    timer->next = NULL;
    _timer_queue_head = next;
    // invalid
    if (func == NULL) return true;
    // run
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_execute: %f %p", timer->time, timer);
    #endif
    double seconds = func(data);
    // complete
    seconds > 0 ? _timer_insert(timer, seconds) : pct_free(timer);
    return false;
}

void timer_cancel(Timer *timer) {
    if (timer == NULL) return;
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_cancel: %f %p", timer->time, timer);
    #endif
    timer->data = NULL;
    timer->func = NULL;
}

Timer *timer_delay(double seconds, void *data, TIMER_FUNC func) {
    if (seconds < 0) {
        seconds = 0;
    } else if (seconds == 0) {
        seconds = 0.00001;
    }
    Timer *timer = (Timer *)pct_mallloc(sizeof(Timer));
    Object_init(timer, PCT_OBJ_TIMER);
    timer->data = data;
    timer->func = func;
    timer->next = NULL;
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_delay: %f %p", timer->time, timer);
    #endif
    return _timer_insert(timer, seconds);
}

void timer_clean(TIMER_CLEAN callback) {
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_clean");
    #endif
    Timer *current = _timer_queue_head;
    while (current != NULL) {
        Timer *next = current->next;
        if (callback != NULL) {
            callback(current->data);
        }
        timer_cancel(current);
        current->next = NULL;
        pct_free(current);
        current = next;
    }
    _timer_queue_head = NULL;
}

void timer_each(TIMER_EACH callback) {
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_each");
    #endif
    Timer *current = _timer_queue_head;
    while (current != NULL) {
        Timer *next = current->next;
        if (callback != NULL) {
            callback(current->data);
        }
        current = next;
    }
}

bool timer_check() {
    bool stop = false;
    while (!stop) stop = _timer_execute();
    bool finished = _timer_queue_head == NULL;
    return finished;
}

void timer_loop() {
    #ifdef PCT_TIMER_DEBUG
    log_debug("timer_loop:");
    #endif
    while(true) {
        bool finished = timer_check();
        if (finished) break;
    }
}

// 

#ifdef PCT_TIMER_DEBUG

Timer *testTimer = NULL;

double _timer_test_func(void *data) {
    double *_data = (double *)data;
    if (*_data == 2 && testTimer != NULL) {
        timer_cancel(testTimer);
        testTimer = NULL;
    }
    double delay = *_data == 2 ? 2 : -1;
    log_debug("timer_run... %f %f", *_data, delay);
    return delay;
}

void _timer_test_main() {
    log_debug("timer_test:");
    double a = 1;
    double b = 2;
    double c = 4;
    double d = 10;
    timer_delay(a, &a, &_timer_test_func);
    timer_delay(b, &b, &_timer_test_func);
    testTimer = timer_delay(c, &c, &_timer_test_func);
    timer_delay(d, &d, &_timer_test_func);
    timer_loop();
}

#endif


// ./files/helpers.h

// helpers

#ifndef H_PCT_HELPERS
#define H_PCT_HELPERS


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
