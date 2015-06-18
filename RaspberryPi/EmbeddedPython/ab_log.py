__author__ = 'Dan'

from common import ABLogConstant
import logging
import logging.config


# Store the Log Configuration Thread Reference
log_configuration_thread = None


# Initialize the Logging Setup
def initialize_logging():
    global log_configuration_thread
    logging.config.fileConfig('log_config.conf')
    log_configuration_thread = logging.config.listen(ABLogConstant.CONFIG_PORT)
    log_configuration_thread.start()


# Get an instance of the logger
def get_logger(category):
    return logging.getLogger(category)


# Clean the Threading Configuration Log
def cleanup_logging():
    global log_configuration_thread
    logging.config.stopListening()
    log_configuration_thread.join()
