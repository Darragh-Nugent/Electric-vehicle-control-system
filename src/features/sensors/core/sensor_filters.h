#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define PREV_NUM 8

typedef struct {
    float buffer[PREV_NUM];
    float sum;
    uint8_t index;
} moving_avg_t;

typedef struct {
    float alpha;
    float prev;
} exp_filter_t;

float filterMovingAverage(moving_avg_t *filter, float newValue);
float filterExponential(exp_filter_t *filter, float newValue);