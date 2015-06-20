__author__ = 'Dan'

import RPi.GPIO as GPIO
from hc_sr04_driver import HCSR04Driver
import ab_log as ABLog


# Application Start
if __name__ == "__main__":
    print("Starting HCSR04 Test...")
    ABLog.initialize_logging()
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)

    sonic = HCSR04Driver()
    sonic.initialize([HCSR04Driver.HC_SR04_AXIS_FRONT,
                      HCSR04Driver.HC_SR04_AXIS_REAR])

    sonic.start_sense_cycle()      # 1
    sonic.finish_sense_cycle()     # Averaging

    ABLog.cleanup_logging()
    print("Test Complete.")
