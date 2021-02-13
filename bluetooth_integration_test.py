from spike import PrimeHub, LightMatrix, Button, StatusLight, ForceSensor, MotionSensor, Speaker, ColorSensor, App, DistanceSensor, Motor, MotorPair
from spike.control import wait_for_seconds, wait_until, Timer
import utime
import ubluetooth
import ubinascii
import struct
from micropython import const

class BLEHandler:

    def __init__(self):
        # constants
        self.__IRQ_SCAN_RESULT = const(1 << 4)
        self.__IRQ_SCAN_COMPLETE = const(1 << 5)
        self.__IRQ_PERIPHERAL_CONNECT = const(1 << 6)
        self.__IRQ_PERIPHERAL_DISCONNECT = const(1 << 7)
        self.__IRQ_GATTC_SERVICE_RESULT = const(1 << 8)
        self.__IRQ_GATTC_CHARACTERISTIC_RESULT = const(1 << 9)
        self.__IRQ_GATTC_READ_RESULT = const(1 << 11)
        self.__IRQ_GATTC_NOTIFY = const(1 << 13)

        # enter device specific service and characteristic UUIDs (from nRF Connect app)
        self.__PERIPHERAL_SERVICE_UUID = ubluetooth.UUID(0xFFE0)
        self.__PERIPHERAL_SERVICE_CHAR = ubluetooth.UUID(0xFFE1)
        self.__DEVICE_ID = b'diN\x80*^'

        # class specific
        self.__ble = ubluetooth.BLE()
        self.__ble.active(True)
        self.__ble.irq(handler=self.__irq)
        self.__decoder = Decoder()
        self.__reset()

    def __reset(self):
        # cached data
        self.__addr = None
        self.__addr_type = None
        self.__adv_type = None
        self.__services = None
        self.__man_data = None
        self.__name = None
        self.__conn_handle = None
        self.__value_handle = None

        # reserved callbacks
        self.__scan_callback = None
        self.__read_callback = None
        self.__notify_callback = None
        self.__connected_callback = None
        self.__disconnected_callback = None

    # start scan for ble devices
    def scan_start(self, timeout, callback):
        self.__scan_callback = callback
        self.__ble.gap_scan(timeout, 30000, 30000)

    # stop current scan
    def scan_stop(self):
        self.__ble.gap_scan(None)

    # write gatt client data
    def write(self, data, adv_value=None):
        if not self.__is_connected():
            return
        if adv_value:
            self.__ble.gattc_write(self.__conn_handle, adv_value, data)
        else:
            self.__ble.gattc_write(self.__conn_handle, self.__value_handle, data)

    # read gatt client
    def read(self, callback):
        if not self.__is_connected():
            return
        self.__read_callback = callback
        self.__ble.gattc_read(self.__conn_handle, self.__value_handle)

    # connect to ble device
    def connect(self, addr_type, addr):
        self.__ble.gap_connect(addr_type, addr)
        
    # disconnect from ble device
    def disconnect(self):
        if not self.__is_connected():
            return
        self.__ble.gap_disconnect(self.__conn_handle)
        self.__reset()

    # get notification
    def on_notify(self, callback):
        self.__notify_callback = callback

    # get callback on connect
    def on_connect(self, callback):
        self.__connected_callback = callback

    # get callback on connect
    def on_disconnect(self, callback):
        self.__disconnected_callback = callback

    # +-------------------+
    # | Private Functions |
    # +-------------------+

    # connection status
    def __is_connected(self):
        return self.__conn_handle is not None

    # ble event handler
    def __irq(self, event, data):
        # called for every result of a ble scan
        if event == self.__IRQ_SCAN_RESULT:
            addr_type, addr, adv_type, rssi, adv_data = data
            print(ubinascii.hexlify(addr))
            print(self.__decoder.decode_services(adv_data), addr_type)
            #if self.__PERIPHERAL_SERVICE_UUID in self.__decoder.decode_services(adv_data):
            if bytes(self.__DEVICE_ID) == bytes(addr):
                print('device found')
                self.__addr_type = addr_type
                self.__addr = bytes(addr)
                self.__adv_type = adv_type
                self.__name = self.__decoder.decode_name(adv_data)
                print("Name=" + self.__name)
                self.__services = self.__decoder.decode_services(adv_data)
                print("Getting services")
                self.__man_data = self.__decoder.decode_manufacturer(adv_data)
                print("Getting manufacturer details")
                self.scan_stop()

        # called after a ble scan is finished
        elif event == self.__IRQ_SCAN_COMPLETE:
            if self.__addr:
                if self.__scan_callback:
                    self.__scan_callback(self.__addr_type, self.__addr, self.__man_data)
                self.__scan_callback = None
            else:
                self.__scan_callback(None, None, None)

        # called if a peripheral device is connected
        elif event == self.__IRQ_PERIPHERAL_CONNECT:
            print('Device connected')
            conn_handle, addr_type, addr = data
            self.__conn_handle = conn_handle
            self.__ble.gattc_discover_services(self.__conn_handle)

        # called if a peripheral device is disconnected
        elif event == self.__IRQ_PERIPHERAL_DISCONNECT:
            conn_handle, _, _ = data
            self.__disconnected_callback()
            if conn_handle == self.__conn_handle:
                self.__reset()

        # called if a service is returned
        elif event == self.__IRQ_GATTC_SERVICE_RESULT:
            print('getting service')
            conn_handle, start_handle, end_handle, uuid = data
            print(uuid)
            if conn_handle == self.__conn_handle and uuid == self.__PERIPHERAL_SERVICE_UUID:
                print('found service')
                self.__ble.gattc_discover_characteristics(self.__conn_handle, start_handle, end_handle)

        # called if a characteristic is returned
        elif event == self.__IRQ_GATTC_CHARACTERISTIC_RESULT:
            print('getting characteristics')
            conn_handle, def_handle, value_handle, properties, uuid = data
            print(uuid)
            if conn_handle == self.__conn_handle and uuid == self.__PERIPHERAL_SERVICE_CHAR:
                print('found characteristic')
                self.__value_handle = value_handle
                # finished discovering, connecting finished
                self.__connected_callback()

        # called if data was successfully read
        elif event == self.__IRQ_GATTC_READ_RESULT:
            conn_handle, value_handle, char_data = data
            if self.__read_callback:
                self.__read_callback(char_data)

        # called if a notification appears
        elif event == self.__IRQ_GATTC_NOTIFY:
            conn_handle, value_handle, notify_data = data
            if self.__notify_callback:
                self.__notify_callback(notify_data)

class Decoder:

    def __init__(self):
        self.__COMPANY_IDENTIFIER_CODES = {"4d48": "DSD Tech"}

    def decode_manufacturer(self, payload):
        man_data = []
        n = self.__decode_field(payload, const(0xFF))
        if not n:
            return []
        company_identifier = ubinascii.hexlify(struct.pack('<h', *struct.unpack('>h', n[0])))
        print("Identifier=" + company_identifier.decode())
        company_name = self.__COMPANY_IDENTIFIER_CODES.get(company_identifier.decode(), "?")
        print("Company Name=" + company_name)
        company_data = n[0][2:]
        man_data.append(company_identifier.decode())
        man_data.append(company_name)
        man_data.append(company_data)
        return man_data

    def decode_name(self, payload):
        n = self.__decode_field(payload, const(0x09))
        return str(n[0], "utf-8") if n else "parsing failed!"

    def decode_services(self, payload):
        services = []
        for u in self.__decode_field(payload, const(0x3)):
            services.append(ubluetooth.UUID(struct.unpack("<h", u)[0]))
        for u in self.__decode_field(payload, const(0x5)):
            services.append(ubluetooth.UUID(struct.unpack("<d", u)[0]))
        for u in self.__decode_field(payload, const(0x7)):
            services.append(ubluetooth.UUID(u))
        return services

    def __decode_field(self, payload, adv_type):
        i = 0
        result = []
        while i + 1 < len(payload):
            if payload[i + 1] == adv_type:
                result.append(payload[i + 2: i + payload[i] + 1])
            i += 1 + payload[i]
        return result


class BLEPeripheral:

    def __init__(self):
        # constants

        # class specific
        self.__handler = BLEHandler()

        # callbacks
        self.__connect_callback = None
        self.__disconnect_callback = None

    def connect(self, timeout=3000):
        self.__handler.on_connect(callback=self.__on_connect)
        self.__handler.on_disconnect(callback=self.__on_disconnect)
        self.__handler.on_notify(callback=self.__on_notify)
        self.__handler.scan_start(timeout, callback=self.__on_scan)

    def disconnect(self):
        self.__handler.disconnect()

    def on_button(self, callback):
        self.__button_callback = callback

    def on_connect(self, callback):
        self.__connect_callback = callback

    def on_disconnect(self, callback):
        self.__disconnect_callback = callback

    # +-------------------+
    # | Private Functions |
    # +-------------------+

    # callback for scan result
    def __on_scan(self, addr_type, addr, man_data):
        self.__handler.connect(addr_type, addr)

    def __on_connect(self):
        if self.__connect_callback:
            self.__connect_callback()

    def __on_disconnect(self):
        if self.__disconnect_callback:
            self.__disconnect_callback()
    
    def __on_notify(self, data):
        print("Data received=")
        print(data)

def on_connect():
    hub.status_light.on("azure")


def on_disconnect():
    hub.status_light.on("white")


# set up hub
hub = PrimeHub()

# create remote and connect
remote = BLEPeripheral()
utime.sleep(1)
remote.on_connect(callback=on_connect)
remote.on_disconnect(callback=on_disconnect)
remote.connect()
