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

