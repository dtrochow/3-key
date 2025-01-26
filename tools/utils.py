import logging
from serial.tools import list_ports


def find_pico_device():
    # Find the Raspberry Pi Pico serial device
    pico_pid = "000a"
    logging.info("Searching for the Raspberry Pi Pico device...")

    ports = [
        port for port in list_ports.comports()
        if pico_pid.lower() in port.hwid.lower()
    ]

    if not ports:
        logging.error("Raspberry Pi Pico not found. Ensure the device is connected and try again.")
        exit(1)

    pico_port = ports[0].device
    logging.info(f"Pico found on port {pico_port}.")
    return pico_port
