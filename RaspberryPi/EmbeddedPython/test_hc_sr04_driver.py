__author__ = 'Dan'

import RPi.GPIO as GPIO
import hc_sr04_driver as HCSR04


# Application Start
if __name__ == "__main__":
    print("Starting HCSR04 Test...")
    HCSR04.initialize([HCSR04.HC_SR04_AXIS_FRONT,
                       HCSR04.HC_SR04_AXIS_REAR,
                       HCSR04.HC_SR04_AXIS_LEFT,
                       HCSR04.HC_SR04_AXIS_RIGHT])
    HCSR04.start_sense_cycle()
    while not HCSR04.sense_cycle_complete:
        None
    print("Axis           Duration")
    for key, value in HCSR04.pulse_duration.iteritems():
        print("{0}          {1}".format(key, value))
    print("Test Complete.")