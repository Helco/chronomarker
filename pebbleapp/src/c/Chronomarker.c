#include "Chronomarker.h"

typedef struct App
{
  MainWindow main;
  ScanWindow scan;
} App;
App app;
GameState game = {
  .o2 = 50,
  .co2 = 13,
  .personalEffects = { 0, 0, 0, 0, 0},
  .envEffects = { 0, 0, 0, 0 },
  .heading = 0,
  .time = 40,
  .playerFlags = 0,
  .bodyName = "PROCYON III",
  .locationName = "PLANET",
  .planetTemperature = -222,
  .planetOxygen = 21,
  .planetGrav = 107
};

void app_handle_gamealert(const GameAlert* alert)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Alert %d: %d, %d", alert->kind, alert->title, alert->subtitle);
}

void app_handle_gamestate(StateChanges changes)
{
  Window* topWindow = window_stack_get_top_window();
  if (topWindow == app.main.window)
    main_window_handle_gamestate(&app.main, changes);
  else if (topWindow == app.scan.window)
    scan_window_handle_gamestate(&app.scan, changes);
}

static void prv_init(void) {
  main_window_create(&app.main);
  scan_window_create(&app.scan);
  communication_init();

  main_window_push(&app.main);
  scan_window_push(&app.scan);
}

static void prv_deinit(void) {
  main_window_destroy(&app.main);
  scan_window_destroy(&app.scan);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  app_event_loop();
  prv_deinit();
}
