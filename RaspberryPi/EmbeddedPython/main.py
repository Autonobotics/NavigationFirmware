__author__ = 'Dan'
from common import ABStatus
from AB_Logging import ab_log as ABLog
from ARMPIT import protocol, message
import camera_module
import RPi.GPIO as GPIO

def main():
    # Get Logger instance
    logger = ABLog.get_logger('main')
    logger.info("Starting application loop")

    # Perform any Camera and OpenCV config and Init

    # Initialize the UART
    logger.info("UART Communication Initializing.")
    protocol.initialize_serial_port()

    #Set GPIO13 to high for STM board to check before performing handshake
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(13, GPIO.OUT)
    #make sure the pin is intially low
    GPIO.setup(13, False)
    GPIO.output(13, True)


    # Perform Handshake
    while protocol.perform_handshake() is not True:
        logger.info("Attempting handshake")
        pass
    logger.info("Finished UART Handshake.")

    # Perform main Loop
    camera_module.camera_loop()

    # Cleanup and Return
    logger.info("Cleaning up UART Resources.")
    protocol.cleanup_serial_port()
    return ABStatus.STATUS_SUCCESS


# Application Start
if __name__ == "__main__":
    print("Initializing Logging...")
    ABLog.initialize_logging()

    # Start the main loop
    print("Starting Application...")
    try:
        status = main()
    except KeyboardInterrupt:
        ABLog.get_logger('root').exception("Keyboard Interrupt Handled. Program Shutdown.")
        status = ABStatus.STATUS_FAILURE

    print("Application Exit Requested...")
    print("Cleaning Logging...")
    ABLog.cleanup_logging()

    print("Exiting with status: {0!s}".format(status))
