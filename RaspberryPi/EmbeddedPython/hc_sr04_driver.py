__author__ = 'Dan'

from common import ABStatus
from common import ABPinConfig
import ab_log as ABLog
import RPi.GPIO as GPIO
import time
import gc


class HCSR04Driver():
    """
    This Driver controls all the HC_SR04 Ultrasonic Sensors.
    """

    #Axis Constants
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
                    HC_SR04_AXIS_BOTTOM: (ABPinConfig.HC_SR04_BOTTOM_AXIS_TRIGGER, ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO)
                    }

    # Driver Logger
    HC_SR04_LOGGER = 'HCSR04'

    # Trigger Pulse Time
    TRIGGER_HOLD = 0.00001  # Time in Seconds, ie. 10 us

    # Driver Constructor
    def __init__(self):
        # Get an instance of the logger
        self.hc_sr04_log = ABLog.get_logger(HCSR04Driver.HC_SR04_LOGGER)

        # Initialized Axis Storage
        self.desired_axis_list = None

        # Pulse Sequence storage
        self.current_cycle_sequence = 1

        # Pulse time Storage
        self.pulse_durations = {ABPinConfig.HC_SR04_FRONT_AXIS_ECHO  : [],
                                ABPinConfig.HC_SR04_REAR_AXIS_ECHO   : [],
                                ABPinConfig.HC_SR04_LEFT_AXIS_ECHO   : [],
                                ABPinConfig.HC_SR04_RIGHT_AXIS_ECHO  : [],
                                ABPinConfig.HC_SR04_TOP_AXIS_ECHO    : [],
                                ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO : []}

        # Driver output
        self.distances = {ABPinConfig.HC_SR04_FRONT_AXIS_ECHO  : 0,
                          ABPinConfig.HC_SR04_REAR_AXIS_ECHO   : 0,
                          ABPinConfig.HC_SR04_LEFT_AXIS_ECHO   : 0,
                          ABPinConfig.HC_SR04_RIGHT_AXIS_ECHO  : 0,
                          ABPinConfig.HC_SR04_TOP_AXIS_ECHO    : 0,
                          ABPinConfig.HC_SR04_BOTTOM_AXIS_ECHO : 0}

    # Initialize the driver
    def initialize(self, axis_list):
        # Log
        self.hc_sr04_log.debug("Initializing HC_SR04 Driver")
        self.hc_sr04_log.debug("Sensor Axis: {0!s}".format(axis_list))

        # Configure the GPIO Trigger Pins for each enabled Axis
        for axis in axis_list:
            if axis in HCSR04Driver.HC_SR04_AXIS:
                GPIO.setup(HCSR04Driver.HC_SR04_AXIS[axis][0], GPIO.OUT)
                GPIO.output(HCSR04Driver.HC_SR04_AXIS[axis][0], False)

        # Configure the GPIO Echo Pins for each enabled Axis
        for axis in axis_list:
            if axis in HCSR04Driver.HC_SR04_AXIS:
                GPIO.setup(HCSR04Driver.HC_SR04_AXIS[axis][1], GPIO.IN)  # Echo Pin is IN
            else:
                self.hc_sr04_log.error("Unknown Axis given at Initialization. Pins may not be correctly configured.")
                return ABStatus.STATUS_FAILURE

        self.desired_axis_list = axis_list
        self.hc_sr04_log.debug("Allowing Sensor to Settle...")
        time.sleep(1)
        self.hc_sr04_log.debug("Initialization Successful.")
        return ABStatus.STATUS_SUCCESS

    def perform_individual_sense_cycle(self):
        # Log Trigger Pulse
        self.hc_sr04_log.debug("Pulsing Trigger for {0} time in cycle.".format(self.current_cycle_sequence))

        # Since the timing is sensitive, shutdown the Garbage collector
        if gc.isenabled():
            gc.disable()

        # Note: At this point all axis should be valid
        # Do each axis sense sequentially
        for axis in self.desired_axis_list:
            GPIO.output(HCSR04Driver.HC_SR04_AXIS[axis][0], True)
            time.sleep(HCSR04Driver.TRIGGER_HOLD)
            GPIO.output(HCSR04Driver.HC_SR04_AXIS[axis][0], False)

            # Busy Wait
            pulse_start = 0
            pulse_end = 0
            while GPIO.input(HCSR04Driver.HC_SR04_AXIS[axis][1]) == 0:
                pulse_start = time.time()
            while GPIO.input(HCSR04Driver.HC_SR04_AXIS[axis][1]) == 1:
                pulse_end = time.time()
            self.pulse_durations[HCSR04Driver.HC_SR04_AXIS[axis][1]].append(pulse_end - pulse_start)

        # Turn back on the Python Garbage Collection
        if not gc.isenabled():
            gc.enable()

    # Starts a new Sense Cycle and takes a measurement
    def start_sense_cycle(self):
        # Set the new pulse sequence to 0
        self.current_cycle_sequence = 1

        # Clean the old distances
        for key in self.distances.iterkeys():
            self.distances[key] = 0

        # Start the Sense Cycle
        self.hc_sr04_log.debug("Starting HC_SR04 Sense Cycles.")
        self.perform_individual_sense_cycle()
        return ABStatus.STATUS_SUCCESS

    # Increments the averaging value and takes a measurement
    def advance_sense_cycle(self):
        # Increment the Sense cycle number for averaging
        self.current_cycle_sequence += 1

        # Perform another Sense
        self.perform_individual_sense_cycle()
        return ABStatus.STATUS_SUCCESS

    def finish_sense_cycle(self):
        # Calculate Averages and Store
        for axis in self.desired_axis_list:
            temp = 0
            for duration in self.pulse_durations[HCSR04Driver.HC_SR04_AXIS[axis][1]]:
                distance = round(duration * 17150, 2)
                temp += distance
            # Clean out the old durations
            self.pulse_durations[HCSR04Driver.HC_SR04_AXIS[axis][1]][:] = []
            # Calculate the Averaged distance
            self.distances[HCSR04Driver.HC_SR04_AXIS[axis][1]] = round(temp / self.current_cycle_sequence, 2)
            # Log the calculated average distance
            message = "Distance on axis \"{0}\": {1} cm".format(axis,
                                                                self.distances[HCSR04Driver.HC_SR04_AXIS[axis][1]])
            self.hc_sr04_log.debug(message)

        # For Profiling purposes, log the start time of the sense cycle
        self.hc_sr04_log.info("Sense Cycle End.")
        return ABStatus.STATUS_SUCCESS
