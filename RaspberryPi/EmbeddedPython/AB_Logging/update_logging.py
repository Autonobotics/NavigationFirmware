#!/usr/bin/env python
__author__ = 'Dan'

import socket
import sys
import struct

from AB_Logging.log_common import ABLogConstant


with open(sys.argv[1], 'rb') as f:
    data_to_send = f.read()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print('connecting...')
s.connect((ABLogConstant.CONFIG_HOST, ABLogConstant.CONFIG_PORT))
print('sending config...')
s.send(struct.pack('>L', len(data_to_send)))
s.send(data_to_send)
s.close()
print('complete')