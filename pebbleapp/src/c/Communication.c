#include "Chronomarker.h"

typedef struct BitStream
{
  const uint16_t* values;
  int valueCount;
  int nextValue;
  int bitsLeft;
  uint32_t bits;
} BitStream;

void bitstream_init(BitStream* stream, const uint8_t* data, const int size)
{
  stream->values = (const uint16_t*)data;
  stream->valueCount = size;
  stream->nextValue = 0;
  stream->bits = 0;
  stream->bitsLeft = 0;
}

int bitstream_read(BitStream* stream, int bits)
{
  ASSERT(bits >= 0 && bits <= 16);
  if (bits == 0) return 0;
  if (stream->bitsLeft < bits)
  {
    if (stream->nextValue >= stream->valueCount) return 0;
    stream->bits |= stream->values[stream->nextValue++] << stream->bitsLeft;
    stream->bitsLeft += 16;
  }
  uint16_t result = stream->bits & ((1 << bits) - 1);
  stream->bits >>= bits;
  stream->bitsLeft -= bits;
  return result;
}

int bitstream_readString(BitStream* stream, int lengthBits, char* out)
{
  int length = bitstream_read(stream, lengthBits);
  for (int i = 0; i < length; i++)
    *out++ = ' ' + bitstream_read(stream, 6);
  *out = 0;
  return length;
}

int bitstream_remaining(const BitStream* stream)
{
  return (stream->valueCount - stream->nextValue) * 16 + stream->bitsLeft;
}

bool bitstream_hasAtLeast(const BitStream* stream, int bits)
{
  return bitstream_remaining(stream) >= bits;
}

#define BITS_PACKETTYPE 5
typedef enum PacketType
{
  PACKET_NOP,
  PACKET_RESET,
  PACKET_O2,
  PACKET_CO2,
  PACKET_HEADING,
  PACKET_BODYNAME,
  PACKET_PLAYERFLAGS,
  PACKET_PLANETSTATS,
  PACKET_TIME,
  PACKET_LOCATIONNAME,
  PACKET_PERSONALEFFECTS,
  PACKET_ENVEFFECTS,
  PACKET_POSALERT,
  PACKET_NEGALERT,
  PACKETTYPE_COUNT
} PacketType;

void handle_effects_packet(BitStream* stream, int maxCount, int firstEffect, EffectIcon* effects)
{
  int streamCount = bitstream_read(stream, BITS_EFFECTS);
  int count = maxCount < streamCount ? maxCount : streamCount;
  int i;
  for (i = 0; i < count; i++)
    effects[i] = firstEffect + bitstream_read(stream, BITS_EFFECTS);
  bitstream_read(stream, (streamCount - i) * BITS_EFFECTS);
  for (; i < maxCount; i++)
    effects[i] = EFFECT_ICON_NONE;
}

StateChanges handle_packet(const uint8_t* data, const int size)
{
  StateChanges changes = 0;
  BitStream stream;
  bitstream_init(&stream, data, size);
  while (bitstream_hasAtLeast(&stream, BITS_PACKETTYPE))
  {
    PacketType type = bitstream_read(&stream, BITS_PACKETTYPE);
    switch(type)
    {
      case PACKET_NOP: break;
      case PACKET_RESET:
        memset(game.personalEffects, 0, sizeof(game.personalEffects));
        memset(game.envEffects, 0, sizeof(game.envEffects));
        changes |= STATE_PERSONALEFFECTS | STATE_ENVEFFECTS;
        break;
      case PACKET_O2:
        game.o2 = bitstream_read(&stream, BITS_O2CO2);
        changes |= STATE_O2CO2;
        break;
      case PACKET_CO2:
        game.co2 = bitstream_read(&stream, BITS_O2CO2);
        changes |= STATE_O2CO2;
        break;
      case PACKET_HEADING:
        game.heading = bitstream_read(&stream, BITS_HEADING);
        changes |= STATE_HEADING;
        break;
      case PACKET_PLAYERFLAGS:
        game.playerFlags = bitstream_read(&stream, BITS_PLAYERFLAGS);
        changes |= STATE_PLAYERFLAGS;
        break;
      case PACKET_BODYNAME:
        bitstream_readString(&stream, BITS_BODYNAME, game.bodyName);
        changes |= STATE_NAMES;
        break;
      case PACKET_PLANETSTATS:
        game.planetGrav = bitstream_read(&stream, BITS_PLANETGRAV);
        game.planetTemperature = bitstream_read(&stream, 11) - 294;
        game.planetOxygen = bitstream_read(&stream, 7);
        changes |= STATE_PLANETSTATS;
        break;
      case PACKET_TIME:
        game.time = bitstream_read(&stream, 6);
        changes |= STATE_TIME;
        break;
      case PACKET_LOCATIONNAME:
        bitstream_readString(&stream, BITS_LOCNAME, game.locationName);
        changes |= STATE_NAMES;
        break;
      case PACKET_PERSONALEFFECTS:
        handle_effects_packet(&stream,
          MAX_PERSONALEFFECTS, EFFECT_ICON_FIRST_PERSONAL, game.personalEffects);
        changes |= STATE_PERSONALEFFECTS;
        break;
      case PACKET_ENVEFFECTS:
        handle_effects_packet(&stream,
          MAX_ENVEFFECTS, EFFECT_ICON_FIRST_ENVIRONMENTAL, game.envEffects);
        changes |= STATE_ENVEFFECTS;
        break;
      case PACKET_POSALERT:
      case PACKET_NEGALERT:
      {
        GameAlert alert;
        alert.positive = type == PACKET_POSALERT;
        alert.kind = bitstream_read(&stream, BITS_ALERTKIND);
        bitstream_readString(&stream, BITS_ALERTTEXT, alert.title);
        bitstream_readString(&stream, BITS_ALERTTEXT, alert.subtitle);
        app_handle_gamealert(&alert);
      }break;
      default:
        APP_LOG(APP_LOG_LEVEL_WARNING, "Got invalid packet type: %d", type);
        return changes;
    }
  }
  return changes;
}

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

static void in_dropped_handler(AppMessageResult reason, void* context)
{
  APP_LOG(APP_LOG_LEVEL_WARNING, "Msg dropped: %s", translate_error(reason));
}

char s_text_buffer[64] = {0};
static void in_received_handler(DictionaryIterator* received, void* context)
{
  Tuple* tuple = dict_find(received, 16);
  if (tuple == NULL || tuple->type != TUPLE_BYTE_ARRAY || tuple->length < 2)
  {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Msg received is invalid");
    return;
  }
  StateChanges changes = handle_packet(tuple->value->data, tuple->length);
  if (changes != 0)
    app_handle_gamestate(changes);
}

void communication_init()
{
  app_message_register_inbox_dropped(in_dropped_handler);
  app_message_register_inbox_received(in_received_handler);
  checkAppMsg(app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, 64));
}
