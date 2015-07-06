__author__ = 'Dan'
from common import ABStatus
from AB_Logging import ab_log as ABLog
from ARMPIT import protocol, message


def main():
    # Get Logger instance
    logger = ABLog.get_logger('main')
    logger.info("Starting application loop")

    # Perform any Camera and OpenCV config and Init

    # Initialize the UART
    protocol.initialize_serial_port()

    # Perform Handshake
    protocol.perform_handshake()

    # Perform main Loop
    status = ABStatus.STATUS_SUCCESS
    while status is ABStatus.STATUS_SUCCESS:
        break

    # Cleanup and Return
    protocol.cleanup_serial_port()
    return ABStatus.STATUS_SUCCESS


# Application Start
if __name__ == "__main__":
    print("Initializing Logging...")
    ABLog.initialize_logging()

    # Start the main loop
    print("Starting Application...")
    status = main()
    print("Application Exit Requested...")
    print("Cleaning Logging...")
    ABLog.cleanup_logging()

    print("Exiting with status: {0!s}".format(status))
