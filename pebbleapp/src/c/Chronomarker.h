#pragma once
#include <pebble.h>

#if 1
#define ASSERT(x) while (!(x)) { \
    APP_LOG(APP_LOG_LEVEL_ERROR, "Assertion failed: %s", #x); \
    *((uint8_t*)NULL) = 0; }
#else
#define ASSERT(x)
#endif

#define BITS_O2CO2 (6)
#define COUNT_O2CO2 (1 << BITS_O2CO2)
#define MAX_O2CO2 (COUNT_O2CO2 - 1)
#define BITS_ALERTTEXT (5)
#define MAX_ALERTTEXT_LENGTH ((1 << BITS_ALERTTEXT) - 1)
#define BITS_HEADING 7
#define BITS_PLAYERFLAGS 3
#define BITS_TIME 6
#define BITS_BODYNAME 4
#define MAX_BODYNAME_LENGTH ((1 << BITS_BODYNAME) - 1)
#define BITS_LOCNAME 5
#define MAX_LOCNAME_LENGTH ((1 << BITS_LOCNAME) - 1)
#define BITS_PLANETGRAV 8
#define BITS_PLANETTEMP 11
#define BITS_PLANETOXYGEN 7
#define BITS_BODYTYPE 3
#define MAX_PERSONALEFFECTS 5
#define MAX_ENVEFFECTS 4
#define BITS_EFFECTS 3
#define BITS_ALERTKIND 4

// ------------------------------------------------------------------------------------------------

typedef struct O2CO2Layer
{
    Layer* layer;
    uint8_t o2, co2;
    bool big;
    GPath o2Path;
    GPath co2Path;
    GPoint points[COUNT_O2CO2 * 2]; // shared between o2/co2
    BitmapLayer *o2Text, *co2Text;
} O2CO2Layer;
void o2co2_create(O2CO2Layer* layer, bool big, Layer* parentLayer);
void o2co2_destroy(O2CO2Layer* layer);
void o2co2_set_values(O2CO2Layer* layer, uint8_t o2, uint8_t co2);

// ------------------------------------------------------------------------------------------------

typedef enum EffectIcon
{
    EFFECT_ICON_NONE = 0,
    EFFECT_ICON_CARDIO,
    EFFECT_ICON_SKELETAL,
    EFFECT_ICON_NERVOUS,
    EFFECT_ICON_DIGESTIVE,
    EFFECT_ICON_MISC,
    EFFECT_ICON_RADIATION,
    EFFECT_ICON_THERMAL,
    EFFECT_ICON_AIRBORNE,
    EFFECT_ICON_CORROSIVE,
    EFFECT_ICON_HEALING,

    EFFECT_ICON_COUNT = EFFECT_ICON_HEALING,
    EFFECT_ICON_FIRST_PERSONAL = EFFECT_ICON_CARDIO,
    EFFECT_ICON_FIRST_ENVIRONMENTAL = EFFECT_ICON_RADIATION
} EffectIcon;
typedef struct EffectIconLayer
{
    Layer* layer;
    EffectIcon icon;
    bool big;
} EffectIconLayer;
void effect_icon_create(EffectIconLayer* layer, Layer* parentLayer, int positionSlot);
void effect_icon_create_big(EffectIconLayer* layer, Layer* parentLayer, bool bioAlert);
void effect_icon_destroy(EffectIconLayer* layer);
void effect_icon_set_icon(EffectIconLayer* layer, EffectIcon icon);

// ------------------------------------------------------------------------------------------------

#define BITS_BODY (4)
#define MAX_BODY_LENGTH (1 << BITS_BODY)

typedef struct CurvedTextLayer
{
    Layer* layer;
    GBitmap* charBitmaps[MAX_BODY_LENGTH + 1];
    GRect charBounds[MAX_BODY_LENGTH];
    uint8_t charCount;
    bool rerender;
    char text[MAX_BODY_LENGTH];
} CurvedTextLayer;
void curved_text_create(CurvedTextLayer* layer, Layer* parentLayer);
void curved_text_destroy(CurvedTextLayer* layer);
void curved_text_set_text(CurvedTextLayer* layer, const char* text);

// ------------------------------------------------------------------------------------------------

#define SUN_POINTS 16
#define SUN_SMALL_POINTS 8
#define SPACE_TIME -100
typedef struct PlanetLayer
{
    Layer* layer;
    GPath path;
    int lastTime;
    bool big;
    GPoint points[SUN_POINTS * 2];
} PlanetLayer;
void planet_create(PlanetLayer* layer, bool big, Layer* parentLayer);
void planet_destroy(PlanetLayer* layer);
void planet_set_time(PlanetLayer* layer, int time);

// ------------------------------------------------------------------------------------------------

typedef struct ScanDecorationLayer
{
    Layer* centerLayer;
    Layer* topLayer;
    Layer* bottomLayer;
    GPath topPath;
    GPath bottomPath;
} ScanDecorationLayer;

void scan_decoration_create(ScanDecorationLayer* layer, Layer* parentLayer);
void scan_decoration_destroy(ScanDecorationLayer* layer);

// ------------------------------------------------------------------------------------------------

typedef struct AlertBackgroundLayer
{
    Layer* layer;
} AlertBackgroundLayer;

void alert_background_create(AlertBackgroundLayer* layer, Layer* parentLayer);
void alert_background_destroy(AlertBackgroundLayer* layer);
void alert_background_set_color(AlertBackgroundLayer* layer, GColor color);

// ------------------------------------------------------------------------------------------------

typedef struct AppStatusLayer
{
    Layer* layer;
    bool bluetooth;
    bool game;
} AppStatusLayer;
void app_status_create(AppStatusLayer* layer, Layer* parentLayer);
void app_status_destroy(AppStatusLayer* layer);

// ------------------------------------------------------------------------------------------------

typedef enum StateChanges
{
    STATE_O2CO2 = 1 << 0,
    STATE_HEADING = 1 << 1,
    STATE_PLAYERFLAGS = 1 << 2,
    STATE_TIME = 1 << 3,
    STATE_NAMES = 1 << 4,
    STATE_PLANETSTATS = 1 << 5,
    STATE_PERSONALEFFECTS = 1 << 6,
    STATE_ENVEFFECTS = 1 << 7,
} StateChanges;
typedef enum PlayerFlags
{
    PLAYER_IN_SPACESHIP = 1 << 0,
    PLAYER_IS_LANDED = 1 << 1,
    PLAYER_IS_SCANNING= 1 << 2,
} PlayerFlags;
typedef enum BodyType
{
    BODY_UNKNOWN = 0,
    BODY_STAR,
    BODY_PLANET,
    BODY_MOON,
    BODY_SATELLITE,
    BODY_ASTEROIDBELT,
    BODY_STATION,
    BODY_SHIP
} BodyType;
typedef struct GameState
{
    uint8_t
        o2, co2,
        heading,
        time,
        planetGrav,
        planetOxygen;
    int16_t planetTemperature;
    BodyType bodyType;
    PlayerFlags playerFlags;
    EffectIcon personalEffects[MAX_PERSONALEFFECTS];
    EffectIcon envEffects[MAX_ENVEFFECTS];
    char bodyName[MAX_BODYNAME_LENGTH + 1];
    char locationName[MAX_LOCNAME_LENGTH + 1];

} GameState;
typedef struct GameAlert
{
    EffectIcon icon;
    bool positive;
    char title[MAX_ALERTTEXT_LENGTH + 1];
    char subtitle[MAX_ALERTTEXT_LENGTH + 1];
} GameAlert;
extern GameState game;
void communication_init();

// ------------------------------------------------------------------------------------------------

typedef struct MainWindow
{
    Window* window;
    PlanetLayer planet;
    O2CO2Layer o2co2;
    EffectIconLayer effectIcons[5 + 4];
    EffectIconLayer alertIcon;
    TextLayer* alertBackground;
    TextLayer* alertText;
} MainWindow;
void main_window_create(MainWindow* window);
void main_window_destroy(MainWindow* window);
void main_window_handle_gamestate(MainWindow* window, StateChanges changes);
void main_window_handle_alert(MainWindow* window, const GameAlert* alert);

// ------------------------------------------------------------------------------------------------

#define SCAN_BUFFER_SIZE 8
typedef struct ScanWindow
{
    Window* window;
    TextLayer* locationName;
    TextLayer* bodyType;
    TextLayer* temperature;
    TextLayer* oxygen;
    TextLayer* gravity;
    TextLayer* legend;
    ScanDecorationLayer decoration;
    PlanetLayer planet;
    O2CO2Layer o2co2;
    char temperatureBuffer[SCAN_BUFFER_SIZE];
    char oxygenBuffer[SCAN_BUFFER_SIZE];
    char gravityBuffer[SCAN_BUFFER_SIZE];
} ScanWindow;
void scan_window_create(ScanWindow* scan);
void scan_window_destroy(ScanWindow* scan);
void scan_window_handle_gamestate(ScanWindow* scan, StateChanges changes);

// ------------------------------------------------------------------------------------------------

typedef struct AlertWindow
{
    Window* window;
    TextLayer* title;
    TextLayer* subtitle;
    EffectIconLayer icon;
    AlertBackgroundLayer background;
    const GameAlert* alert;
} AlertWindow;
void alert_window_create(AlertWindow* aw);
void alert_window_destroy(AlertWindow* aw);
void alert_window_handle_alert(AlertWindow* aw, const GameAlert* alert);

// ------------------------------------------------------------------------------------------------

#define STATUS_BUFFER_SIZE 64
typedef struct AppStatusWindow
{
    Window* window;
    AppStatusLayer status;
    TextLayer* text;
    char textBuffer[STATUS_BUFFER_SIZE];
    bool hadBluetooth, hadActiveComm;
} AppStatusWindow;
void app_status_window_create(AppStatusWindow* asw);
void app_status_window_destroy(AppStatusWindow* asw);
void app_status_set_status(AppStatusWindow* asw, bool bluetooth, bool game);

// ------------------------------------------------------------------------------------------------

#define MAX_ALERTS 4
#define ALERT_TIMEOUT 3500
#define COMM_TIMEOUT 15000
typedef struct App
{
    MainWindow main;
    ScanWindow scan;
    AlertWindow alert;
    AppStatusWindow status;
    GameAlert alerts[MAX_ALERTS];
    int curAlertI, alertCount;
    AppTimer* curAlertTimer;
    AppTimer* timeoutTimer;
    bool hasActiveComm;
} App;
extern App app;
void app_handle_gamestate(StateChanges changes);
void app_handle_gamealert(const GameAlert* alert);
