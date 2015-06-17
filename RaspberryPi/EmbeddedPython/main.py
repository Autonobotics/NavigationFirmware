__author__ = 'Dan'

from common import ABStatus
import ab_log as ABLog
import time


def main():
    # Get Logger instance
    logger = ABLog.get_logger('main')
    logger.info("Starting application loop")

    # Configure the Ultrasonic Sensor Pins


    # Configure the IR Distance Sensor Pins

    # Configure the IR Tracking Sensor Pins

    # Configure the Camera and OpenCV

    # Configure the I2C Interface to the main board

    # Wait for the Controller Board to Signal Ready

    # Perform main Loop


    return ABStatus.STATUS_SUCCESS


# Application Start
if __name__ == "__main__":
    print "Initializing Logging..."
    ABLog.initialize_logging()

    # Start the main loop
    print "Starting Application..."
    status = main()
    print "Application Exit Requested..."
    print "Cleaning Logging..."
    ABLog.cleanup_logging()

    print "Exiting with status: {0!s}".format(status)
