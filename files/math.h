// math

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

float math_lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

double math_quotient(double x, double y) {
    return (x - fmod(x, y)) / y;
}

double math_reminder(double x, double y) {
    return fmod(x, y);
}

double math_int_part(double num) {
    return num - modf(num, 0);
}

double math_dec_part(double num) {
    return modf(num, 0);
}

double math_radian(double degree) {
    return degree * M_PI / 180.0;
}

double math_degree(double radian) {
    return radian * 180.0 / M_PI;
}

