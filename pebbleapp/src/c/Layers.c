#include "Chronomarker.h"

static void prv_o2co2_draw(struct Layer *layer, GContext* ctx);
static void prv_effect_icon_draw(struct Layer *layer, GContext* ctx);

static GPoint s_innerPoints[COUNT_O2CO2]; // anti-clockwise
static GPoint s_outerPoints[COUNT_O2CO2]; // clock-wise
#define O2CO2_INNER_RADIUS 67
#define O2CO2_OUTER_RADIUS 84
#define O2CO2_BOUNDS GRect(7, 91, 166, 82)
#define GRectCenteredCircle(r) GRect(90 - r, 90 - r, r * 2, r * 2)

void o2co2_create(O2CO2Layer* layer, Layer* parentLayer)
{
    static bool calculatedPoints = false;
    static GBitmap *co2Bitmap = NULL, *o2Bitmap = NULL;
    if (!calculatedPoints)
    {
        calculatedPoints = true;
        GRect inner = GRectCenteredCircle(O2CO2_INNER_RADIUS);
        GRect outer = GRectCenteredCircle(O2CO2_OUTER_RADIUS);
        for (int i = 0; i < COUNT_O2CO2; i++)
        {
            s_innerPoints[i] = gpoint_from_polar(inner, GOvalScaleModeFillCircle,
                TRIG_MAX_ANGLE * 3 / 4 - TRIG_MAX_ANGLE * i / 2 / MAX_O2CO2);
            s_outerPoints[i] = gpoint_from_polar(outer, GOvalScaleModeFillCircle,
                TRIG_MAX_ANGLE * 1 / 4 + TRIG_MAX_ANGLE * i / 2 / MAX_O2CO2);

            s_innerPoints[i].x -= O2CO2_BOUNDS.origin.x;
            s_innerPoints[i].y -= O2CO2_BOUNDS.origin.y;
            s_outerPoints[i].x -= O2CO2_BOUNDS.origin.x;
            s_outerPoints[i].y -= O2CO2_BOUNDS.origin.y;
        }
        co2Bitmap = gbitmap_create_with_resource(RESOURCE_ID_CO2);
        o2Bitmap = gbitmap_create_as_sub_bitmap(co2Bitmap, GRect(9, 0, 14, 11));
    }

    memset(&layer->o2Path, 0, sizeof(GPath));
    memset(&layer->co2Path, 0, sizeof(GPath));
    layer->layer = layer_create_with_data(O2CO2_BOUNDS, sizeof(O2CO2Layer*));
    *(O2CO2Layer**)layer_get_data(layer->layer) = layer;
    layer_set_clips(layer->layer, false);
    layer_set_update_proc(layer->layer, prv_o2co2_draw);
    layer_add_child(parentLayer, layer->layer);

    layer->co2Text = bitmap_layer_create(GRect(152, 77, 23, 11));
    bitmap_layer_set_bitmap(layer->co2Text, co2Bitmap);
    bitmap_layer_set_compositing_mode(layer->co2Text, GCompOpSet);
    layer_add_child(parentLayer, bitmap_layer_get_layer(layer->co2Text));

    layer->o2Text = bitmap_layer_create(GRect(8, 77, 14, 11));
    bitmap_layer_set_bitmap(layer->o2Text, o2Bitmap);
    bitmap_layer_set_compositing_mode(layer->o2Text, GCompOpSet);
    layer_add_child(parentLayer, bitmap_layer_get_layer(layer->o2Text));
}

void o2co2_destroy(O2CO2Layer* layer)
{
    layer_destroy(layer->layer);
    bitmap_layer_destroy(layer->o2Text);
    bitmap_layer_destroy(layer->co2Text);
    layer->layer = NULL;
    layer->o2Text = NULL;
    layer->co2Text = NULL;
    layer->o2 = layer->co2 = 0;
    layer->o2Path.num_points = layer->co2Path.num_points = 0;
}

void o2co2_set_values(O2CO2Layer* layer, uint8_t o2, uint8_t co2)
{
    ASSERT(o2 <= MAX_O2CO2 && co2 <= MAX_O2CO2);
    if (layer->o2 == o2 && layer->co2 == co2)
        return;
    layer->o2 = o2;
    layer->co2 = co2;
    //if (o2 > 0 && co2 > 0)
    //    o2--, co2--;
    
    memcpy(layer->points, s_innerPoints, sizeof(GPoint) * o2);
    memcpy(layer->points + o2, s_outerPoints + COUNT_O2CO2 - o2, sizeof(GPoint) * o2);
    layer->o2Path.points = layer->points;
    layer->o2Path.num_points = o2 * 2;

    memcpy(layer->points + o2 * 2, s_innerPoints + COUNT_O2CO2 - co2, sizeof(GPoint) * co2);
    memcpy(layer->points + o2 * 2 + co2, s_outerPoints, sizeof(GPoint) * co2);
    layer->co2Path.points = layer->points + o2 * 2;
    layer->co2Path.num_points = co2 * 2;

    layer_mark_dirty(layer->layer);
}

static void prv_o2co2_draw(struct Layer *layerPbl, GContext* ctx)
{
    O2CO2Layer* layer = *(O2CO2Layer**)layer_get_data(layerPbl);
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, &layer->o2Path);
    graphics_context_set_fill_color(ctx, GColorRed);
    gpath_draw_filled(ctx, &layer->co2Path);
}

static GBitmap *s_iconsFull, *s_icons[EFFECT_ICON_COUNT];
static GBitmap *s_iconTri = NULL;

static const GPoint s_iconPositionSlots[] =
{
    {6, 54},
    {14, 33},
    {29, 18},
    {48, 8},
    {72, 3},

    {96, 7},
    {119, 17},
    {137, 32},
    {149, 54},

    // Alert position missing
};

static const GPoint s_iconOffsets[] =
{
    {3, 9},
    {5, 8},
    {5, 8},
    {4, 8},
    {5, 8},

    {5, 5},
    {5, 5},
    {5, 5},
    {5, 5},

    {6, 6}
};

static const GColor s_iconBgColors[] =
{
    {GColorRedARGB8},
    {GColorOrangeARGB8},
    {GColorVeryLightBlueARGB8},
    {GColorYellowARGB8},
    {GColorLavenderIndigoARGB8},
};
static const GColor s_iconHealingColor = {GColorKellyGreenARGB8};
static const GColor s_iconFgColors[] =
{
    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorLightGrayARGB8},
    {GColorWhiteARGB8},

    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorLightGrayARGB8},

    {GColorWhiteARGB8},
};

void effect_icon_create(EffectIconLayer* layer, Layer* parentLayer, int positionSlot)
{
    if (s_iconTri == NULL)
    {
        s_iconTri = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRI);
        s_iconsFull = gbitmap_create_with_resource(RESOURCE_ID_ICONS);
        GRect sub = GRect(0, 0, 13, 13);
        for (int i = 0; i < EFFECT_ICON_COUNT; i++)
        {
            s_icons[i] = gbitmap_create_as_sub_bitmap(s_iconsFull, sub);
            sub.origin.y += sub.size.h;
        }
    }

    ASSERT(positionSlot >= 0 && positionSlot < sizeof(s_iconPositionSlots) / sizeof(GPoint));
    layer->layer = layer_create_with_data(
        (GRect) { .origin = s_iconPositionSlots[positionSlot], .size = GSize(22, 22) },
        sizeof(EffectIcon));
    *(EffectIcon*)layer_get_data(layer->layer) = EFFECT_ICON_NONE;
    layer_set_hidden(layer->layer, true);
    layer_set_update_proc(layer->layer, prv_effect_icon_draw);
    layer_add_child(parentLayer, layer->layer);
}

void effect_icon_destroy(EffectIconLayer* layer)
{
    layer_destroy(layer->layer);
}

void effect_icon_set_icon(EffectIconLayer* layer, EffectIcon newIcon)
{
    EffectIcon* oldIconPtr = (EffectIcon*)layer_get_data(layer->layer);
    if (*oldIconPtr == newIcon)
        return;
    *oldIconPtr = newIcon;
    layer_set_hidden(layer->layer, newIcon == EFFECT_ICON_NONE);
    layer_mark_dirty(layer->layer);
}

static void prv_effect_icon_draw(Layer* layer, GContext* ctx)
{
    EffectIcon icon = *(EffectIcon*)layer_get_data(layer);
    if (icon == EFFECT_ICON_NONE)
        return;
    const bool isPersonal = icon < EFFECT_ICON_FIRST_ENVIRONMENTAL;
    const GColor bgColor = 
        icon == EFFECT_ICON_HEALING ? s_iconHealingColor
        : s_iconBgColors[(icon - 1) % 5];
    const GRect bounds = layer_get_bounds(layer);

    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    if (isPersonal)
    {
        gbitmap_get_palette(s_iconTri)[1] = bgColor;
        graphics_draw_bitmap_in_rect(ctx, s_iconTri, bounds);
    }
    else
    {
        graphics_context_set_fill_color(ctx, bgColor);
        graphics_fill_circle(ctx, GPoint(10, 10), 10);
    }

    GRect iconBounds = { .origin = s_iconOffsets[icon - 1], .size = GSize(13, 13) };
    gbitmap_get_palette(s_iconsFull)[1] = s_iconFgColors[icon - 1];
    graphics_draw_bitmap_in_rect(ctx, s_icons[icon - 1], iconBounds);
}
