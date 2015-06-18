__author__ = 'Dan'

import RPi.GPIO as GPIO
import hc_sr04_driver as HCSR04
import ab_log as ABLog


# Application Start
if __name__ == "__main__":
    print("Starting HCSR04 Test...")
    ABLog.initialize_logging()
    GPIO.setmode(GPIO.BCM)
    HCSR04.initialize([HCSR04.HC_SR04_AXIS_FRONT], True)
    HCSR04.start_sense_cycle()
    while not HCSR04.sense_cycle_complete:
        None
    print("Axis           Duration")
    for key, value in HCSR04.pulse_duration.iteritems():
        print("{0}          {1}".format(key, value))
    ABLog.cleanup_logging()
    print("Test Complete.")