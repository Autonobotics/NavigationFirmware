__author__ = 'Dan'


# Logging Constants
class ABLogConstant():
    CONFIG_HOST = 'localhost'
    CONFIG_PORT = 9999


# Define Status Constants Class
class ABStatus():
    STATUS_SUCCESS = 0
    STATUS_FAILURE = -1


# Defines the Pin Configuration of Board
class ABPinConfig(): # TODO: Update Pin values
    # HC_SR04 Sensor Trigger Pins
    HC_SR04_SHARED_TRIGGER      = 16
    HC_SR04_LEFT_AXIS_TRIGGER   = 1
    HC_SR04_RIGHT_AXIS_TRIGGER  = 1
    HC_SR04_REAR_AXIS_TRIGGER   = 1
    HC_SR04_FRONT_AXIS_TRIGGER  = 1
    HC_SR04_TOP_AXIS_TRIGGER    = 1
    HC_SR04_BOTTOM_AXIS_TRIGGER = 1

    # HC_SR04 Sensor Echo Pins
    HC_SR04_LEFT_AXIS_ECHO      = 1
    HC_SR04_RIGHT_AXIS_ECHO     = 1
    HC_SR04_REAR_AXIS_ECHO      = 1
    HC_SR04_FRONT_AXIS_ECHO     = 20
    HC_SR04_TOP_AXIS_ECHO       = 1
    HC_SR04_BOTTOM_AXIS_ECHO    = 1
