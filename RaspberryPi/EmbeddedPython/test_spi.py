__author__ = 'Dan'

import spidev
import time

spi = spidev.SpiDev()
spi.open(0, 0)

SPI_MASTER_SYNBYTE = 0xAC
SPI_SLAVE_SYNBYTE = 0x53

SPI_ACK_BYTES = 0xA5A5
SPI_NACK_BYTES = 0xDEAD


def ReverseBits(byte):
    byte = ((byte & 0xF0) >> 4) | ((byte & 0x0F) << 4)
    byte = ((byte & 0xCC) >> 2) | ((byte & 0x33) << 2)
    byte = ((byte & 0xAA) >> 1) | ((byte & 0x55) << 1)
    return byte


def sync():
    print "Sending"
    print hex(SPI_MASTER_SYNBYTE)
    print "Expecting"
    print hex(SPI_SLAVE_SYNBYTE)
    test = spi.xfer2([SPI_MASTER_SYNBYTE])
    print "Received"
    print hex(test[0])
    reverse = ReverseBits(test[0])
    print "Reversed"
    print hex(reverse)
    if reverse != SPI_SLAVE_SYNBYTE:
        print "Sync did not return SPI slave Sync bite"
    else:
        print "Sync return Sync value"

    print "Sending"
    print hex(SPI_MASTER_SYNBYTE)
    print "Expecting"
    print hex(SPI_SLAVE_SYNBYTE)
    test = spi.xfer2([SPI_MASTER_SYNBYTE])
    print "Received"
    print hex(test[0])
    reverse = ReverseBits(test[0])
    print "Reversed"
    print hex(reverse)
    if reverse != SPI_SLAVE_SYNBYTE:
        print "Sync did not return SPI slave Sync bite"
    else:
        print "Sync return Sync value"

#try:
#    while True:
#        sync()
#
#except KeyboardInterrupt:
#    spi.close()

sync()
spi.close()