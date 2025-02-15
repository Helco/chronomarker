import uuid
from libpebble2.communication import PebbleConnection
from libpebble2.communication.transports.qemu import *
from libpebble2.communication.transports.websocket import *
from libpebble2.services.appmessage import *

# pebble = PebbleConnection(QemuTransport("127.0.0.1", 41563))
pebble = PebbleConnection(WebsocketTransport("ws://127.0.0.1:38375"))
pebble.connect()

appmessage = AppMessageService(pebble)
appmessage.send_message(uuid.UUID("4a022ac1-39ae-4903-b538-ea5c035a0e81"), { 16: Uint8(137) })

