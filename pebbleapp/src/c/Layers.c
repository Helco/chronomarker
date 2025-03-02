#include "Chronomarker.h"

static void prv_o2co2_draw(struct Layer *layer, GContext* ctx);
static void prv_effect_icon_draw(struct Layer *layer, GContext* ctx);
static void prv_curved_text_draw(struct Layer *layer, GContext* ctx);
static void prv_planet_draw(struct Layer* layer, GContext* ctx);
static void prv_scan_decoration_crescent_draw(struct Layer* layer, GContext* ctx);
static void prv_scan_decoration_center_draw(struct Layer* layer, GContext* ctx);

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

// ------------------------------------------------------------------------------------------------

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

    ASSERT(positionSlot >= 0 && positionSlot < (int)(sizeof(s_iconPositionSlots) / sizeof(GPoint)));
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

void planet_create(PlanetLayer* layer, bool big, Layer* parentLayer)
{
    if (big && s_planetBitmap == NULL)
        s_planetBitmap = gbitmap_create_with_resource(RESOURCE_ID_PLANET);
    if (!big && s_planetSmallBitmap == NULL)
        s_planetSmallBitmap = gbitmap_create_with_resource(RESOURCE_ID_PLANET_SMALL);
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
    ASSERT(time >= 0 && time < (1 << BITS_TIME));
    if (layer->lastTime == time) return;

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
    if (!layer->big) // there is a white background for the small planet :(
    {
        graphics_context_set_compositing_mode(ctx, GCompOpAssign);
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_circle(ctx, GPoint(21, 21), 20);
    }

    graphics_context_set_fill_color(ctx, layer->big ? GColorLightGray : GColorWhite);
    gpath_draw_filled(ctx, &layer->path);

    GBitmap* bitmap = layer->big ? s_planetBitmap : s_planetSmallBitmap;
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
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
    //return;
    graphics_context_set_fill_color(ctx, GColorBlack);
    if (path->points == s_scanTopPoints)
        graphics_fill_rect(ctx, GRect(0, 31, 180, 3), 0, 0);
    else
        graphics_fill_rect(ctx, GRect(0, 3, 180, 3), 0, 0);
}
