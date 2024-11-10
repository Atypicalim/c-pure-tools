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
