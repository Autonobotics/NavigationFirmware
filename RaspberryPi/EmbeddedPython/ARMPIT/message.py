__author__ = 'Dan'
import struct
from array import array


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

    def load_from_string(self, string_input):
        temp = map(ord, string_input)
        if len(temp) is 16:
            self.buffer = temp
            self.cmd = self.buffer[0]

    def _update(self):
        self.buffer[0] = self.cmd

    def get_byte_stream(self):
        return "".join(map(chr, self.buffer))


class SyncMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[7]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[7]

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_SYNC
        self.flag = ARMPiTMessage.FLAG_END

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[7] = self.flag

    def get_byte_stream(self):
        self._update()
        ARMPiTMessage.get_byte_stream(self)


class AckMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[1]

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.flag = self.buffer[1]

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_ACK
        self.flag = ARMPiTMessage.FLAG_END

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.flag

    def get_byte_stream(self):
        self._update()
        ARMPiTMessage.get_byte_stream(self)


class RackMessage(ARMPiTMessage):

    def __init__(self):
        ARMPiTMessage.__init__(self)
        self.flag = self.buffer[7]
        self.sub_cmd = self.buffer[1]
        self.axis = self.buffer[2]
        self.distance = 0x00000000

    def load_from_string_with_command(self, cmd_input, string_input):
        ARMPiTMessage.load_from_string_with_command(self, cmd_input, string_input)
        self.sub_cmd = self.buffer[1]
        self.axis = self.buffer[2]
        self.distance = collapse_bytes_to_int(self.buffer[3:7])
        self.flag = self.buffer[7]

    def set_defaults(self):
        self.cmd = ARMPiTMessage.CMD_RACK
        self.flag = ARMPiTMessage.FLAG_END

    def _update(self):
        ARMPiTMessage._update(self)
        self.buffer[1] = self.sub_cmd
        self.buffer[2] = self.axis
        self.buffer[3:7] = expand_int_to_bytes(self.distance)
        self.buffer[7] = self.flag

    def get_byte_stream(self):
        self._update()
        ARMPiTMessage.get_byte_stream(self)