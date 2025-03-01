#include "Chronomarker.h"

static Window *s_window;
static TextLayer *s_text_layer;

char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

#define checkAppMsg(call) do { \
  AppMessageResult result = (call); \
  if (result != APP_MSG_OK) \
    APP_LOG(APP_LOG_LEVEL_ERROR, "Call failed with %s", translate_error(result)); \
  } while (0)

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Up");
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Down");
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static BitmapLayer* s_planetLayer;
static GBitmap* s_planetBitmap;
static O2CO2Layer s_o2co2;
static EffectIconLayer s_effectIcons[5 + 4];
//static CurvedTextLayer s_bodyName;

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(s_window, GColorBlack);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);

  s_planetBitmap = gbitmap_create_with_resource(RESOURCE_ID_PLANET);
  s_planetLayer = bitmap_layer_create(GRect(180/2 - 50, 180/2 - 50, 100, 100));
  bitmap_layer_set_bitmap(s_planetLayer, s_planetBitmap);
  bitmap_layer_set_compositing_mode(s_planetLayer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_planetLayer));

  o2co2_create(&s_o2co2, window_layer);
  o2co2_set_values(&s_o2co2, 22, 42);
  //curved_text_create(&s_bodyName, window_layer);
  //curved_text_set_text(&s_bodyName, "TRITON");

  for (int i = 0; i < 5 + 4; i++)
  {
    effect_icon_create(s_effectIcons + i, window_layer, i);
    effect_icon_set_icon(s_effectIcons + i, EFFECT_ICON_CARDIO + i);
  }
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  bitmap_layer_destroy(s_planetLayer);
  gbitmap_destroy(s_planetBitmap);
  o2co2_destroy(&s_o2co2);
  //curved_text_destroy(&s_bodyName);
  for (int i = 0; i < 5 + 4; i++)
    effect_icon_destroy(s_effectIcons + i);
}

static void in_dropped_handler(AppMessageResult reason, void* context)
{
  APP_LOG(APP_LOG_LEVEL_WARNING, "Msg dropped: %s", translate_error(reason));
}

char s_text_buffer[64] = {0};
static void in_received_handler(DictionaryIterator* received, void* context)
{
  Tuple* tuple = dict_find(received, 16);
  if (tuple == NULL || tuple->type != TUPLE_UINT || tuple->length != 1)
    APP_LOG(APP_LOG_LEVEL_WARNING, "Msg received invalid");
  else
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Msg received: %d", (int)tuple->value->uint8);
    snprintf(s_text_buffer, 64, "%d", (int)tuple->value->uint8);
    text_layer_set_text(s_text_layer, s_text_buffer);
  }
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_inbox_received(in_received_handler);
  checkAppMsg(app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, 64));
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
