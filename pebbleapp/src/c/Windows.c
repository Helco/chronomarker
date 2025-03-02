#include "Chronomarker.h"

static void prv_main_window_load(Window* window)
{
    MainWindow* m = (MainWindow*)window_get_user_data(window);
    window_set_background_color(window, GColorBlack);
    Layer *window_layer = window_get_root_layer(window);

    planet_create(&m->planet, window_layer);
    o2co2_create(&m->o2co2, window_layer);
    //curved_text_create(&s_bodyName, window_layer);
    //curved_text_set_text(&s_bodyName, "TRITON");

    for (int i = 0; i < 5 + 4; i++)
    {
        effect_icon_create(m->effectIcons + i, window_layer, i);
    }
}

static void prv_main_window_unload(Window* window)
{
    MainWindow* m = (MainWindow*)window_get_user_data(window);
    planet_destroy(&m->planet);
    o2co2_destroy(&m->o2co2);
    //curved_text_destroy(&s_bodyName);
    for (int i = 0; i < 5 + 4; i++)
        effect_icon_destroy(m->effectIcons + i);
}

static void prv_main_window_appear(Window* window)
{
    MainWindow* m = (MainWindow*)window_get_user_data(window);
    main_window_handle_gamestate(m, ~0);
}

void main_window_create(MainWindow* m)
{
    m->window = window_create();
    window_set_user_data(m->window, m);
    window_set_window_handlers(m->window, (WindowHandlers) {
        .load = prv_main_window_load,
        .unload = prv_main_window_unload,
        .appear = prv_main_window_appear
    });
}

void main_window_destroy(MainWindow* m)
{
    window_destroy(m->window);
}

void main_window_handle_gamestate(MainWindow* m, StateChanges changes)
{
    if (changes & STATE_O2CO2)
        o2co2_set_values(&m->o2co2, game.o2, game.co2);
    if (changes & STATE_TIME)
        planet_set_time(&m->planet, game.time);
    if (changes & STATE_PERSONALEFFECTS)
    {
        for (int i = 0; i < MAX_PERSONALEFFECTS; i++)
            effect_icon_set_icon(m->effectIcons + i, game.personalEffects[i]);
    }
    if (changes & STATE_ENVEFFECTS)
    {
        for (int i = 0; i < MAX_ENVEFFECTS; i++)
            effect_icon_set_icon(m->effectIcons + 5 + i, game.envEffects[i]);
    }
}

void main_window_push(MainWindow* m)
{
    window_stack_push(m->window, true);
}

// ------------------------------------------------------------------------------------------------

static GFont s_profontwindows_big, s_profontwindows_small;

static void prv_scan_window_load(Window* window)
{
    if (s_profontwindows_big == NULL)
    {
        s_profontwindows_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PROFONTWINDOWS_18));
        s_profontwindows_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_PROFONTWINDOWS_16));
    }

    ScanWindow* scan = (ScanWindow*)window_get_user_data(window);
    window_set_background_color(window, GColorBlack);
    Layer *window_layer = window_get_root_layer(window);

    scan_decoration_create(&scan->decoration, window_layer);

    scan->bodyName = text_layer_create(GRect(0, 51, 180, 24));
    text_layer_set_text_alignment(scan->bodyName, GTextAlignmentCenter);
    text_layer_set_text_color(scan->bodyName, GColorWhite);
    text_layer_set_background_color(scan->bodyName, GColorClear);
    text_layer_set_font(scan->bodyName, s_profontwindows_big);
    text_layer_set_text(scan->bodyName, game.bodyName);
    layer_add_child(window_layer, text_layer_get_layer(scan->bodyName));

    scan->locationName = text_layer_create(GRect(8, 74, 164, 18));
    text_layer_set_text_alignment(scan->locationName, GTextAlignmentCenter);
    text_layer_set_text_color(scan->locationName, GColorLightGray);
    text_layer_set_background_color(scan->locationName, GColorClear);
    text_layer_set_font(scan->locationName, s_profontwindows_big);
    text_layer_set_text(scan->locationName, game.locationName);
    layer_add_child(window_layer, text_layer_get_layer(scan->locationName));

    scan->temperature = text_layer_create(GRect(9, 100, 60, 38));
    text_layer_set_overflow_mode(scan->temperature, GTextOverflowModeTrailingEllipsis);
    text_layer_set_text_alignment(scan->temperature, GTextAlignmentCenter);
    text_layer_set_text_color(scan->temperature, GColorWhite);
    text_layer_set_background_color(scan->temperature, GColorClear);
    text_layer_set_font(scan->temperature, s_profontwindows_big);
    text_layer_set_text(scan->temperature, scan->temperatureBuffer);
    layer_add_child(window_layer, text_layer_get_layer(scan->temperature));

    scan->oxygen = text_layer_create(GRect(68, 100, 44, 18));
    text_layer_set_text_alignment(scan->oxygen, GTextAlignmentCenter);
    text_layer_set_text_color(scan->oxygen, GColorWhite);
    text_layer_set_background_color(scan->oxygen, GColorClear);
    text_layer_set_font(scan->oxygen, s_profontwindows_big);
    text_layer_set_text(scan->oxygen, scan->oxygenBuffer);
    layer_add_child(window_layer, text_layer_get_layer(scan->oxygen));

    scan->gravity = text_layer_create(GRect(115, 100, 52, 18));
    text_layer_set_text_alignment(scan->gravity, GTextAlignmentCenter);
    text_layer_set_text_color(scan->gravity, GColorWhite);
    text_layer_set_background_color(scan->gravity, GColorClear);
    text_layer_set_font(scan->gravity, s_profontwindows_big);
    text_layer_set_text(scan->gravity, scan->gravityBuffer);
    layer_add_child(window_layer, text_layer_get_layer(scan->gravity));

    scan->legend = text_layer_create(GRect(19, 120, 142, 16));
    text_layer_set_text_alignment(scan->legend, GTextAlignmentCenter);
    text_layer_set_text_color(scan->legend, GColorLightGray);
    text_layer_set_background_color(scan->legend, GColorClear);
    text_layer_set_font(scan->legend, s_profontwindows_small);
    text_layer_set_text(scan->legend, "TEMP    O2   GRAV");
    layer_add_child(window_layer, text_layer_get_layer(scan->legend));
}

static void prv_scan_window_unload(Window* window)
{
    ScanWindow* scan = (ScanWindow*)window_get_user_data(window);
    text_layer_destroy(scan->bodyName);
    text_layer_destroy(scan->locationName);
    text_layer_destroy(scan->temperature);
    text_layer_destroy(scan->oxygen);
    text_layer_destroy(scan->gravity);
    text_layer_destroy(scan->legend);
    scan_decoration_destroy(&scan->decoration);
}

static void prv_scan_window_appear(Window* window)
{
    ScanWindow* scan = (ScanWindow*)window_get_user_data(window);
    scan_window_handle_gamestate(scan, ~0);
}

void scan_window_create(ScanWindow* scan)
{
    scan->window = window_create();
    window_set_user_data(scan->window, scan);
    window_set_window_handlers(scan->window, (WindowHandlers) {
        .load = prv_scan_window_load,
        .unload = prv_scan_window_unload,
        .appear = prv_scan_window_appear
    });
}

void scan_window_destroy(ScanWindow* scan)
{
    window_destroy(scan->window);
}

void scan_window_handle_gamestate(ScanWindow* scan, StateChanges changes)
{
    if (changes & STATE_NAMES)
    {
        layer_mark_dirty(text_layer_get_layer(scan->bodyName));
        layer_mark_dirty(text_layer_get_layer(scan->locationName));
    }
    if (changes & STATE_PLANETSTATS)
    {
        snprintf(scan->temperatureBuffer, SCAN_BUFFER_SIZE, "%d°", game.planetTemperature);
        snprintf(scan->oxygenBuffer, SCAN_BUFFER_SIZE, "%d%%", game.planetOxygen);
        div_t d = div(game.planetGrav, 100);
        snprintf(scan->gravityBuffer, SCAN_BUFFER_SIZE, "%c.%02d", '0' + d.quot, d.rem);
        layer_mark_dirty(text_layer_get_layer(scan->temperature));
        layer_mark_dirty(text_layer_get_layer(scan->oxygen));
        layer_mark_dirty(text_layer_get_layer(scan->gravity));
    }
}

void scan_window_push(ScanWindow* scan)
{
    window_stack_push(scan->window, true);
}
