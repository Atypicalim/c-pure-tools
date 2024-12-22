// json

// https://github.com/yinghaoyu/TinyJson

#include <stddef.h>  // size_t
#include <assert.h>  // assert()
#include <errno.h>   // errno, ERANGE
#include <math.h>    // HUGE_VAL
#include <stdio.h>   // sprintf()
#include <stdlib.h>  // NULL, strtod()
#include <string.h>  // memcpy()
#include <stdbool.h>

// JSON has six type of data
// null, bool, number, string, array, object

typedef enum {
  JSON_NULL,
  JSON_BOOLEAN,
  JSON_NUMBER,
  JSON_STRING,
  JSON_ARRAY,
  JSON_OBJECT
} JType;

typedef struct JValue JValue;
typedef struct JMember JMember;
#define JElement JValue

#define _JSON_MIN_CAPACITY 10

struct JValue
{
  union {
    struct {
      JMember *m;
      size_t size;
      size_t capacity;
    } o;  // object
    struct {
      JValue *e;
      size_t size;
      size_t capacity;
    } a;  // array
    struct {
      char *s;
      size_t len;
    } s;       // string
    double n;  // number
    bool b;  // bool
  } u;
  JType type;
};

struct JMember {
  char *k;       // key
  size_t klen;   // key len
  JValue v;  // value
};

#define _JSON_NULL "null"
#define _JSON_TRUE "true"
#define _JSON_FALSE "false"

enum {
  JSON_ERROR_OK = 0,
  JSON_ERROR_EXPECT_VALUE,  // none charactor
  JSON_ERROR_INVALID_VALUE,
  JSON_ERROR_ROOT_NOT_SINGULAR,  // over than one charactor
  JSON_ERROR_NUMBER_TOO_BIG,
  JSON_ERROR_MISS_QUOTATION_MARK,    // miss match
  JSON_ERROR_INVALID_STRING_ESCAPE,  // escape code error
  JSON_ERROR_INVALID_STRING_CHAR,
  JSON_ERROR_INVALID_UNICODE_HEX,
  JSON_ERROR_INVALID_UNICODE_SURROGATE,
  JSON_ERROR_MISS_COMMA_OR_SQUARE_BRACKET,
  JSON_ERROR_MISS_KEY,
  JSON_ERROR_MISS_COLON,
  JSON_ERROR_MISS_COMMA_OR_CURLY_BRACKET,
};

#define json_init(v)       \
  do {                     \
    (v)->type = JSON_NULL; \
  } while (0)


JValue json_new(JType type);
JType json_type(const JValue *v);
void json_print(const JValue *v);
void json_free(JValue *v);

void json_get_null(const JValue *v);
void json_set_null(JValue *v);

bool json_get_boolean(const JValue *v);
void json_set_boolean(JValue *v, bool b);

double json_get_number(const JValue *v);
void json_set_number(JValue *v, double n);

const char *json_get_string(const JValue *v, size_t *len);
void json_set_string(JValue *v, const char *s, size_t len);

size_t json_array_get_size(const JValue *v);
size_t json_array_get_capacity(const JValue *v);
JElement *json_array_get_index(const JValue *v, size_t index);
void json_array_set_index(JValue *v, size_t index, JElement *);
void json_array_clear(JValue *v);

size_t json_object_get_size(const JValue *v);
size_t json_object_get_capacity(const JValue *v);
JMember *json_object_get_index(const JValue *v, size_t index);
void json_object_set_index(JValue *v, size_t index, JMember *);
void json_object_clear(JValue *v);

bool json_is_equal(const JValue *lhs, const JValue *rhs);
void json_move(JValue *dst, JValue *src);
void json_copy(JValue *dst, const JValue *src);
void json_swap(JValue *lhs, JValue *rhs);

///////////////////////////////////////////////////////////////////

#ifndef JSON_ERROR_STACK_INIT_SIZE
#define JSON_ERROR_STACK_INIT_SIZE 256
#endif

#ifndef JSON_ERROR_STRINGIFY_INIT_SIZE
#define JSON_ERROR_STRINGIFY_INIT_SIZE 256
#endif

#define _JSON_EXPECT(c, ch)         \
  do {                        \
    assert(*c->json == (ch)); \
    c->json++;                \
  } while (0)

#define _JSON_IS0TO9(ch) ((ch) >= '0' && (ch) <= '9')
#define _JSON_IS1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define _JSON_PUT_CHR(c, ch) do { *(char *) json_context_push(c, sizeof(char)) = (ch); } while (0)
#define _JSON_PUT_STR(c, s, len) memcpy(json_context_push(c, len), s, len)

typedef struct
{
  const char *json;
  char *stack;
  size_t size, top;  // size表示栈的容量
} json_context;

// 进栈size个字符
static void *json_context_push(json_context *c, size_t size) {
    void *ret;
    assert(size > 0);
    if (c->top + size >= c->size) {
        if (c->size == 0) {
        c->size = JSON_ERROR_STACK_INIT_SIZE;
        }
        while (c->top + size >= c->size) {  // 扩容
        c->size += c->size >> 1; /* c->size * 1.5 */
        }
        c->stack = (char *) realloc(c->stack, c->size);
    }
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

// 出栈size个字符
static void *json_context_pop(json_context *c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
}

static void json_parse_whitespace(json_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int json_parse_literal(json_context *c, JValue *v, const char *literal, JType type) {
    size_t i;
    _JSON_EXPECT(c, literal[0]);
    // end with '\0', '\0' ASCII == 0
    for (i = 0; literal[i + 1]; i++) {
        if (c->json[i] != literal[i + 1]) {
        return JSON_ERROR_INVALID_VALUE;
        }
    }
    c->json += i;
    v->type = type;
    return JSON_ERROR_OK;
}

static int json_parse_boolean(json_context *c, JValue *v, const char *check, bool b) {
    const char *p = c->json;
    if (strcmp(p, check) == 1) {
        c->json += strlen(check);
        v->type = JSON_BOOLEAN;
        v->u.b = b;
        return JSON_ERROR_OK;
    }
    return JSON_ERROR_INVALID_VALUE;
}

static int json_parse_number(json_context *c, JValue *v) {
    const char *p = c->json;
    if (*p == '-') {  // 负数
        p++;
    }
    if (*p == '0') {  // 只有单个0，不能有前导0，比如0123
        p++;
    } else {
        // 一个 1-9
        if (!_JSON_IS1TO9(*p)) {
        return JSON_ERROR_INVALID_VALUE;
        }
        // 一个 1-9 再加上任意数量的 digit
        for (p++; _JSON_IS0TO9(*p); p++) {
        }
    }
    if (*p == '.') {
        p++;
        // 小数点后至少应有一个 digit
        if (!_JSON_IS0TO9(*p)) {
        return JSON_ERROR_INVALID_VALUE;
        }
        for (p++; _JSON_IS0TO9(*p); p++) {
        }
    }
    if (*p == 'e' || *p == 'E') {
        // 有指数部分
        p++;
        if (*p == '+' || *p == '-') {
        p++;
        }
        if (!_JSON_IS0TO9(*p)) {
        return JSON_ERROR_INVALID_VALUE;
        }
        for (p++; _JSON_IS0TO9(*p); p++) {
        }
    }
    errno = 0;
    v->u.n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return JSON_ERROR_NUMBER_TOO_BIG;
    }
    v->type = JSON_NUMBER;
    c->json = p;
    return JSON_ERROR_OK;
}

// 读取4位16进制数
static const char *json_parse_hex4(const char *p, unsigned *u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9') {
        *u |= ch - '0';
        } else if (ch >= 'A' && ch <= 'F') {
        *u |= ch - ('A' - 10);
        } else if (ch >= 'a' && ch <= 'f') {
        *u |= ch - ('a' - 10);
        } else {
        return NULL;
        }
    }
    return p;
    // 这种方案会错误接受"\u 123"不合法的JSON，因为 strtol() 会跳过开始的空白
    // 要解决的话，还需要检测第一个字符是否 [0-9A-Fa-f]，或者 !isspace(*p)
    // char *end;
    //*u = (unsigned) strtol(p, &end, 16);
    // return end == p + 4 ? end : NULL;
}

static void json_encode_utf8(json_context *c, unsigned u) {
    if (u <= 0x7F) {
        // 写进一个 char，为什么要做 x & 0xFF 这种操作呢？
        // 这是因为 u 是 unsigned 类型，一些编译器可能会警告这个转型可能会截断数据
        _JSON_PUT_CHR(c, u & 0xFF);
    } else if (u <= 0x7FF) {
        _JSON_PUT_CHR(c, 0xC0 | ((u >> 6) & 0xFF));
        _JSON_PUT_CHR(c, 0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        _JSON_PUT_CHR(c, 0xE0 | ((u >> 12) & 0xFF));
        _JSON_PUT_CHR(c, 0x80 | ((u >> 6) & 0x3F));
        _JSON_PUT_CHR(c, 0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        _JSON_PUT_CHR(c, 0xF0 | ((u >> 18) & 0xFF));
        _JSON_PUT_CHR(c, 0x80 | ((u >> 12) & 0x3F));
        _JSON_PUT_CHR(c, 0x80 | ((u >> 6) & 0x3F));
        _JSON_PUT_CHR(c, 0x80 | (u & 0x3F));
    }
}

#define STRING_ERROR(ret) \
    do {                  \
        c->top = head;    \
        return ret;       \
    } while (0)

static int json_parse_string_raw(json_context *c, char **str, size_t *len) {
    size_t head = c->top;
    unsigned u, u2;
    const char *p;
    _JSON_EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
        case '\"':
        // 匹配""
        *len = c->top - head;
        *str = json_context_pop(c, *len);
        c->json = p;
        return JSON_ERROR_OK;
        case '\\':
        // 转义字符
        switch (*p++) {
        case '\"':
            _JSON_PUT_CHR(c, '\"');
            break;
        case '\\':
            _JSON_PUT_CHR(c, '\\');
            break;
        case '/':
            _JSON_PUT_CHR(c, '/');
            break;
        case 'b':
            _JSON_PUT_CHR(c, '\b');
            break;
        case 'f':
            _JSON_PUT_CHR(c, '\f');
            break;
        case 'n':
            _JSON_PUT_CHR(c, '\n');
            break;
        case 'r':
            _JSON_PUT_CHR(c, '\r');
            break;
        case 't':
            _JSON_PUT_CHR(c, '\t');
            break;
        case 'u':
            if (!(p = json_parse_hex4(p, &u))) {
            STRING_ERROR(JSON_ERROR_INVALID_UNICODE_HEX);
            }
            // 如果第一个码点在0xD800 ~ 0xDBFF之间
            if (u >= 0xD800 && u <= 0xDBFF) {
            /* surrogate pair */
            // 应该伴随一个 U+DC00 ~ U+DFFF的低级代理项
            if (*p++ != '\\') {
                STRING_ERROR(JSON_ERROR_INVALID_UNICODE_SURROGATE);
            }
            if (*p++ != 'u') {
                STRING_ERROR(JSON_ERROR_INVALID_UNICODE_SURROGATE);
            }
            if (!(p = json_parse_hex4(p, &u2))) {
                STRING_ERROR(JSON_ERROR_INVALID_UNICODE_HEX);
            }
            if (u2 < 0xDC00 || u2 > 0xDFFF) {
                STRING_ERROR(JSON_ERROR_INVALID_UNICODE_SURROGATE);
            }
            // 计算真实的码点
            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
            }
            json_encode_utf8(c, u);
            break;
        default:
            STRING_ERROR(JSON_ERROR_INVALID_STRING_ESCAPE);
        }
        break;
        case '\0':
        // 不匹配""
        STRING_ERROR(JSON_ERROR_MISS_QUOTATION_MARK);
        default:
        // char 带不带符号，是实现定义的。
        // 如果编译器定义 char 为带符号的话，(unsigned char)ch >= 0x80 的字符，都会变成负数，并产生 json_PARSE_INVALID_STRING_CHAR 错误。
        // 我们现时还没有测试 ASCII 以外的字符，所以有没有转型至不带符号都不影响，但开始处理 Unicode 的时候就要考虑了
        if ((unsigned char) ch < 0x20) {
            STRING_ERROR(JSON_ERROR_INVALID_STRING_CHAR);
        }
        _JSON_PUT_CHR(c, ch);  // 把字符进栈
        }
    }
}

static int json_parse_string(json_context *c, JValue *v) {
    int ret;
    char *s;
    size_t len;
    if ((ret = json_parse_string_raw(c, &s, &len)) == JSON_ERROR_OK)
        json_set_string(v, s, len);
    return ret;
}

static int json_parse_value(json_context *c, JValue *v);

static int json_parse_array(json_context *c, JValue *v) {
    size_t size = 0;
    int i, ret;
    _JSON_EXPECT(c, '[');
    // 解析空白字符
    json_parse_whitespace(c);
    if (*c->json == ']') {
        // 数组内没有元素
        c->json++;
        v->type = JSON_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return JSON_ERROR_OK;
    }
    for (;;) {
        JValue e;
        json_init(&e);
        // buggy
        // 因为 json_parse_value() 及之下的函数都需要调用 json_context_push()
        // 而 json_context_push() 在发现栈满了的时候会用 realloc() 扩容。
        // 这时候，我们上层的 e 就会失效

        // JValue *e = json_context_push(c, sizeof(JValue));  // 可能会失效
        // json_init(e);
        // size++;
        // if ((ret = json_parse_value(c, e)) != json_PARSE_OK)  // 可能会realloc()
        //  return ret;

        if ((ret = json_parse_value(c, &e)) != JSON_ERROR_OK) {
        break;
        }
        // 解析空白字符
        json_parse_whitespace(c);
        memcpy(json_context_push(c, sizeof(JValue)), &e, sizeof(JValue));
        size++;
        if (*c->json == ',') {
        c->json++;
        // 解析空白字符
        json_parse_whitespace(c);
        } else if (*c->json == ']') {
        // 数组结束
        c->json++;
        v->type = JSON_ARRAY;
        v->u.a.size = size;
        // size 表示的是元素的数量
        size *= sizeof(JValue);
        memcpy(v->u.a.e = (JValue *) malloc(size), json_context_pop(c, size), size);
        return JSON_ERROR_OK;
        } else {
        // 不匹配 ']'
        ret = JSON_ERROR_MISS_COMMA_OR_SQUARE_BRACKET;
        break;
        }
    }
    /* Pop and free values on the stack */
    for (i = 0; i < size; i++) {
        json_free((JValue *) json_context_pop(c, sizeof(JValue)));
    }
    return ret;
}

static int json_parse_object(json_context *c, JValue *v) {
    size_t i, size;
    JMember m;
    int ret;
    _JSON_EXPECT(c, '{');
    json_parse_whitespace(c);
    if (*c->json == '}') {
        c->json++;
        v->type = JSON_OBJECT;
        v->u.o.m = 0;
        v->u.o.size = 0;
        return JSON_ERROR_OK;
    }
    m.k = NULL;
    size = 0;
    for (;;) {
        char *str;
        json_init(&m.v);
        /* parse key */
        if (*c->json != '"') {
            ret = JSON_ERROR_MISS_KEY;
            break;
        }
        if ((ret = json_parse_string_raw(c, &str, &m.klen)) != JSON_ERROR_OK) {
            break;
        }
        memcpy(m.k = (char *) malloc(m.klen + 1), str, m.klen);
        m.k[m.klen] = '\0';
        /* parse ws colon ws */
        json_parse_whitespace(c);
        if (*c->json != ':') {
            ret = JSON_ERROR_MISS_COLON;
            break;
        }
        c->json++;
        json_parse_whitespace(c);
        /* parse value */
        if ((ret = json_parse_value(c, &m.v)) != JSON_ERROR_OK) {
            break;
        }
        memcpy(json_context_push(c, sizeof(JMember)), &m, sizeof(JMember));
        size++;
        m.k = NULL; /* ownership is transferred to member on stack */
                    /* parse ws [comma | right-curly-brace] ws */
        json_parse_whitespace(c);
        if (*c->json == ',') {
            c->json++;
            json_parse_whitespace(c);
        } else if (*c->json == '}') {
            size_t s = sizeof(JMember) * size;
            c->json++;
            v->type = JSON_OBJECT;
            v->u.o.size = size;
            memcpy(v->u.o.m = (JMember *) malloc(s), json_context_pop(c, s), s);
            return JSON_ERROR_OK;
        } else {
            ret = JSON_ERROR_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    /* Pop and free members on the stack */
    free(m.k);
    for (i = 0; i < size; i++) {
        JMember *m = (JMember *) json_context_pop(c, sizeof(JMember));
        free(m->k);
        json_free(&m->v);
    }
    v->type = JSON_NULL;
    return ret;
}

static int json_parse_value(json_context *c, JValue *v) {
    switch (*c->json) {
    case 't':
        return json_parse_boolean(c, v, _JSON_TRUE, true);
    case 'f':
        return json_parse_boolean(c, v, _JSON_FALSE, true);
    case 'n':
        return json_parse_literal(c, v, _JSON_NULL, JSON_NULL);
    case '"':
        return json_parse_string(c, v);
    case '[':
        return json_parse_array(c, v);
    case '{':
        return json_parse_object(c, v);
    case '\0':
        return JSON_ERROR_EXPECT_VALUE;
    default:
        return json_parse_number(c, v);
    }
}

static void json_stringify_string(json_context *c, const char *s, size_t len) {
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    size_t i, size;
    char *head, *p;
    assert(s != NULL);
    p = head = json_context_push(c, size = len * 6 + 2); /* "\u00xx..." */
    *p++ = '"';
    for (i = 0; i < len; i++) {
        unsigned char ch = (unsigned char) s[i];
        switch (ch) {
        case '\"':
        *p++ = '\\';
        *p++ = '\"';
        break;
        case '\\':
        *p++ = '\\';
        *p++ = '\\';
        break;
        case '\b':
        *p++ = '\\';
        *p++ = 'b';
        break;
        case '\f':
        *p++ = '\\';
        *p++ = 'f';
        break;
        case '\n':
        *p++ = '\\';
        *p++ = 'n';
        break;
        case '\r':
        *p++ = '\\';
        *p++ = 'r';
        break;
        case '\t':
        *p++ = '\\';
        *p++ = 't';
        break;
        default:
        if (ch < 0x20) {
            *p++ = '\\';
            *p++ = 'u';
            *p++ = '0';
            *p++ = '0';
            *p++ = hex_digits[ch >> 4];
            *p++ = hex_digits[ch & 15];
        } else {
            *p++ = s[i];
        }
        }
    }
    *p++ = '"';
    c->top -= size - (p - head);
}

static void json_stringify_value(json_context *c, const JValue *v) {
    size_t i;
    switch (v->type) {
    case JSON_NULL:
        _JSON_PUT_STR(c, _JSON_NULL, strlen(_JSON_NULL));
        break;
    case JSON_BOOLEAN:
        if (v->u.b > 0) {
        _JSON_PUT_STR(c, _JSON_TRUE, strlen(_JSON_TRUE));
        } else {
        _JSON_PUT_STR(c, _JSON_FALSE, strlen(_JSON_FALSE));
        }
        break;
    case JSON_NUMBER:
        c->top -= 32 - sprintf(json_context_push(c, 32), "%.17g", v->u.n);
        break;
    case JSON_STRING:
        json_stringify_string(c, v->u.s.s, v->u.s.len);
        break;
    case JSON_ARRAY:
        _JSON_PUT_CHR(c, '[');
        for (i = 0; i < v->u.a.size; i++) {
        if (i > 0)
            _JSON_PUT_CHR(c, ',');
        json_stringify_value(c, &v->u.a.e[i]);
        }
        _JSON_PUT_CHR(c, ']');
        break;
    case JSON_OBJECT:
        _JSON_PUT_CHR(c, '{');
        for (i = 0; i < v->u.o.size; i++) {
        if (i > 0)
            _JSON_PUT_CHR(c, ',');
        json_stringify_string(c, v->u.o.m[i].k, v->u.o.m[i].klen);
        _JSON_PUT_CHR(c, ':');
        json_stringify_value(c, &v->u.o.m[i].v);
        }
        _JSON_PUT_CHR(c, '}');
        break;
    default:
        assert(0 && "invalid type");
    }
}

void json_copy(JValue *dst, const JValue *src) {
    assert(src != NULL && dst != NULL && src != dst);
    switch (src->type) {
    case JSON_STRING:
        json_set_string(dst, src->u.s.s, src->u.s.len);
        break;
    case JSON_ARRAY:
        /* \todo */
        break;
    case JSON_OBJECT:
        /* \todo */
        break;
    default:
        json_free(dst);
        memcpy(dst, src, sizeof(JValue));
        break;
    }
}

void json_move(JValue *dst, JValue *src) {
    assert(dst != NULL && src != NULL && src != dst);
    json_free(dst);
    memcpy(dst, src, sizeof(JValue));
    json_init(src);
}

void json_swap(JValue *lhs, JValue *rhs) {
    assert(lhs != NULL && rhs != NULL);
    if (lhs != rhs) {
        JValue temp;
        memcpy(&temp, lhs, sizeof(JValue));
        memcpy(lhs, rhs, sizeof(JValue));
        memcpy(rhs, &temp, sizeof(JValue));
    }
}

bool json_is_equal(const JValue *lhs, const JValue *rhs) {
    size_t i;
    assert(lhs != NULL && rhs != NULL);
    if (lhs->type != rhs->type)
        return false;
    switch (lhs->type) {
    case JSON_STRING:
        return lhs->u.s.len == rhs->u.s.len && memcmp(lhs->u.s.s, rhs->u.s.s, lhs->u.s.len) == 0;
    case JSON_NUMBER:
        return lhs->u.n == rhs->u.n;
    case JSON_ARRAY:
        if (lhs->u.a.size != rhs->u.a.size)
        return false;
        for (i = 0; i < lhs->u.a.size; i++)
        if (!json_is_equal(&lhs->u.a.e[i], &rhs->u.a.e[i])) {
            return false;
        } else {
            return true;
        }
    case JSON_OBJECT:
        /* \todo */
        return true;
    default:
        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////

JValue json_new(JType type) {
    JValue v;
    json_init(&v);
    v.type = type;
    v.u.b = 0;
    return v;
}

JType json_type(const JValue *v) {
    assert(v != NULL);
    return v->type;
}

void json_free(JValue *v) {
    size_t i;
    assert(v != NULL);
    switch (v->type) {
    case JSON_STRING:
        free(v->u.s.s);
        break;
    case JSON_ARRAY:
        for (i = 0; i < v->u.a.size; i++) {
        json_free(&v->u.a.e[i]);
        }
        free(v->u.a.e);
        break;
    case JSON_OBJECT:
        for (i = 0; i < v->u.o.size; i++) {
        free(v->u.o.m[i].k);
        json_free(&v->u.o.m[i].v);
        }
        free(v->u.o.m);
        break;
    default:
        break;
    }
    v->type = JSON_NULL;
}

/////////////////////////////////////////////////////////////////////////////

JValue json_new_null() {
    JValue v;
    json_init(&v);
    v.type = JSON_NULL;
    return v;
}

void json_get_null(const JValue *v) {
    assert(v != NULL && v->type == JSON_NULL);
}

void json_set_null(JValue *v) {
    json_free(v);
    v->type = JSON_NULL;
}

JValue json_new_boolean(bool b) {
    JValue v;
    json_init(&v);
    v.type = JSON_BOOLEAN;
    v.u.b = b;
    return v;
}

bool json_get_boolean(const JValue *v) {
    assert(v != NULL && v->type == JSON_BOOLEAN);
    return v->u.b > 0;
}

void json_set_boolean(JValue *v, bool b) {
    json_free(v);
    v->type = JSON_BOOLEAN;
    v->u.b = b;
}

JValue json_new_number(double n) {
    JValue v;
    json_init(&v);
    v.type = JSON_NUMBER;
    v.u.n = n;
    return v;
}

double json_get_number(const JValue *v) {
    assert(v != NULL && v->type == JSON_NUMBER);
    return v->u.n;
}

void json_set_number(JValue *v, double n) {
    json_free(v);
    v->type = JSON_NUMBER;
    v->u.n = n;
}

/////////////////////////////////////////////////////////////////////////////


void __json_set_text_with_length(JValue *v, const char *s, size_t l) {
    v->u.s.s = (char *) malloc(l + 1);
    memcpy(v->u.s.s, s, l);
    v->u.s.s[l] = '\0';
    v->u.s.len = l;
}

JValue json_new_string(char *s, size_t l) {
    JValue v;
    json_init(&v);
    v.type = JSON_STRING;
    __json_set_text_with_length(&v, s, l);
}

const char *json_get_string(const JValue *v, size_t *l) {
    assert(v != NULL && v->type == JSON_STRING);
    if (*l != 0) *l = v->u.s.len;
    return v->u.s.s;
}

void json_set_string(JValue *v, const char *s, size_t l) {
    assert(v != NULL && (s != NULL || l == 0));
    json_free(v);
    v->type = JSON_STRING;
    __json_set_text_with_length(v, s, l);
}


/////////////////////////////////////////////////////////////////////////////

void _json_member_free(JMember *m) {
    json_free(&m->v);
    free(m->k);
}

void _json_element_free(JElement *m) {
    json_free(m);
}

void _json_array_check_resize(JValue *v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    if (v->u.a.size >= v->u.a.capacity) {
        v->u.a.capacity = MAX(_JSON_MIN_CAPACITY, v->u.a.size * 1.5);
        v->u.a.e = (JValue *) realloc(v->u.a.e, v->u.a.capacity * sizeof(JValue));
    } else if (v->u.a.size < v->u.a.capacity / 2) {
        v->u.a.capacity = MAX(_JSON_MIN_CAPACITY, v->u.a.size / 2);
        v->u.a.e = (JValue *) realloc(v->u.a.e, v->u.a.capacity * sizeof(JValue));
    }
}

void _json_object_check_resize(JValue *v) {
    assert(v != NULL && v->type == JSON_OBJECT);
    if (v->u.o.size >= v->u.o.capacity) {
        v->u.o.capacity = MAX(_JSON_MIN_CAPACITY, v->u.o.size * 1.5);
        v->u.o.m = (JMember *) realloc(v->u.o.m, v->u.o.capacity * sizeof(JMember));
    } else if (v->u.o.size < v->u.o.capacity / 2) {
        v->u.o.capacity = MAX(_JSON_MIN_CAPACITY, v->u.o.size / 2);
        v->u.o.m = (JMember *) realloc(v->u.o.m, v->u.o.capacity * sizeof(JMember));
    }
}

void json_set_array(JValue *v, size_t capacity) {
    assert(v != NULL);
    json_free(v);
    v->type = JSON_ARRAY;
    v->u.a.size = 0;
    v->u.a.capacity = capacity;
    v->u.a.e = capacity > 0 ? (JValue *) malloc(capacity * sizeof(JValue)) : NULL;
}

void json_set_object(JValue *v, size_t capacity) {
    assert(v != NULL);
    json_free(v);
    v->type = JSON_OBJECT;
    v->u.o.size = 0;
    v->u.o.capacity = capacity;
    v->u.o.m = capacity > 0 ? (JMember *) malloc(capacity * sizeof(JMember)) : NULL;
}

/////////////////////////////////////////////////////////////////////////////

void json_object_clear(JValue *v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    if (v->u.o.size <= 0) return;
    for (size_t i = 0; i < v->u.o.size; i++) {
        _json_member_free(&v->u.o.m[i]);
    }
    v->u.o.size = 0;
    _json_object_check_resize(v);
}

size_t json_object_get_size(const JValue *v) {
    assert(v != NULL && v->type == JSON_OBJECT);
    return v->u.o.size;
}

size_t json_object_get_capacity(const JValue *v) {
    assert(v != NULL && v->type == JSON_OBJECT);
    return v->u.o.capacity;
}

void json_clear_object(JValue *v) {
    assert(v != NULL && v->type == JSON_OBJECT);
    /* \todo */
}

JMember *json_object_get_index(const JValue *v, size_t index) {
    assert(v != NULL && v->type == JSON_OBJECT);
    assert(index < v->u.o.size);
    return &v->u.o.m[index];
}

void json_object_del_index(JValue *v, size_t index) {
    assert(v != NULL && v->type == JSON_OBJECT);
    if (v->u.a.size <= 0) return;
    assert(index < v->u.o.size);
    _json_member_free(&v->u.o.m[index]);
    for (size_t i = index; i < v->u.o.size-1; i++) {
        v->u.o.m[i] = v->u.o.m[i+1];
    }
    // v->u.o.m[v->u.o.size-1] = NULL;
    v->u.o.size--;
}

void json_object_set_index(JValue *v, size_t index, JMember *member) {
    assert(v != NULL && v->type == JSON_OBJECT);
    if (v->u.o.size <= 0) return;
    assert(index < v->u.o.size);
    if (member == NULL) {
        json_object_del_index(v, index);
    } else {
        _json_member_free(&v->u.o.m[index]);
        v->u.o.m[index] = *member;
    }
}

char *json_object_get_index_key(const JValue *v, size_t index) {
    assert(index < v->u.o.size);
    JMember *member = json_object_get_index(v, index);
    return member->k;
}

JValue *json_object_get_index_value(const JValue *v, size_t index) {
    assert(index < v->u.o.size);
    JMember *member = json_object_get_index(v, index);
    return &member->v;
}

size_t json_object_find_key_index(const JValue *v, const char *key) {
    size_t i;
    assert(v != NULL && v->type == JSON_OBJECT && key != NULL);
    size_t len = strlen(key);
    for (i = 0; i < v->u.o.size; i++)
        if (v->u.o.m[i].klen == len && memcmp(v->u.o.m[i].k, key, len) == 0){
        return i;
        }
    return -1;
}

JValue *json_object_find_key_value(JValue *v, const char *key) {
    size_t index = json_object_find_key_index(v, key);
    return index != -1 ? json_object_get_index_value(v, index) : NULL;
}

void json_object_add_member(JValue *v, char *key, JValue *val) {
    assert(v != NULL && v->type == JSON_ARRAY);
    _json_object_check_resize(v);
    json_init(&v->u.o.m[v->u.o.size].v);
    JMember m;
    m.k = key;
    m.klen = strlen(key);
    m.v = *val;
    v->u.o.m[v->u.o.size++] = m;
}

/////////////////////////////////////////////////////////////////////////////

void json_array_clear(JValue *v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    if (v->u.a.size <= 0) return;
    for (size_t i = 0; i < v->u.a.size; i++) {
        _json_element_free(&v->u.a.e[i]);
    }
    v->u.a.size = 0;
    _json_array_check_resize(v);
}

size_t json_array_get_size(const JValue *v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    return v->u.a.size;
}

size_t json_array_get_capacity(const JValue *v) {
    assert(v != NULL && v->type == JSON_ARRAY);
    return v->u.a.capacity;
}

JValue *json_array_get_index(const JValue *v, size_t index) {
    assert(v != NULL && v->type == JSON_ARRAY);
    assert(index < v->u.a.size);
    return &v->u.a.e[index];
}

void json_array_del_index(JValue *v, size_t index) {
    assert(v != NULL && v->type == JSON_ARRAY);
    if (v->u.a.size <= 0) return;
    assert(index < v->u.a.size);
    _json_element_free(&v->u.a.e[index]);
    for (size_t i = index; i < v->u.a.size-1; i++) {
        v->u.a.e[i] = v->u.a.e[i+1];
    }
    // v->u.a.e[v->u.a.size-1] = NULL;
    v->u.a.size--;
}

void json_array_set_index(JValue *v, size_t index, JElement *element) {
    assert(v != NULL && v->type == JSON_ARRAY);
    if (v->u.a.size <= 0) return;
    assert(index < v->u.a.size);
    if (element == NULL) {
        json_array_del_index(v, index);
    } else {
        _json_element_free(&v->u.a.e[index]);
        v->u.a.e[index] = *element;
    }
}

void json_array_add_element(JValue *v, JValue *val) {
    assert(v != NULL && v->type == JSON_ARRAY);
    _json_array_check_resize(v);
    json_init(&v->u.a.e[v->u.a.size]);
    JElement *element = val;
    v->u.a.e[v->u.a.size++] = *element;
}

/////////////////////////////////////////////////////////////////////////////

int json_decode(JValue *v, const char *json) {
    int ret;
    json_context c;
    assert(v != NULL);
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;
    json_init(v);
    json_parse_whitespace(&c);
    if ((ret = json_parse_value(&c, v)) == JSON_ERROR_OK) {
        json_parse_whitespace(&c);
        if (*c.json != '\0') {
        v->type = JSON_NULL;
        ret = JSON_ERROR_ROOT_NOT_SINGULAR;
        }
    }
    assert(c.top == 0);
    free(c.stack);
    return ret;
}

int json_encode(char **json, const JValue *v) {
    json_context c;
    assert(v != NULL);
    c.stack = (char *) malloc(c.size = JSON_ERROR_STRINGIFY_INIT_SIZE);
    c.top = 0;
    json_stringify_value(&c, v);
    // length = c.top;
    _JSON_PUT_CHR(&c, '\0');
    *json = c.stack;
    return JSON_ERROR_OK;
}

/////////////////////////////////////////////////////////////////////////////

#define _JSON_PREFIX " "

void _json_print(const JValue *, char *);
void _json_print(const JValue *v, char *prefix) {
    if (v == NULL) return;
    // 
    char *_prefix = malloc(strlen(prefix) + strlen(_JSON_PREFIX) + 1);
    strcpy(_prefix, prefix);
    strcat(_prefix, _JSON_PREFIX);
    // 
    switch (v->type) {
        case JSON_NULL:
            printf("%s,\n", _JSON_NULL);
            break;
        case JSON_BOOLEAN:
            printf("%s,\n", v->u.b ? _JSON_TRUE : _JSON_FALSE);
            break;
        case JSON_NUMBER:
            double n = v->u.n;
            if (n == (int)n) {
                printf("%d,\n", (int)n);
            } else {
                printf("%f,\n", (float)n);
            }
            break;
        case JSON_STRING:
            printf("\"%s\",\n", v->u.s.s);
            break;
        case JSON_ARRAY:
            printf("[\n");
            for (int i = 0; i < v->u.a.size; i++) {
                printf("%s%d: ", _prefix, i);
                _json_print(&v->u.a.e[i], _prefix);
            }
            printf("%s],\n", prefix);
            break;
        case JSON_OBJECT:
            printf("{\n");
            for (int i = 0; i < v->u.o.size; i++) {
                printf("%s%s: ", _prefix, v->u.o.m[i].k);
                _json_print(&v->u.o.m[i].v, _prefix);
            }
            printf("%s},\n", prefix);
            break;
        default:
            break;
    }
    free(_prefix);
}

void json_print(const JValue *v) {
    _json_print(v, _JSON_PREFIX);
}

/////////////////////////////////////////////////////////////////////////////
