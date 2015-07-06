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


testa = 603996232
testb = 16456

testc = [0x24, 0x00, 0x40, 0x48]
testd = [0x40, 0x48]

print "Test expand_int_to_bytes"
print expand_int_to_bytes(testa)

print "Test expand_short_to_bytes"
print expand_short_to_bytes(testb)

print "Test collapse_bytes_to_short"
print collapse_bytes_to_short(testd)

print "Test collapse_bytes_to_int"
print collapse_bytes_to_int(testc)