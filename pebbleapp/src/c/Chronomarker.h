#pragma once
#include <pebble.h>

#if 1
#define ASSERT(x) while (!(x)) { \
    APP_LOG(APP_LOG_LEVEL_ERROR, "Assertion failed: %s", #x); \
    *((uint8_t*)NULL) = 0; }
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
    BitmapLayer *o2Text, *co2Text;
} O2CO2Layer;
void o2co2_create(O2CO2Layer* layer, Layer* parentLayer);
void o2co2_destroy(O2CO2Layer* layer);
void o2co2_set_values(O2CO2Layer* layer, uint8_t o2, uint8_t co2);

typedef enum EffectIcon
{
    EFFECT_ICON_NONE = 0,
    EFFECT_ICON_CARDIO,
    EFFECT_ICON_SKELETAL,
    EFFECT_ICON_NERVOUS,
    EFFECT_ICON_DIGESTIVE,
    EFFECT_ICON_MISC,
    EFFECT_ICON_RADIATION,
    EFFECT_ICON_THERMAL,
    EFFECT_ICON_AIRBORNE,
    EFFECT_ICON_CORROSIVE,
    EFFECT_ICON_HEALING,

    EFFECT_ICON_COUNT = EFFECT_ICON_HEALING - 1,
    EFFECT_ICON_FIRST_ENVIRONMENTAL = EFFECT_ICON_RADIATION
} EffectIcon;
typedef struct EffectIconLayer
{
    Layer* layer;
} EffectIconLayer;
void effect_icon_create(EffectIconLayer* layer, Layer* parentLayer, int positionSlot);
void effect_icon_destroy(EffectIconLayer* layer);
void effect_icon_set_icon(EffectIconLayer* layer, EffectIcon icon);

#define BITS_BODY (4)
#define MAX_BODY_LENGTH (1 << BITS_BODY)

typedef struct CurvedTextLayer
{
    Layer* layer;
    GBitmap* charBitmaps[MAX_BODY_LENGTH + 1];
    GRect charBounds[MAX_BODY_LENGTH];
    uint8_t charCount;
    bool rerender;
    char text[MAX_BODY_LENGTH];
} CurvedTextLayer;
void curved_text_create(CurvedTextLayer* layer, Layer* parentLayer);
void curved_text_destroy(CurvedTextLayer* layer);
void curved_text_set_text(CurvedTextLayer* layer, const char* text);
