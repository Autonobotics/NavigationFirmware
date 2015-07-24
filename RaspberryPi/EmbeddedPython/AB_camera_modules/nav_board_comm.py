__author__ = 'Pravjot'
import numpy as np

from ARMPIT import protocol as ARMPit
from ARMPIT import message
import beacon_processing
from AB_Logging import ab_log as AB_Log

#intialize logger
cam_logger = AB_Log.get_logger('AB_CAMERA_MODULE')

#various flags for the camera module
class ABFlags():
    BEACON_DETECTED     = 0x01
    NO_BEACON_DETECTED  = 0x02
    BEACON_ROTATION     = 0x03
    AVOID_FRONT   = 0x04
    QUERY_ROTATION      = 0x05

    ACK_RECEIVED             = 0x30
    QUERY_ROTATION_COMPLETE  = 0x31
    COLLISION_DETECTED       = 0x32

    STATUS                  = 0x00

#handler for sending different messages to the navigation board
def send_and_wait(data, flag):
    global cam_logger

    if flag == ABFlags.BEACON_DETECTED:
        while send_beacon_detected(data) is not True:
            pass
    elif flag == ABFlags.NO_BEACON_DETECTED:
        while send_no_beacon() is not True:
            pass
    elif flag == ABFlags.QUERY_ROTATION:
        while query_drone_rotation() is not True:
            pass
    elif flag ==  ABFlags.BEACON_ROTATION:
        while send_beacon_rotation(data) is not True:
            pass
    elif flag == ABFlags.AVOID_FRONT:
        while send_edge_distance(data) is not True:
            pass

def send_beacon_detected(distance_vector):
    global cam_logger

    #distance_vector  = [X Y DISTNACE FROM CAMERA TO CENTER OF BEACON]
    beacon_message = message.BeaconDetectedMessage().set_defaults()

    #load the data into the message
    beacon_message.x_distance = np.int16(32767**distance_vector.x)
    beacon_message.y_distance = np.int16(32767*distance_vector.y)
    beacon_message.z_distance = np.int16(distance_vector.z)
    cam_logger.info('BEACON: sending location ({0}, {1}, {2})'.format(beacon_message.x_distance, beacon_message.y_distance, beacon_message.z_distance))

    #send the message
    ARMPit.uart_transmit(beacon_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()
    cam_logger.debug("BEACON: received response: {0}".format(received_message))

    if received_message == ABFlags.ACK_RECEIVED:
        #correct flag was received
        return True
    else:
        cam_logger.error("CMD_BEACON_DETECTED was not received!")
        #the wrong flag was detected!
        return False

def send_no_beacon():
    global cam_logger

    no_beacon_message = message.NoBeaconMessage().set_defaults()

    #recieve and process the response from the nav board
    cam_logger.info('BEACON: sending no beacon')

    #send the message
    ARMPit.uart_transmit(no_beacon_message)

    received_message = receive_and_process()
    cam_logger.debug("BEACON: received response: {0}".format(received_message))

    if received_message == ABFlags.ACK_RECEIVED:
        #correct flag was received
        cam_logger.info("BEACON: ACK received")
        return True
    else:
        cam_logger.error("CMD_NO_BEACON was not received!")
        #the wrong flag was detected!
        return False

def send_edge_distance(distance):
    global cam_logger

    edge_message = message.EdgeDetectedMessage().set_defaults()

    #load the data into the message
    edge_message.x_distance = distance
    cam_logger.info("BEACON: sending edge distance")
    #send message
    ARMPit.uart_transmit(edge_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()
    cam_logger.debug("BEACON: received response: {0}".format(received_message))
    if received_message == ABFlags.ACK_RECEIVED:
        cam_logger.info("BEACON: ACK received")
        #correct flag was received
        return True
    else:
        cam_logger.error("CMD_EDGE_DETECTED was not received!")
        #the wrong flag was detected!
        return False

#send the rotation for the next beacon location
def send_beacon_rotation(rotation):
    global cam_logger

    rotation_message = message.BeaconRotationMessage().set_defaults()
    rotation_message.x_rotation = np.int16(rotation)

    cam_logger.debug("BEACON: sending next beacon rotation: {0}".format(rotation))
    #send message
    ARMPit.uart_transmit(rotation_message)

    #recieve and process the response from the nav board
    received_message = receive_and_process()
    cam_logger.debug("BEACON: received response: {0}".format(received_message))
    #check to see if we got a ACk message back
    if received_message == ABFlags.ACK_RECEIVED:
        #correct flag was received
        cam_logger.info('BEACON: ACK received')
        return True
    else:
        #the wrong flag was detected!
        cam_logger.error("CMD_BEACON_ROTATION was not received!")
        return False

#query for the drones current rotation action
def query_drone_rotation():
    global cam_logger
    query_drone_rot = message.QueryRotationMessage().set_defaults()

    cam_logger.debug('BEACON: Query rotation')
    #send message
    ARMPit.uart_transmit(query_drone_rot)

    #receive amd process the response from the nav board
    received_message = receive_and_process()
    cam_logger.debug("BEACON: received response: {0}".format(received_message))
    if received_message == ABFlags.QUERY_ROTATION_COMPLETE:
        cam_logger.info('BEACON: Query rotation complete received')
        return True
    else:
        cam_logger.error("CMD_QUERY_ROTATION was not received!")
        return False

#receive the message from the nav board and process it as needed
def receive_and_process():
    global cam_logger
    #global uart_logger
    #wait for response
    cmd = ARMPit.uart_receive_cmd()
    cam_logger.debug('Received Command: {0}'.format(cmd))

    #simple ack
    if cmd is message.ARMPiTMessage.CMD_ACK:
        cam_logger.debug('ACK')
        received_package = ARMPit.uart_receive_packet()
        return ABFlags.ACK_RECEIVED

    #case we get a RACK from the board, processes the information
    if cmd is message.ARMPiTMessage.CMD_RACK:
        cam_logger.debug('RACK')
        received_package = ARMPit.uart_receive_packet()
        received_message = message.RackMessage().load_from_string_with_command(cmd, received_package)
        cam_logger.debug('Received Message: {0}'.format(received_message))

        #check to see if the message is valid
        if received_message.flag is not message.ARMPiTMessage.FLAG_END:
            cam_logger.error("Uart RackMessage was malformed")
            return False
        #checking for frontal collision message
        if received_message.sub_cmd is message.ARMPiTMessage.SUBCMD_COLLISION_DETECTED:
            cam_logger.debug('SUBCMD_COLLISION_DETECTED')
            distance_to_object = received_message.payload_a
            ABFlags.STATUS = ABFlags.COLLISION_DETECTED
            beacon_processing.collision_distance = distance_to_object
            return ABFlags.COLLISION_DETECTED
        #checking for rotation complete message
        elif received_message.sub_cmd is message.ARMPiTMessage.SUBCMD_ROTATION_COMPLETE:
            cam_logger.debug('SUBCMD_ROTATION_COMPLETE')
            return ABFlags.QUERY_ROTATION_COMPLETE

    if cmd is message.ARMPiTMessage.CMD_INVD:
        cam_logger.error('INVALID COMMAND!')
