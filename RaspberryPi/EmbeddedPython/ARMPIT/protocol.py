__author__ = 'Dan'
from serial import Serial
from AB_Logging import ab_log as AB_Log
import message


serial_port = None
uart_logger = None


def translate_command_byte(cmd_byte_str):
    return ord(cmd_byte_str)


def initialize_serial_port():
    global serial_port, uart_logger
    serial_port = Serial("/dev/ttyAMA0", 115200)
    if serial_port.isOpen() is False:
        serial_port.open()
    serial_port.flushInput()
    serial_port.flushOutput()
    uart_logger = AB_Log.get_logger('UART')


def cleanup_serial_port():
    global serial_port
    serial_port.close()


# Read a Byte from the UART (Blocking) and translate to CMD
def uart_receive_cmd():
    return translate_command_byte(serial_port.read(1))


# Read the remaining Bytes from the UART (Blocking)
def uart_receive_packet():
    return serial_port.read(15)


# Write a ARMPIT Message to the UART
def uart_transmit(armpit_message):
    global uart_logger

    sent = serial_port.write(armpit_message.get_byte_stream())
    if sent != 16:
        uart_logger.error("Uart did not send the expected 16 Bytes. Sent {0} bytes instead.", sent)


def perform_handshake():
    global uart_logger

    # Wait for the Sync message
    cmd = uart_receive_cmd()
    if cmd is not message.ARMPiTMessage.CMD_SYNC:
        uart_logger.error("Uart did not receive SYNC during handshake.")
    sync_package = uart_receive_packet()

    # Validate the Sync packet
    sync_message = message.SyncMessage().load_from_string_with_command(cmd, sync_package)
    if sync_message.flag is not message.ARMPiTMessage.FLAG_END:
        uart_logger.error("Uart SYNC packet was malformed.")

    # Sent a response Sync message
    response_sync = message.SyncMessage().set_defaults()
    uart_transmit(response_sync)

    # Wait for the Ack packet before continuing
    cmd = uart_receive_cmd()
    if cmd is not message.ARMPiTMessage.CMD_ACK:
        uart_logger.error("Uart did not receive ACK during handshake.")
    sync_package = uart_receive_packet()