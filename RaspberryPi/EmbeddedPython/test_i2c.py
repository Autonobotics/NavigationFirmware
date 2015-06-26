__author__ = 'Dan'

#!/usr/bin/python

import smbus

bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x3E      #7 bit address (will be left shifted to add the read write bit)
READ_CMD = 0x12
WRITE_CMD = 0x34

test_data = list(bytearray(" ****I2C_TwoBoards ****"))


#Write the string
bus.write_i2c_block_data(DEVICE_ADDRESS, WRITE_CMD, test_data)

#Read back the string
back_data = bus.read_12c_block_data(DEVICE_ADDRESS, READ_CMD)

print back_data