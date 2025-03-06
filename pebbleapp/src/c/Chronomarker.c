#include "Chronomarker.h"

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
  .planetGrav = 107,
};

static void prv_app_finish_gamealert(void* data);

static void prv_app_next_gamealert()
{
  ASSERT(app.alertCount > 0);
  ASSERT(app.curAlertTimer == NULL);
  app.curAlertTimer = app_timer_register(ALERT_TIMEOUT, prv_app_finish_gamealert, NULL);
  const GameAlert* curAlert = app.alerts + app.curAlertI;
  if (curAlert->icon >= EFFECT_ICON_FIRST_ENVIRONMENTAL)
    alert_window_handle_alert(&app.alert, curAlert);
  else
    main_window_handle_alert(&app.main, curAlert);
  //vibes_long_pulse();
}

static void prv_app_finish_gamealert(void* data)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Remove alert");
  main_window_handle_alert(&app.main, NULL);
  alert_window_handle_alert(&app.alert, NULL);
  app.curAlertTimer = NULL;
  if (app.alertCount > 0)
  {
    app.alertCount--;
    app.curAlertI = (app.curAlertI + 1) % MAX_ALERTS;
  }
  if (app.alertCount > 0)
    prv_app_next_gamealert();
}

void app_handle_gamealert(const GameAlert* alert)
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Alert %d: %s, %s", alert->icon, alert->title, alert->subtitle);
  if (app.alertCount >= MAX_ALERTS)
  {
    // I just guess this does not happen
    APP_LOG(APP_LOG_LEVEL_WARNING, "Too many alerts, dropping...");
    return;
  }
  int i = (app.curAlertI + app.alertCount) % MAX_ALERTS;
  memcpy(app.alerts + i, alert, sizeof(GameAlert));
  app.alertCount++;
  if (app.curAlertTimer == NULL)
    prv_app_next_gamealert();
}

void app_handle_gamestate(StateChanges changes)
{
  Window* topWindow = window_stack_get_top_window();
  Window* supposedWindow = game.playerFlags & PLAYER_IS_SCANNING
    ? app.scan.window : app.main.window;
  if (topWindow != supposedWindow)
  {
    if (topWindow != app.main.window)
      window_stack_pop(true);
    else
      window_stack_push(supposedWindow, true);
  }
  else if (topWindow == app.main.window)
    main_window_handle_gamestate(&app.main, changes);
  else if (topWindow == app.scan.window)
    scan_window_handle_gamestate(&app.scan, changes);
}

static void prv_init(void) {
  memset(&app, 0, sizeof(app));

  app_status_window_create(&app.status);
  main_window_create(&app.main);
  scan_window_create(&app.scan);
  alert_window_create(&app.alert);

  window_stack_push(app.status.window, false);
  app_status_set_status(&app.status, true, false);

  communication_init();
}

static void prv_deinit(void) {
  app_status_window_destroy(&app.status);
  main_window_destroy(&app.main);
  scan_window_destroy(&app.scan);
  alert_window_destroy(&app.alert);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing");

  app_event_loop();
  prv_deinit();
}
