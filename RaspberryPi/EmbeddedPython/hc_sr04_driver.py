__author__ = 'Dan'

from common import ABStatus
from common import ABPinConfig
import ab_log as ABLog
import RPi.GPIO as GPIO
import time
import gc

HC_SR04_AXIS_FRONT  = 'front'
HC_SR04_AXIS_REAR   = 'rear'
HC_SR04_AXIS_LEFT   = 'left'
HC_SR04_AXIS_RIGHT  = 'right'
HC_SR04_AXIS_TOP    = 'top'
HC_SR04_AXIS_BOTTOM = 'bottom'

# Pin Mapping Lookup Table
HC_SR04_AXIS = {HC_SR04_AXIS_FRONT:  (ABPinConfig.HC_SR04_FRONT_AXIS_TRIGGER,  ABPinConfig.HC_SR04_FRONT_AXIS_ECHO),
                HC_SR04_AXIS_REAR:   (ABPinConfig.HC_SR04_REAR_AXIS_TRIGGER,   ABPinConfig.HC_SR04_REAR_AXIS_ECHO),
                HC_SR04_AXIS_LEFT:   (ABPinConfig.HC_SR04_LEFT_AXIS_TRIGGER,   ABPinConfig.HC_SR04_LEFT_AXIS_ECHO),
                HC_SR04_AXIS_RIGHT:  (ABPinConfig.HC_SR04_RIGHT_AXIS_TRIGGER,  ABPinConfig.HC_SR04_RIGHT_AXIS_ECHO),
                HC_SR04_AXIS_TOP:    (ABPinConfig.HC_SR04_TOP_AXIS_TRIGGER,    ABPinConfig.HC_SR04_TOP_AXIS_ECHO),
                HC_SR04_AXIS_BOTTOM: (ABPinConfig.HC_SR04_BOTTOM_AXIS_TRIGGER, ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO)}

# Driver Logger
HCSR04_LOGGER = 'HCSR04'
hc_sr04_log = None

# Shared Trigger Flag
trigger_is_shared = False

# Trigger Pulse Time
TRIGGER_HOLD = 0.00001  # Time in Seconds, ie. 10 us

# Driver Initialized Flag
is_initialized = False

# Sense Cycle Complete Flag
sense_cycle_complete = False

# Stored Initialized Axis List
desired_axis_list = None

# Axis Triggered List
still_waiting = []

# Pulse Sequence storage
current_trigger_sequence = 0
TRIGGER_SEQUENCE_LIMIT = 3

# Pulse time Storage
initial_pulse_time = {ABPinConfig.HC_SR04_FRONT_AXIS_ECHO  : 0,
                      ABPinConfig.HC_SR04_REAR_AXIS_ECHO   : 0,
                      ABPinConfig.HC_SR04_LEFT_AXIS_ECHO   : 0,
                      ABPinConfig.HC_SR04_RIGHT_AXIS_ECHO  : 0,
                      ABPinConfig.HC_SR04_TOP_AXIS_ECHO    : 0,
                      ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO : 0}

pulse_duration = {ABPinConfig.HC_SR04_FRONT_AXIS_ECHO  : 0,
                  ABPinConfig.HC_SR04_REAR_AXIS_ECHO   : 0,
                  ABPinConfig.HC_SR04_LEFT_AXIS_ECHO   : 0,
                  ABPinConfig.HC_SR04_RIGHT_AXIS_ECHO  : 0,
                  ABPinConfig.HC_SR04_TOP_AXIS_ECHO    : 0,
                  ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO : 0}


# Initialize the Driver
def initialize(axis_list, shared_trigger):
    # Get an instance of the logger
    global hc_sr04_log
    hc_sr04_log = ABLog.get_logger(HCSR04_LOGGER)
    hc_sr04_log.debug("Initializing HC_SR04 Driver")
    hc_sr04_log.debug("Shared Trigger: {0!s}".format(shared_trigger))
    hc_sr04_log.debug("Sensor Axis: {0!s}".format(axis_list))

    # Configure the GPIO Trigger Pins for each enabled Axis
    if shared_trigger:
        # Using a Shared Trigger Pins
        GPIO.setup(ABPinConfig.HC_SR04_SHARED_TRIGGER, GPIO.OUT, pull_up_down=GPIO.PUD_DOWN)  # Trigger Pin is OUT

        # Set Default value to low
        GPIO.output(ABPinConfig.HC_SR04_SHARED_TRIGGER, False)

    else:
        # Independent Trigger Pins
        for axis in axis_list:
            if axis in HC_SR04_AXIS:
                GPIO.setup(HC_SR04_AXIS[axis][0], GPIO.OUT, pull_up_down=GPIO.PUD_DOWN)
                GPIO.output(HC_SR04_AXIS[axis][0], False)

    # Configure the GPIO Echo Pins for each enabled Axis
    for axis in axis_list:
        if axis in HC_SR04_AXIS:
            GPIO.setup(HC_SR04_AXIS[axis][1], GPIO.IN)  # Echo Pin is IN
            GPIO.add_event_detect(HC_SR04_AXIS[axis][1], GPIO.BOTH, callback=edge_callback_wrapper, bouncetime=2)
        else:
            hc_sr04_log.error("Unknown Axis given at Initialization. Pins may not be correctly configured.")
            return ABStatus.STATUS_FAILURE

    global is_initialized
    global desired_axis_list
    global trigger_is_shared
    is_initialized = True
    desired_axis_list = axis_list
    trigger_is_shared = shared_trigger
    hc_sr04_log.debug("Initialization Successful.")
    return ABStatus.STATUS_SUCCESS


def pulse_trigger(pulse_sequence):
    # Log Trigger Pulse
    global hc_sr04_log
    hc_sr04_log.debug("Pulsing Trigger for {0} time in cycle.".format(pulse_sequence))

    # Add the triggered axis to the waiting list by port number
    global desired_axis_list
    for axis in desired_axis_list:
        still_waiting.append(axis[1])
    hc_sr04_log.debug("Still Waiting List: {0}".format(still_waiting))

    # If the trigger is shared, pulse it, else pulse each trigger individually
    if trigger_is_shared:
        # Trigger the shared pin
        GPIO.output(ABPinConfig.HC_SR04_SHARED_TRIGGER, True)
        time.sleep(TRIGGER_HOLD)
        GPIO.output(ABPinConfig.HC_SR04_SHARED_TRIGGER, False)

    else:
        # Note: At this point all axis should be valid
        for axis in desired_axis_list:
            GPIO.output(HC_SR04_AXIS[axis][0], True)
            time.sleep(TRIGGER_HOLD)
            GPIO.output(HC_SR04_AXIS[axis][0], False)


def start_sense_cycle():
    # Ensure Pulse Trigger is not called before Initialization
    global hc_sr04_log
    if not is_initialized:
        hc_sr04_log.error("Cannot proceed. HC_SR04 Driver has not been initialized.")
        return ABStatus.STATUS_FAILURE

    # For Profiling purposes, log the start time of the sense cycle
    hc_sr04_log.info("Sense Cycle Start: {0}".format(time.time()))

    # Since the timing is sensitive, shutdown the Garbage collector
    if gc.isenabled():
        hc_sr04_log.debug("Python Garbage Collection Disabled.")
        gc.disable()

    # Set the new pulse sequence to 0 and the sense cycle complete to False
    global current_trigger_sequence
    global sense_cycle_complete
    current_trigger_sequence = 0
    sense_cycle_complete = False

    # Start the Trigger Cycle
    hc_sr04_log.debug("Starting HC_SR04 Sense Cycles.")
    pulse_trigger(current_trigger_sequence)
    return ABStatus.STATUS_SUCCESS


def edge_callback_wrapper(channel):
    hc_sr04_log.debug("Channel in Edge Callback: {0}".format(channel))
    hc_sr04_log.debug("Pin is {0}".format(GPIO.input(channel)))
    if GPIO.input(channel):
        # Pin is high, meaning Rising Edge
        rising_edge_callback(channel)
    else:
        # Pin is low, meaning Falling Edge
        falling_edge_callback(channel)


def rising_edge_callback(channel):
    # Store the Initial Time of the Pulse
    hc_sr04_log.debug("In Rising Edge Callback")
    global initial_pulse_time
    initial_pulse_time[channel] = -time.time()


def falling_edge_callback(channel):
    # Compute the duration of the pulse
    hc_sr04_log.debug("In Falling Edge Callback")
    global initial_pulse_time
    global pulse_duration
    pulse_duration[channel] = initial_pulse_time[channel] + time.time()

    # Remove the Axis from the waiting list
    still_waiting.remove(channel)

    # If all of the Axis are done waiting, determine the next sense cycle
    if 0 == len(still_waiting):
        global sense_cycle_complete
        sense_cycle_complete = True    # TODO: Extend to variable length sequence with Averaging

        # For Profiling purposes, log the start time of the sense cycle
        hc_sr04_log.info("Sense Cycle End: {0}".format(time.time()))
        gc.enable()