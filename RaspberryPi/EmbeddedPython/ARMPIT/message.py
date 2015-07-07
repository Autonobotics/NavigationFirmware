__author__ = 'Dan'
import struct
from array import array

DEBUG = 1

def expand_short_to_bytes(short):
    return [(short >> (8*i)) & 0xFF for i in range(1, -1, -1)]


def expand_int_to_bytes(integer):
    return [(integer >> (8*i)) & 0xFF for i in range(3, -1, -1)]


def collapse_bytes_to_short(byte_list):
    byte_array = array('B', byte_list)
    return struct.unpack_from('!h', byte_array)[0]


def collapse_bytes_to_int(byte_list):
    byte_array = array('B', byte_list)
    return struct.unpack_from('!i', byte_array)[0]


class ARMPiTMessage():

    CMD_INVD = 0x00
    CMD_SYNC = 0x01
    CMD_DYNC = 0xFE
    CMD_ACK = 0x02
    CMD_RACK = 0x03

    CMD_NO_BEACON = 0x30
    CMD_BEACON_DETECTED = 0x31
    CMD_EDGE_DETECTED = 0x32
    CMD_BEACON_ROTATION = 0x33
    CMD_QUERY_ROTATION = 0x34

    FLAG_END = 0xFF

    def __init__(self):
        self.buffer = [0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]
        self.cmd = self.buffer[0]

    def load_from_string_with_command(self, cmd_input, string_input):
        temp = map(ord, string_input)
        if len(temp) is 15:
            self.buffer = temp
            self.buffer.insert(0, cmd_input)
            self.cmd = cmd_input
        return self

    def load_from_string(self, string_input):
        temp = map(ord, string_input)
        if len(temp) is 16:
            self.buffer = temp
            self.cmd = self.buffer[0]
        return self

    def _update(self):
        self.buffer[0] = self.cmd

    def get_byte_stream(self):
        if DEBUG: print("Sending: {0}".format("".join(map(chr, self.buffer))))
        return "".join(map(chr, self.buffer))


class SyncMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[7]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[7]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_SYNC
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[7] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class AckMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[1]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[1]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_ACK
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class RackMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[7]
        self.sub_cmd = self.buffer[1]
        self.axis = self.buffer[2]
        self.payload_a = 0x0000
        self.payload_b = 0x0000

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.sub_cmd = self.buffer[1]
        self.axis = self.buffer[2]
        self.payload_a = collapse_bytes_to_short(self.buffer[4:6])
        self.payload_b = collapse_bytes_to_short(self.buffer[6:8])
        self.flag = self.buffer[8]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_RACK
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.sub_cmd
        self.buffer[2] = self.axis
        self.buffer[4:6] = expand_short_to_bytes(self.payload_a)
        self.buffer[6:8] = expand_short_to_bytes(self.payload_b)
        self.buffer[8] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class NoBeaconMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[1]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[1]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_NO_BEACON
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class BeaconDetectedMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.x_distance = 0x0000
        self.y_distance = 0x0000
        self.z_distance = 0x0000
        self.flag = self.buffer[8]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.x_distance = collapse_bytes_to_short(self.buffer[2:4])
        self.y_distance = collapse_bytes_to_short(self.buffer[4:6])
        self.z_distance = collapse_bytes_to_short(self.buffer[6:8])
        self.flag = self.buffer[8]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_BEACON_DETECTED
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[2:4] = expand_short_to_bytes(self.x_distance)
        self.buffer[4:6] = expand_short_to_bytes(self.y_distance)
        self.buffer[6:8] = expand_short_to_bytes(self.z_distance)
        self.buffer[8] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class EdgeDetectedMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.x_distance = 0x0000
        self.flag = self.buffer[4]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.x_distance = collapse_bytes_to_short(self.buffer[2:4])
        self.flag = self.buffer[4]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_EDGE_DETECTED
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[2:4] = expand_short_to_bytes(self.x_distance)
        self.buffer[4] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class BeaconRotationMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.x_rotation = 0x0000
        self.flag = self.buffer[4]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.x_rotation = collapse_bytes_to_short(self.buffer[2:4])
        self.flag = self.buffer[4]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_BEACON_ROTATION
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[2:4] = expand_short_to_bytes(self.x_rotation)
        self.buffer[4] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)


class QueryRotationMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[1]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[1]
        return self

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_QUERY_ROTATION
        self.flag = ARMPiTMessage.FLAG_END
        return self

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.flag

    def get_byte_stream(self):
        self._update()
        return ARMPiTMessage.get_byte_stream(self)