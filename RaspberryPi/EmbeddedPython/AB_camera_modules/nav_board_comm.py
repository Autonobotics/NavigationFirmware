__author__ = 'Pravjot'
from ARMPIT import protocol as ARMPit
from ARMPIT import message
import beacon_processing
import numpy as np
from AB_Logging import ab_log as AB_Log

class ABFlags():
    BEACON_DETECTED     = 0x01
    NO_BEACON_DETECTED  = 0x02
    BEACON_ROTATION     = 0x03
    FRONTAL_COLLISION   = 0x04
    QUERY_ROTATION      = 0x05


def send_and_wait(data, flag):
    if flag == ABFlags.BEACON_DETECTED:
        while send_beacon_detected(data):
            pass
    elif flag == ABFlags.NO_BEACON_DETECTED:
        while send_no_beacon():
            pass
    elif flag == ABFlags.QUERY_ROTATION:
        while query_drone_rotation():
            pass
    elif flag ==  ABFlags.BEACON_ROTATION:
        while send_beacon_rotation(data):
            pass
    elif flag == ABFlags.FRONTAL_COLLISION:
        while send_edge_distance(data):
            pass

def send_beacon_detected(distance_vector):
    #distance_vector  = [X Y DISTNACE FROM CAMERA TO CENTER OF BEACON]
    beacon_message = message.BeaconDetectedMessage().set_defaults()
    #load the data into the message
    beacon_message.x_distance = np.int_(distance_vector.x)
    beacon_message.y_distance = np.int_(distance_vector.y)
    beacon_message.z_distance = np.int_(distance_vector.z)

    #send the message
    ARMPit.uart_transmit(beacon_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()

    if received_message is True:
        #correct flag was received
        return True
    else:
        #uart_logger.error("CMD_BEACON_DETECTED was not received! RECEIVED FLAG: {0}".format(received_message.flag))
        #the wrong flag was detected!
        return False

def send_no_beacon():
    no_beacon_message = message.NoBeaconMessage().set_defaults()

    #send the message
    ARMPit.uart_transmit(no_beacon_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()

    if received_message is True:
        #correct flag was received
        return True
    else:
        #uart_logger.error("CMD_NO_BEACON was not received! RECEIVED FLAG: {0}".format(received_message.flag))
        #the wrong flag was detected!
        return False

def send_edge_distance(distance):
    edge_message = message.EdgeDetectedMessage().set_defaults()

    #load the data into the message
    edge_message.x_distance = distance

    #send message
    ARMPit.uart_transmit(edge_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()

    if received_message is True:
        #correct flag was received
        return True
    else:
        #uart_logger.error("CMD_EDGE_DETECTED was not received! RECEIVED FLAG: {0}".format(received_message.flag))
        #the wrong flag was detected!
        return False

def send_beacon_rotation(rotation):
    rotation_message = message.BeaconRotationMessage().set_defaults()
    rotation_message.x_rotation = rotation

    #send message
    ARMPit.uart_transmit(rotation_message)
    #recieve and process the response from the nav board
    received_message = receive_and_process()

    if received_message is True:
        #correct flag was received
        return True
    else:
        #the wrong flag was detected!
        #uart_logger.error("CMD_BEACON_ROTATION was not received! RECEIVED FLAG: {0}".format(received_message.flag))
        return False

def query_drone_rotation():
    query_drone_rot = message.QueryRotationMessage().set_defaults()

    #send message
    ARMPit.uart_transmit(query_drone_rot)
    #receive amd process the response from the nav board
    received_message = receive_and_process()

    if received_message is True:
        return True
    else:
        #uart_logger.error("CMD_QUERY_ROTATION was not received! RECEIVED FLAG: {0}".format(received_message.flag))
        return False


def receive_and_process():
    #global uart_logger
    #wait for response
    cmd = ARMPit.uart_receive_cmd()
    if cmd is message.ARMPiTMessage.CMD_ACK:
        received_package = ARMPit.uart_receive_packet()
        return True
    if cmd is message.ARMPiTMessage.CMD_RACK:
        received_package = ARMPit.uart_receive_packet()
        received_message = message.RackMessage().load_from_string_with_command(cmd, received_package)

        if received_message.flag is not message.ARMPiTMessage.FLAG_END:
            uart_logger.error("Uart RackMessage was malformed")
            return False

        if received_message.sub_cmd is message.ARMPiTMessage.SUBCMD_COLLISION_DETECTED:
            distance_to_object = received_message.payload_a
            beacon_processing.frontal_collision(distance_to_object)
            return True
        elif received_message.sub_cmd is message.ARMPiTMessage.SUBCMD_ROTATION_COMPLETE:
            return True

