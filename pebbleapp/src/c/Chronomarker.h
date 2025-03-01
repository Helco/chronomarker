#pragma once
#include <pebble.h>

#if defined DEBUG || defined _DEBUG
#define ASSERT(x) while (!(x)) { \
    APP_LOG(APP_LOG_LEVEL_ERROR, "Assertion failed: %s", #x); \
    *((void*)NULL) = 0; }
#else
#define ASSERT(x)
#endif

#define BITS_O2CO2 (6)
#define COUNT_O2CO2 (1 << BITS_O2CO2)
#define MAX_O2CO2 (COUNT_O2CO2 - 1)

typedef struct O2CO2Layer
{
    Layer* layer;
    uint8_t o2, co2;
    GPath o2Path;
    GPath co2Path;
    GPoint points[COUNT_O2CO2 * 2]; // shared between o2/co2
} O2CO2Layer;

void o2co2_create(O2CO2Layer* layer, Layer* parentLayer);
void o2co2_destroy(O2CO2Layer* layer);
void o2co2_set_values(O2CO2Layer* layer, uint8_t o2, uint8_t co2);
