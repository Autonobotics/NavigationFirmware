__author__ = 'Dan'

import const
import common
import logging
import logging.config
import time


def main_loop():
    print "Starting Main Loop"

    logger = logging.getLogger()
    try:
        # loop through logging calls to see the difference
        # new configurations make, until Ctrl+C is pressed
        while True:
            logger.debug('debug message')
            logger.info('info message')
            logger.warn('warn message')
            logger.error('error message')
            logger.critical('critical message')
            time.sleep(5)
    except KeyboardInterrupt:
        # cleanup
        logging.config.stopListening()
        t.join()

    return const.STATUS_SUCCESS


# Application Start
if __name__ == "__main__":
    # Perform any initial Environment Setup
    logging.config.fileConfig('log_config.conf')
    t = logging.config.listen(common.LOGGING_CONFIG_PORT)
    t.start()

    # Start the main loop
    status = main_loop()
    print "Exiting with status: {0!s}".format(status)
