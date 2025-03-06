#include "Chronomarker.h"

static void prv_o2co2_draw(struct Layer *layer, GContext* ctx);
static void prv_effect_icon_draw(struct Layer *layer, GContext* ctx);
static void prv_curved_text_draw(struct Layer *layer, GContext* ctx);
static void prv_planet_draw(struct Layer* layer, GContext* ctx);
static void prv_scan_decoration_crescent_draw(struct Layer* layer, GContext* ctx);
static void prv_scan_decoration_center_draw(struct Layer* layer, GContext* ctx);
static void prv_alert_background_draw(struct Layer* layer, GContext* ctx);
static void prv_app_status_draw(struct Layer* layer, GContext* ctx);

static GPoint s_innerPoints[COUNT_O2CO2]; // anti-clockwise
static GPoint s_outerPoints[COUNT_O2CO2]; // clock-wise
static GPoint s_innerPointsSmall[COUNT_O2CO2]; // anti-clockwise
static GPoint s_outerPointsSmall[COUNT_O2CO2]; // clock-wise
#define O2CO2_BOUNDS GRect(0, 91, 180, 82)
#define GRectCenteredCircle(r) GRect(90 - r, 90 - r, r * 2, r * 2)

static void prv_o2co2_calc_points(GPoint* innerPoints, GPoint* outerPoints, int innerRadius, int outerRadius)
{
    GRect inner = GRectCenteredCircle(innerRadius);
    GRect outer = GRectCenteredCircle(outerRadius);
    for (int i = 0; i < COUNT_O2CO2; i++)
    {
        innerPoints[i] = gpoint_from_polar(inner, GOvalScaleModeFillCircle,
            TRIG_MAX_ANGLE * 3 / 4 - TRIG_MAX_ANGLE * i / 2 / MAX_O2CO2);
        outerPoints[i] = gpoint_from_polar(outer, GOvalScaleModeFillCircle,
            TRIG_MAX_ANGLE * 1 / 4 + TRIG_MAX_ANGLE * i / 2 / MAX_O2CO2);

        innerPoints[i].x -= O2CO2_BOUNDS.origin.x;
        innerPoints[i].y -= O2CO2_BOUNDS.origin.y;
        outerPoints[i].x -= O2CO2_BOUNDS.origin.x;
        outerPoints[i].y -= O2CO2_BOUNDS.origin.y;
    }
}

void o2co2_create(O2CO2Layer* layer, bool big, Layer* parentLayer)
{
    static bool calculatedPoints = false;
    static GBitmap *co2Bitmap = NULL, *o2Bitmap = NULL;
    if (!calculatedPoints)
    {
        calculatedPoints = true;
        prv_o2co2_calc_points(s_innerPoints, s_outerPoints, 67, 84);
        prv_o2co2_calc_points(s_innerPointsSmall, s_outerPointsSmall, 85, 92);
        co2Bitmap = gbitmap_create_with_resource(RESOURCE_ID_CO2);
        o2Bitmap = gbitmap_create_as_sub_bitmap(co2Bitmap, GRect(9, 0, 14, 11));
    }

    layer->big = big;
    memset(&layer->o2Path, 0, sizeof(GPath));
    memset(&layer->co2Path, 0, sizeof(GPath));
    layer->layer = layer_create_with_data(O2CO2_BOUNDS, sizeof(O2CO2Layer*));
    *(O2CO2Layer**)layer_get_data(layer->layer) = layer;
    layer_set_clips(layer->layer, false);
    layer_set_update_proc(layer->layer, prv_o2co2_draw);
    layer_add_child(parentLayer, layer->layer);

    if (big)
    {
        layer->co2Text = bitmap_layer_create(GRect(152, 77, 23, 11));
        bitmap_layer_set_bitmap(layer->co2Text, co2Bitmap);
        bitmap_layer_set_compositing_mode(layer->co2Text, GCompOpSet);
        layer_add_child(parentLayer, bitmap_layer_get_layer(layer->co2Text));

        layer->o2Text = bitmap_layer_create(GRect(8, 77, 14, 11));
        bitmap_layer_set_bitmap(layer->o2Text, o2Bitmap);
        bitmap_layer_set_compositing_mode(layer->o2Text, GCompOpSet);
        layer_add_child(parentLayer, bitmap_layer_get_layer(layer->o2Text));
    }
    else
        layer->o2Text = layer->co2Text = NULL;
}

void o2co2_destroy(O2CO2Layer* layer)
{
    layer_destroy(layer->layer);
    if (layer->big)
    {
        bitmap_layer_destroy(layer->o2Text);
        bitmap_layer_destroy(layer->co2Text);
    }
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

    const GPoint* innerPoints = layer->big ? s_innerPoints : s_innerPointsSmall;
    const GPoint* outerPoints = layer->big ? s_outerPoints : s_outerPointsSmall;
    
    memcpy(layer->points, innerPoints, sizeof(GPoint) * o2);
    memcpy(layer->points + o2, outerPoints + COUNT_O2CO2 - o2, sizeof(GPoint) * o2);
    layer->o2Path.points = layer->points;
    layer->o2Path.num_points = o2 * 2;

    memcpy(layer->points + o2 * 2, innerPoints + COUNT_O2CO2 - co2, sizeof(GPoint) * co2);
    memcpy(layer->points + o2 * 2 + co2, outerPoints, sizeof(GPoint) * co2);
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

// ------------------------------------------------------------------------------------------------

static GBitmap *s_iconsFull, *s_icons[EFFECT_ICON_COUNT];
static GBitmap *s_iconTri = NULL, *s_iconTriBig = NULL;

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

    {6, 4}
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
    {GColorDarkGrayARGB8},
    {GColorWhiteARGB8},

    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorWhiteARGB8},
    {GColorDarkGrayARGB8},

    {GColorWhiteARGB8},
};

void effect_icon_create(EffectIconLayer* layer, Layer* parentLayer, int positionSlot)
{
    if (s_iconTri == NULL)
    {
        s_iconTri = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRI);
        s_iconTriBig = gbitmap_create_with_resource(RESOURCE_ID_ICON_TRI_BIG);
        s_iconsFull = gbitmap_create_with_resource(RESOURCE_ID_ICONS);
        GRect sub = GRect(0, 0, 13, 13);
        for (int i = 0; i < EFFECT_ICON_COUNT; i++)
        {
            s_icons[i] = gbitmap_create_as_sub_bitmap(s_iconsFull, sub);
            sub.origin.y += sub.size.h;
        }
    }

    ASSERT(positionSlot >= 0 && positionSlot < (int)(sizeof(s_iconPositionSlots) / sizeof(GPoint)));
    layer->big = false;
    layer->icon = EFFECT_ICON_NONE;
    layer->layer = layer_create_with_data(
        (GRect) { .origin = s_iconPositionSlots[positionSlot], .size = GSize(22, 22) },
        sizeof(EffectIconLayer*));
    *(EffectIconLayer**)layer_get_data(layer->layer) = layer;
    layer_set_hidden(layer->layer, true);
    layer_set_update_proc(layer->layer, prv_effect_icon_draw);
    layer_add_child(parentLayer, layer->layer);
}

void effect_icon_create_big(EffectIconLayer* layer, Layer* parentLayer, bool down)
{
    layer->big = true;
    layer->icon = EFFECT_ICON_NONE;
    layer->layer = layer_create_with_data(GRect(75, 8 + down * 12, 30, 30), sizeof(EffectIconLayer*));
    *(EffectIconLayer**)layer_get_data(layer->layer) = layer;
    layer_set_hidden(layer->layer, true);
    layer_set_clips(layer->layer, false);
    layer_set_update_proc(layer->layer, prv_effect_icon_draw);
    layer_add_child(parentLayer, layer->layer);
}

void effect_icon_destroy(EffectIconLayer* layer)
{
    layer_destroy(layer->layer);
}

void effect_icon_set_icon(EffectIconLayer* layer, EffectIcon newIcon)
{
    if (layer->icon == newIcon)
        return;
    layer->icon = newIcon;
    layer_set_hidden(layer->layer, newIcon == EFFECT_ICON_NONE);
    layer_mark_dirty(layer->layer);
}

static void prv_effect_icon_draw(Layer* layerPbl, GContext* ctx)
{
    EffectIconLayer* layer = *(EffectIconLayer**)layer_get_data(layerPbl);
    if (layer->icon == EFFECT_ICON_NONE)
        return;
    const bool isPersonal = layer->icon < EFFECT_ICON_FIRST_ENVIRONMENTAL;
    const GColor bgColor = 
        layer->icon == EFFECT_ICON_HEALING ? s_iconHealingColor
        : s_iconBgColors[(layer->icon - 1) % 5];
    const GRect bounds = layer_get_bounds(layer->layer);

    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    if (isPersonal)
    {
        GBitmap* icon = layer->big ? s_iconTriBig : s_iconTri;
        gbitmap_get_palette(icon)[layer->big ? 3 : 1] = bgColor;
        graphics_draw_bitmap_in_rect(ctx, icon, bounds);
    }
    else if (layer->big)
    {
        graphics_context_set_fill_color(ctx, bgColor);
        graphics_context_set_stroke_color(ctx, GColorBlack);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_fill_circle(ctx, GPoint(15, 15), 14);
        graphics_draw_circle(ctx, GPoint(15, 15), 12);
        graphics_context_set_stroke_width(ctx, 1);
        graphics_draw_circle(ctx, GPoint(15, 15), 15);
        graphics_draw_circle(ctx, GPoint(15, 15), 16);
    }
    else
    {
        graphics_context_set_fill_color(ctx, bgColor);
        graphics_fill_circle(ctx, GPoint(10, 10), 10);
    }

    GRect iconBounds = { .origin = s_iconOffsets[layer->icon - 1], .size = GSize(13, 13) };
    if (layer->big)
    {
        iconBounds.origin.x += 4;
        iconBounds.origin.y += 6;
    }
    gbitmap_get_palette(s_iconsFull)[1] = s_iconFgColors[layer->icon - 1];
    graphics_draw_bitmap_in_rect(ctx, s_icons[layer->icon - 1], iconBounds);
}

// ------------------------------------------------------------------------------------------------

#define CURVED_CHAR_SIZE 10
#define CURVED_CHAR_GSIZE GSize(CURVED_CHAR_SIZE, CURVED_CHAR_SIZE)
#define CURVED_CHAR_IC GPoint(CURVED_CHAR_SIZE / 2, CURVED_CHAR_SIZE / 2)
#define CURVED_CHAR_SRCIC GPoint(5, 4)
#define CURVED_CHAR_BOUNDS GRect(0, 1, CURVED_CHAR_SIZE, CURVED_CHAR_SIZE)
#define CURVED_CHAR_ANGLEPERCHAR (DEG_TO_TRIGANGLE(10))
#define CURVED_CHAR_RADIUS 60

static GBitmap* s_fontFull, * s_fontChars['Z' - 'A' + 11];

void curved_text_create(CurvedTextLayer* layer, Layer* parentLayer)
{
    if (s_fontFull == NULL)
    {
        s_fontFull = gbitmap_create_with_resource(RESOURCE_ID_FONT_8);
        GBitmapFormat format = gbitmap_get_format(s_fontFull) ;
        ASSERT(format == GBitmapFormat8Bit);
        GRect sub = GRect(0, 0, 10, 8);
        for (int i = 0; i < (int)(sizeof(s_fontChars) / sizeof(s_fontChars[0])); i++)
        {
            s_fontChars[i] = gbitmap_create_as_sub_bitmap(s_fontFull, sub);
            sub.origin.y += sub.size.h;
        }
    }

    // Unfortunately full 8Bit to be able to do semi-efficient offscreen rendering
    for (int i = 0; i < MAX_BODY_LENGTH + 1; i++)
        layer->charBitmaps[i] = gbitmap_create_blank(CURVED_CHAR_GSIZE, GBitmapFormat8Bit);
    layer->layer = layer_create_with_data(GRect(23, 91, 134, 66), sizeof(CurvedTextLayer*));
    *(CurvedTextLayer**)layer_get_data(layer->layer) = layer;
    layer_set_update_proc(layer->layer, prv_curved_text_draw);
    layer_set_clips(layer->layer, false);
    layer_add_child(parentLayer, layer->layer);
    layer->charCount = 0;
    layer->text[0] = 0;
}

void curved_text_destroy(CurvedTextLayer* layer)
{
    for (int i = 0; i < MAX_BODY_LENGTH + 1; i++)
        gbitmap_destroy(layer->charBitmaps[i]);
    layer_destroy(layer->layer);
}

void curved_text_set_text(CurvedTextLayer* layer, const char* text)
{
    if (strncmp(layer->text, text, MAX_BODY_LENGTH) == 0)
        return;
    layer->charCount = strlen(text);
    if (layer->charCount > MAX_BODY_LENGTH)
        layer->charCount = MAX_BODY_LENGTH;
    memcpy(layer->text, text, layer->charCount);
    layer->rerender = true;
}


// integer_sqrt, polar_div and my_graphics_draw_rotated_bitmap are adapted versions of 
// the released, original pebble firmware, licensed under Apache-2.0

//! newton's method for floor(sqrt(x)) -> should always converge
int32_t integer_sqrt(int64_t x) {
  if (x < 0) {
    return 0;
  }
  int64_t last_res = 0x3fff;
  uint16_t iterations = 0;
  while ((last_res > 0) && (iterations < 15)) {
    last_res = ((x / last_res) + last_res)/2;
    iterations++;
  }
  return (last_res);
}

typedef struct DivResult {
  int32_t quot;
  int32_t rem;
} DivResult;

//! a div and mod operation where any remainder will always be the same direction as the numerator
static DivResult polar_div(int32_t numer, int32_t denom) {
  DivResult res;
  res.quot = numer / denom;
  res.rem = numer % denom;
  if (numer < 0 && res.rem > 0) {
    res.rem -= denom;
    res.quot += denom;
  }
  return res;
}

#define MAX(a,b) ((a)>=(b)?(a):(b))
#define MIN(a,b) ((a)<=(b)?(a):(b))
#define WITHIN(x,a,b) ((x)>=(a) && (x)<=(b))



void my_graphics_draw_rotated_bitmap(GBitmap* dest_bitmap, GBitmap *src, GPoint src_ic, int rotation,
                                  GPoint dest_ic) {

  GRect dest_clip = gbitmap_get_bounds(dest_bitmap);
  GRect src_bounds = gbitmap_get_bounds(src);

  if (grect_contains_point(&src_bounds, &src_ic)) {

    const int16_t max_width = MAX(src_bounds.origin.x + src_bounds.size.w - src_ic.x,
                                  src_ic.x - src_bounds.origin.x);
    const int16_t max_height = MAX(src_bounds.origin.y + src_bounds.size.h - src_ic.y,
                                   src_ic.y - src_bounds.origin.y);
    const int32_t width = 2 * (max_width + 1);   // Add one more pixel in case on the edge
    const int32_t height = 2 * (max_height + 1); // Add one more pixel in case on the edge

    // add two pixels just in case of rounding isssues
    const int32_t max_distance = integer_sqrt((width * width) + (height * height)) + 2;
    const int32_t min_x = src_ic.x - max_distance;
    const int32_t min_y = src_ic.y - max_distance;

    const int32_t size_x = max_distance*2;
    const int32_t size_y = size_x;

    const GRect dest_clip_min = GRect(dest_ic.x + min_x, dest_ic.y + min_y, size_x, size_y);
    grect_clip(&dest_clip, &dest_clip_min);
  }


  for (int y = dest_clip.origin.y; y < dest_clip.origin.y + dest_clip.size.h; ++y) {
    for (int x = dest_clip.origin.x; x < dest_clip.origin.x + dest_clip.size.w; ++x) {
      // only draw if within the dest range
      const GBitmapDataRowInfo dest_info = gbitmap_get_data_row_info(dest_bitmap, y);
      if (!WITHIN(x, dest_info.min_x, dest_info.max_x)) {
        continue;
      }

      const int32_t cos_value = cos_lookup(-rotation);
      const int32_t sin_value = sin_lookup(-rotation);
      const int32_t src_numerator_x = cos_value * (x - dest_ic.x) - sin_value * (y - dest_ic.y);
      const int32_t src_numerator_y = cos_value * (y - dest_ic.y) + sin_value * (x - dest_ic.x);

      const DivResult src_vector_x = polar_div(src_numerator_x, TRIG_MAX_RATIO);
      const DivResult src_vector_y = polar_div(src_numerator_y, TRIG_MAX_RATIO);

      const int32_t src_x = src_ic.x + src_vector_x.quot;
      const int32_t src_y = src_ic.y + src_vector_y.quot;

      // only draw if within the src range
      const GBitmapDataRowInfo src_info = gbitmap_get_data_row_info(src, src_y + src_bounds.origin.y);
      if (!(WITHIN(src_x, 0, src_bounds.size.w - 1) &&
        WITHIN(src_y, 0, src_bounds.size.h - 1) &&
        WITHIN(src_x, src_info.min_x, src_info.max_x))) {
        continue;
      }
      dest_info.data[x] = src_info.data[src_x];
    }
  }
}

void prv_curved_text_draw(Layer* layerPbl, GContext* ctx)
{
    CurvedTextLayer* layer = *(CurvedTextLayer**)layer_get_data(layerPbl);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_context_set_antialiased(ctx, false);

    if (layer->rerender)
    {
        layer->rerender = false;
        int rotation = layer->charCount * CURVED_CHAR_ANGLEPERCHAR / 2;
        GRect frame = layer_get_frame(layerPbl);
        for (int i = 0; i < layer->charCount; i++)
        {
            int charI = -1;
            char ch = layer->text[i];
            if (ch >= 'A' && ch <= 'Z') charI = ch - 'A';
            if (ch >= '0' && ch <= '9') charI = 'Z' - 'A' + 1 + ch - '0';
            if (charI < 0)
            {
                layer->charBounds[i] = GRect(0,0,0,0);
            }
            else
            {
                uint8_t* data = gbitmap_get_data(layer->charBitmaps[i]);
                memset(data, 0, CURVED_CHAR_SIZE * CURVED_CHAR_SIZE);
                my_graphics_draw_rotated_bitmap(layer->charBitmaps[i], s_fontChars[charI], CURVED_CHAR_SRCIC, rotation, CURVED_CHAR_IC);
                layer->charBounds[i] = grect_centered_from_polar(
                    GRectCenteredCircle(CURVED_CHAR_RADIUS), GOvalScaleModeFitCircle,
                    rotation + TRIG_MAX_ANGLE / 2, CURVED_CHAR_GSIZE);
                layer->charBounds[i].origin.x -= frame.origin.x; 
                layer->charBounds[i].origin.y -= frame.origin.y;
            }
            rotation += -CURVED_CHAR_ANGLEPERCHAR;
        }
    }

    for (int i = 0; i < layer->charCount; i++)
    {
        if (layer->charBounds[i].size.w > 0)
            graphics_draw_bitmap_in_rect(ctx, layer->charBitmaps[i], layer->charBounds[i]);
    }
}

// ------------------------------------------------------------------------------------------------

static GBitmap* s_planetBitmap, *s_planetSmallBitmap;
static GBitmap* s_starsBigBitmap, *s_starsSmallBitmap;

void planet_create(PlanetLayer* layer, bool big, Layer* parentLayer)
{
    if (big && s_planetBitmap == NULL)
    {
        s_planetBitmap = gbitmap_create_with_resource(RESOURCE_ID_PLANET);
        s_starsBigBitmap = gbitmap_create_with_resource(RESOURCE_ID_STARS_BIG);
    }
    if (!big && s_planetSmallBitmap == NULL)
    {
        s_planetSmallBitmap = gbitmap_create_with_resource(RESOURCE_ID_PLANET_SMALL);
        s_starsSmallBitmap = gbitmap_create_with_resource(RESOURCE_ID_STARS_SMALL);
    }
    const int layerRadius = big ? 50 : 21;
    const GRect bounds = big
        ? GRectCenteredCircle(layerRadius)
        : GRect(90 - layerRadius, 7, layerRadius * 2, layerRadius * 2);

    layer->layer = layer_create_with_data(bounds, sizeof(PlanetLayer*));
    *(PlanetLayer**)layer_get_data(layer->layer) = layer;
    layer_set_update_proc(layer->layer, prv_planet_draw);
    layer_add_child(parentLayer, layer->layer);
    layer_set_clips(layer->layer, false);
    layer->lastTime = -1;
    layer->big = big;
    layer->path.points = layer->points;
    layer->path.num_points = (big ? SUN_POINTS : SUN_SMALL_POINTS) * 2;
    layer->path.offset = GPoint(layerRadius, layerRadius);
    layer->path.rotation = 0;
}

void planet_destroy(PlanetLayer* layer)
{
    layer_destroy(layer->layer);
}

void planet_set_time(PlanetLayer* layer, int time)
{
    ASSERT((time >= 0 && time < (1 << BITS_TIME)) || time == SPACE_TIME);
    if (layer->lastTime == time) return;
    if (time == SPACE_TIME)
    {
        layer->lastTime = SPACE_TIME;
        layer_mark_dirty(layer->layer);
        return;
    }

    // The sun highlight has two strips of vertices, one going on the planet edge and one going across
    // the one going across is an oval stretched horizontally based on the time phase
    // Finally all is tilted.
    // This is based on JS code found on the LPV6, but translated into fixed point

    const int32_t MaxTime = 1 << BITS_TIME;
    const int32_t HalfTime = 1 << (BITS_TIME - 1);
    const int32_t AnglePerPoint = TRIG_MAX_ANGLE / 2 / (layer->big ? SUN_POINTS : SUN_SMALL_POINTS);
    const int32_t OuterRadius = layer->big ? 50 : 19;
    const int32_t innerRadius = time < HalfTime
        ? -OuterRadius + OuterRadius * 4 * time / MaxTime
        : OuterRadius - OuterRadius * 4 * (time - HalfTime) / MaxTime;
    const int32_t tilt = time < HalfTime
        ? DEG_TO_TRIGANGLE(57)
        : DEG_TO_TRIGANGLE(180 + 57);
    const int32_t // TODO: clean up switcharoo
        cosTilt = sin_lookup(tilt) >> 1, // I sacrify one bit of precision to avoid 64bit arithmetic later
        sinTilt = cos_lookup(tilt) >> 1; 
    int i = 0;
    GPoint* point = layer->points;

    // stretched oval
    for (i = 0; i < TRIG_MAX_ANGLE / 2; i += AnglePerPoint, point++)
    {
        const int32_t sinEdge = sin_lookup(i), cosEdge = cos_lookup(i);
        const int32_t sinSin = (sinEdge * sinTilt) >> 15, sinCos = (sinEdge * cosTilt) >> 15,
                      cosSin = (cosEdge * sinTilt) >> 15, cosCos = (cosEdge * cosTilt) >> 15;
        point->x = (-OuterRadius * cosSin + innerRadius * sinCos) >> 16,
        point->y = (OuterRadius * cosCos + innerRadius * sinSin) >> 16;
    }

    // planet edge (only if we have gone over the zenith)
    if (layer->lastTime < 0 || (layer->lastTime < HalfTime) != (time < HalfTime))
    {
        for (; i < TRIG_MAX_ANGLE; i += AnglePerPoint, point++)
        {
            const int32_t sinEdge = sin_lookup(i), cosEdge = cos_lookup(i);
            const int32_t sinSin = (sinEdge * sinTilt) >> 15, sinCos = (sinEdge * cosTilt) >> 15,
                          cosSin = (cosEdge * sinTilt) >> 15, cosCos = (cosEdge * cosTilt) >> 15;
            point->x = (OuterRadius * (-cosSin + sinCos)) >> 16;
            point->y = (OuterRadius * (cosCos + sinSin)) >> 16;
        }
    }

    layer->lastTime = time;
    layer_mark_dirty(layer->layer);
}

void prv_planet_draw(Layer* layerPbl, GContext* ctx)
{
    PlanetLayer* layer = *(PlanetLayer**)layer_get_data(layerPbl);
    graphics_context_set_antialiased(ctx, false);
    GBitmap* bitmap;
    if (layer->big) // there is a white background for the small planet :(
    {
        graphics_context_set_compositing_mode(ctx, GCompOpSet);
        bitmap = layer->lastTime == SPACE_TIME ? s_starsBigBitmap : s_planetBitmap;
    }
    else
    {
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
        if (layer->lastTime == SPACE_TIME)
            bitmap = s_starsSmallBitmap;
        else
        {
            graphics_context_set_fill_color(ctx, GColorBlack);
            graphics_fill_circle(ctx, GPoint(21, 21), 20);
            graphics_context_set_compositing_mode(ctx, GCompOpSet);
            bitmap = s_planetSmallBitmap;
        }
    }

    if (layer->lastTime != SPACE_TIME)
    {
        graphics_context_set_fill_color(ctx, layer->big ? GColorLightGray : GColorWhite);
        gpath_draw_filled(ctx, &layer->path);
    }
    graphics_draw_bitmap_in_rect(ctx, bitmap, gbitmap_get_bounds(bitmap));
}

// ------------------------------------------------------------------------------------------------

#define SCAN_CRESCENT_POINTS 8
#define SCAN_CRESCENT_INNER_POINTS 8
#define SCAN_CRESCENT_RADIUS (174/2)
#define SCAN_CRESCENT_ANGLE DEG_TO_TRIGANGLE(90 - 27)

static GPoint s_scanTopPoints[SCAN_CRESCENT_POINTS];
static GPoint s_scanBottomPoints[SCAN_CRESCENT_POINTS];

void scan_decoration_create(ScanDecorationLayer* layer, Layer* parentLayer)
{
    layer->centerLayer = layer_create(GRect(65, 102, 50, 38));
    layer_set_update_proc(layer->centerLayer, prv_scan_decoration_center_draw);
    layer_add_child(parentLayer, layer->centerLayer);

    layer->bottomLayer = layer_create_with_data(GRect(0, 144, 180, 36), sizeof(GPath*));
    *(GPath**)layer_get_data(layer->bottomLayer) = &layer->bottomPath;
    layer_set_update_proc(layer->bottomLayer, prv_scan_decoration_crescent_draw);
    layer_add_child(parentLayer, layer->bottomLayer);

    layer->topLayer = layer_create_with_data(GRect(0, 0, 180, 36), sizeof(GPath*));
    *(GPath**)layer_get_data(layer->topLayer) = &layer->topPath;
    layer_set_update_proc(layer->topLayer, prv_scan_decoration_crescent_draw);
    layer_add_child(parentLayer, layer->topLayer);

    layer->bottomPath.points = s_scanBottomPoints;
    layer->topPath.points = s_scanTopPoints;
    layer->bottomPath.num_points = layer->topPath.num_points = SCAN_CRESCENT_POINTS;
    layer->bottomPath.offset = layer->topPath.offset = GPointZero;
    layer->bottomPath.rotation = layer->topPath.rotation = 0;

    static bool hasCalculatedPoints = false;
    if (hasCalculatedPoints)
        return;
    hasCalculatedPoints = true;
    for (int i = 0; i < SCAN_CRESCENT_POINTS; i++)
    {
        int angle = -SCAN_CRESCENT_ANGLE + SCAN_CRESCENT_ANGLE * 2 * i / (SCAN_CRESCENT_POINTS - 1);
        int x = ((sin_lookup(angle) * SCAN_CRESCENT_RADIUS) >> 16);
        int y = ((cos_lookup(angle) * SCAN_CRESCENT_RADIUS) >> 16);
        s_scanTopPoints[i] = GPoint(x + 90, SCAN_CRESCENT_RADIUS - y);
        s_scanBottomPoints[i] = GPoint(x + 90, y - 40); 
    }
}

void scan_decoration_destroy(ScanDecorationLayer* layer)
{
    layer_destroy(layer->centerLayer);
    layer_destroy(layer->bottomLayer);
    layer_destroy(layer->topLayer);
}

static void prv_scan_decoration_center_draw(struct Layer* layer, GContext* ctx)
{
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_rect(ctx, GRect(0, 0, 3, 38), 0, 0);
    graphics_fill_rect(ctx, GRect(47, 0, 3, 38), 0, 0);
}

static void prv_scan_decoration_crescent_draw(struct Layer* layer, GContext* ctx)
{
    GPath* path = *(GPath**)layer_get_data(layer);
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_fill_color(ctx, GColorLightGray);
    gpath_draw_filled(ctx, path);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    if (path->points == s_scanTopPoints)
        graphics_fill_rect(ctx, GRect(0, 31, 180, 3), 0, 0);
    else
        graphics_fill_rect(ctx, GRect(0, 3, 180, 3), 0, 0);
}

// ------------------------------------------------------------------------------------------------

void alert_background_create(AlertBackgroundLayer* layer, Layer* parentLayer)
{
    layer->layer = layer_create_with_data(GRect(0, 0, 180, 180), sizeof(GColor));
    layer_set_update_proc(layer->layer, prv_alert_background_draw);
    layer_add_child(parentLayer, layer->layer);
}

void alert_background_destroy(AlertBackgroundLayer* layer)
{
    layer_destroy(layer->layer);
}

void alert_background_set_color(AlertBackgroundLayer* layer, GColor color)
{
    *(GColor*)layer_get_data(layer->layer) = color;
    layer_mark_dirty(layer->layer);
}

static void prv_alert_background_draw(Layer* layer, GContext* ctx)
{
    const GColor color = *(GColor*)layer_get_data(layer);
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_fill_color(ctx, color);
    graphics_context_set_stroke_color(ctx, color);
    graphics_context_set_stroke_width(ctx, 3);
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    graphics_draw_circle(ctx, GPoint(90, 90), 89);
    graphics_draw_line(ctx, GPoint(12, 63), GPoint(168, 63));
    graphics_draw_line(ctx, GPoint(12, 118), GPoint(168, 118));

    const int XOffset = 6; // but from the circular framebuffer edges
    const int YOffset = 7;
    const int Height = 52;
    const int StartOffset = 18;
    const int OnWidth = 13;
    const int OffWidth = 18; // surely no coincidence that OnWidth + OffWidth == 32 == 2^5 right?
    GBitmap* framebuffer = graphics_capture_frame_buffer(ctx);
    for (int y = 0; y < Height; y++)
    {
        // I am going to assume that pixel 90 is always valid and that the framebuffer is symmetric
        GBitmapDataRowInfo upper = gbitmap_get_data_row_info(framebuffer, YOffset + y);
        GBitmapDataRowInfo lower = gbitmap_get_data_row_info(framebuffer, 180 - YOffset - y);
        int lineOffset = (StartOffset + y) & 31;
        int max_x = upper.max_x - 90;
        max_x -= XOffset;
        for (int x = 0; ; x += OnWidth + OffWidth)
        {
            int minX = x - lineOffset, maxX = minX + OnWidth;
            if (minX < 0)
            {
                //maxX += minX - 0;
                minX = 0;
            }
            if (maxX < 0)
                continue;
            if (minX > max_x)
                break;
            if (maxX > max_x)
                maxX = max_x;
            int width = maxX - minX + 1;
            memset(upper.data + 90 - maxX, color.argb, width);
            memset(upper.data + 90 + minX, color.argb, width);
            memset(lower.data + 90 - maxX, color.argb, width);
            memset(lower.data + 90 + minX, color.argb, width);
            
        }        
    }
    graphics_release_frame_buffer(ctx, framebuffer);

    // yes I knwow this can be done much more efficient, I don't have the time for it
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_circle(ctx, GPoint(90, 90), 85);
}

// ------------------------------------------------------------------------------------------------

static GBitmap *s_statusIcons, *s_statusBLYes, *s_statusBLNo, *s_statusGameYes, *s_statusGameNo;

void app_status_create(AppStatusLayer* layer, Layer* parentLayer)
{
    if (s_statusIcons == NULL)
    {
        s_statusIcons = gbitmap_create_with_resource(RESOURCE_ID_STATUS_ICONS);
        s_statusBLYes = gbitmap_create_as_sub_bitmap(s_statusIcons, GRect(0, 0, 32, 32));
        s_statusBLNo = gbitmap_create_as_sub_bitmap(s_statusIcons, GRect(32, 0, 32, 32));
        s_statusGameYes = gbitmap_create_as_sub_bitmap(s_statusIcons, GRect(0, 32, 32, 32));
        s_statusGameNo = gbitmap_create_as_sub_bitmap(s_statusIcons, GRect(32, 32, 32, 32));
    }

    layer->layer = layer_create_with_data(GRect(41, 27, 98, 44), sizeof(AppStatusLayer*));
    *(AppStatusLayer**)layer_get_data(layer->layer) = layer;
    layer_set_update_proc(layer->layer, prv_app_status_draw);
    layer_add_child(parentLayer, layer->layer);
}

void app_status_destroy(AppStatusLayer* layer)
{
    layer_destroy(layer->layer);
}

static void prv_app_status_draw(Layer* layerPbl, GContext* ctx)
{
    AppStatusLayer* layer = *(AppStatusLayer**)layer_get_data(layerPbl);
    graphics_context_set_antialiased(ctx, false);
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_context_set_fill_color(ctx, GColorLightGray);
    graphics_fill_rect(ctx, GRect(0, 0, 44, 44), 8, GCornersAll);
    graphics_fill_rect(ctx, GRect(54, 0, 44, 44), 8, GCornersAll);
    graphics_draw_bitmap_in_rect(ctx, layer->bluetooth ? s_statusBLYes : s_statusBLNo, GRect(6, 6, 32, 32));
    graphics_draw_bitmap_in_rect(ctx, layer->game ? s_statusGameYes : s_statusGameNo, GRect(60, 6, 32, 32));
}
