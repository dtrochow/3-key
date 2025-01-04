import subprocess
import logging
from serial.tools import list_ports


def configure_logging():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s - %(levelname)s - %(message)s"
    )


def check_picocom_installed():
    """Check if picocom is installed."""
    try:
        subprocess.run(["picocom", "--help"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        logging.info("picocom is installed.")
    except FileNotFoundError:
        logging.error("picocom is not installed. Please install picocom using your package manager (e.g., 'sudo apt install picocom').")
        exit(1)
    except subprocess.CalledProcessError:
        logging.error("picocom is not installed or not functioning correctly. Please reinstall it.")
        exit(1)


def find_pico_device():
    """Find the Raspberry Pi Pico serial device."""
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


def connect_to_pico(pico_port):
    """Connect to the Raspberry Pi Pico using picocom."""
    try:
        logging.info(f"Connecting to Pico on port {pico_port} using picocom...")
        subprocess.run(["picocom", pico_port], check=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to connect to Pico using picocom: {e}")
        exit(1)


def main():
    configure_logging()
    check_picocom_installed()

    try:
        pico_port = find_pico_device()
        connect_to_pico(pico_port)
    except Exception as e:
        logging.error(f"An error occurred: {e}")
        exit(1)


if __name__ == "__main__":
    main()
