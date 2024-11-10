
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

