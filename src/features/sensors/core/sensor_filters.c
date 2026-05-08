#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sensor_filters.h"

#define PREV_NUM 8

float filterMovingAverage(moving_avg_t *filter, float newValue)
{
    filter->sum -= filter->buffer[filter->index];
    filter->sum += newValue;
    filter->buffer[filter->index] = newValue;
    filter->index = (filter->index + 1) % PREV_NUM;

    return filter->sum / PREV_NUM;
}

float filterExponential(exp_filter_t *filter, float newValue)
{
    filter->prev = filter->alpha * newValue + (1 - filter->alpha) * filter->prev;
    return filter->prev;
}