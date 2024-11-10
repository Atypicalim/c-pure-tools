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
