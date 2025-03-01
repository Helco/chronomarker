#include "Chronomarker.h"

static void prv_o2co2_draw(struct Layer *layer, GContext* ctx);

static GPoint s_innerPoints[COUNT_O2CO2]; // anti-clockwise
static GPoint s_outerPoints[COUNT_O2CO2]; // clock-wise
#define O2CO2_INNER_RADIUS 67
#define O2CO2_OUTER_RADIUS 84
#define O2CO2_BOUNDS GRect(7, 91, 166, 82)
#define GRectCenteredCircle(r) GRect(90 - r, 90 - r, r * 2, r * 2)

void o2co2_create(O2CO2Layer* layer, Layer* parentLayer)
{
    static bool calculatedPoints = false;
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
    }

    memset(&layer->o2Path, 0, sizeof(GPath));
    memset(&layer->co2Path, 0, sizeof(GPath));
    layer->layer = layer_create_with_data(O2CO2_BOUNDS, sizeof(O2CO2Layer*));
    *(O2CO2Layer**)layer_get_data(layer->layer) = layer;
    layer_set_clips(layer->layer, false);
    layer_set_update_proc(layer->layer, prv_o2co2_draw);
    layer_add_child(parentLayer, layer->layer);
}

void o2co2_destroy(O2CO2Layer* layer)
{
    layer_destroy(layer->layer);
    layer->layer = NULL;
}

void o2co2_set_values(O2CO2Layer* layer, uint8_t o2, uint8_t co2)
{
    ASSERT(o2 <= MAX_O2CO2 && co2 <= MAX_O2CO2);
    if (layer->o2 == o2 && layer->co2 == co2)
        return;
    layer->o2 = o2;
    layer->co2 = co2;
    if (o2 > 0 && co2 > 0)
        o2--, co2--;
    
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
