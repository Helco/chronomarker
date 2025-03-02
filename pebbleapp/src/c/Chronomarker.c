#include "Chronomarker.h"

typedef struct App
{
    Window* window;
    PlanetLayer planet;
    O2CO2Layer o2co2;
    EffectIconLayer effectIcons[5 + 4];
} App;
App app;
GameState game;

void app_handle_gamealert(const GameAlert* alert)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Alert %d: %d, %d", alert->kind, alert->title, alert->subtitle);
}

void app_handle_gamestate(StateChanges changes)
{
  if (changes & STATE_O2CO2)
    o2co2_set_values(&app.o2co2, game.o2, game.co2);
  if (changes & STATE_TIME)
    planet_set_time(&app.planet, game.time);
  if (changes & STATE_PERSONALEFFECTS)
  {
    for (int i = 0; i < MAX_PERSONALEFFECTS; i++)
      effect_icon_set_icon(app.effectIcons + i, game.personalEffects[i]);
  }
  if (changes & STATE_ENVEFFECTS)
  {
    for (int i = 0; i < MAX_ENVEFFECTS; i++)
      effect_icon_set_icon(app.effectIcons + 5 + i, game.envEffects[i]);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);

  planet_create(&app.planet, window_layer);
  planet_set_time(&app.planet, 20);
  o2co2_create(&app.o2co2, window_layer);
  o2co2_set_values(&app.o2co2, 22, 42);
  //curved_text_create(&s_bodyName, window_layer);
  //curved_text_set_text(&s_bodyName, "TRITON");

  for (int i = 0; i < 5 + 4; i++)
  {
    effect_icon_create(app.effectIcons + i, window_layer, i);
  }
}

static void prv_window_unload(Window *window) {
  planet_destroy(&app.planet);
  o2co2_destroy(&app.o2co2);
  //curved_text_destroy(&s_bodyName);
  for (int i = 0; i < 5 + 4; i++)
    effect_icon_destroy(app.effectIcons + i);
}

static void prv_init(void) {
  app.window = window_create();
  window_set_window_handlers(app.window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(app.window, animated);

  communication_init();
}

static void prv_deinit(void) {
  window_destroy(app.window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", app.window);

  app_event_loop();
  prv_deinit();
}
