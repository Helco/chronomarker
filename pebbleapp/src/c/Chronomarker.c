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

static void prv_app_status_changed()
{
  bool bluetooth = bluetooth_connection_service_peek();
  if (window_stack_get_top_window() == app.status.window)
  {
    app_status_set_status(&app.status, bluetooth, app.hasActiveComm);
    if (bluetooth && app.hasActiveComm)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Good status, pushing main window");
      window_stack_push(app.main.window, true);
      vibes_short_pulse();
    }
  }
  else
  {
    if (bluetooth && app.hasActiveComm)
      return;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bad status, resetting to status");
    vibes_double_pulse();
    window_stack_pop_all(false);
    window_stack_push(app.status.window, false);
  }
}

static void prv_bluetooth_handler(bool connected)
{
  prv_app_status_changed();
}

static void prv_communication_timeout()
{
  app.timeoutTimer = NULL;
  app.hasActiveComm = false;
  prv_app_status_changed();
}

static void prv_game_is_active()
{
  if (app.timeoutTimer == NULL || !app_timer_reschedule(app.timeoutTimer, COMM_TIMEOUT))
    app.timeoutTimer = app_timer_register(COMM_TIMEOUT, prv_communication_timeout, NULL);
  app.hasActiveComm = true;
  prv_app_status_changed();
}

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
  vibes_long_pulse();
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
  Window* topWindow = window_stack_get_top_window();
  if (topWindow == app.status.window)
  {
    prv_game_is_active();
    return;
  }
  prv_game_is_active();

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
  // Are we in the game at all?
  Window* topWindow = window_stack_get_top_window();
  if (topWindow == app.status.window)
  {
    prv_game_is_active();
    return;
  }
  prv_game_is_active();
  
  // Handle scanning flag
  if (game.playerFlags & PLAYER_IS_SCANNING)
  {
    if (topWindow == app.main.window)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Scanning and not in alert, pushing scan window");
      window_stack_push(app.scan.window, true);
    }
  }
  else if (topWindow == app.scan.window)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No longer scanning");
    window_stack_pop(true);
  }

  // Passing gamestate change event
  if (topWindow == app.main.window)
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
  bluetooth_connection_service_subscribe(prv_bluetooth_handler);
  prv_communication_timeout();
}

static void prv_deinit(void) {
  bluetooth_connection_service_unsubscribe();
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
