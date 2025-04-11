import os
import shutil
import subprocess
import serial
import argparse
import logging
from serial.tools import list_ports
from time import sleep
from tools.utils import find_pico_device

UART_BAUD_RATE = 115200


def configure_logging():
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s - %(levelname)s - %(message)s"
    )


def get_absolute_path(relative_path):
    return os.path.abspath(os.path.join(os.path.dirname(__file__), relative_path))


def clean_build_directory(build_dir):
    if os.path.exists(build_dir):
        logging.info(f"Removing existing build directory: {build_dir}")
        shutil.rmtree(build_dir)


def build_firmware(clean, build_type):
    """Build the firmware using CMake and Make."""
    # Set the build type for CMake
    os.environ["BUILD_TYPE"] = build_type
    logging.info(f"Build type set to: {build_type}")

    # Set the build directory to be directly in the project root (using `build/`)
    build_dir = get_absolute_path("build")

    # Set the source directory (where CMakeLists.txt is)
    source_dir = get_absolute_path("firmware")  # Assuming CMakeLists.txt is in the firmware directory

    if clean:
        clean_build_directory(build_dir)

    os.makedirs(build_dir, exist_ok=True)

    try:
        logging.info("Running cmake ..")
        subprocess.run(["cmake", source_dir], cwd=build_dir, check=True)  # Specify the source directory for CMake
        logging.info("Running make -j8")
        subprocess.run(["make", "-j8"], cwd=build_dir, check=True)
        logging.info("Firmware build completed successfully.")
    except subprocess.CalledProcessError as e:
        logging.error(f"Build process failed: {e}")
        raise


def check_picotool_installed():
    """Check if picotool is installed."""
    try:
        subprocess.run(["picotool", "version"], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        logging.info("picotool is installed.")
    except FileNotFoundError:
        logging.error("picotool is not installed. Please install picotool from the following link: https://github.com/raspberrypi/picotool")
        exit(1)
    except subprocess.CalledProcessError:
        logging.error("picotool is not installed or not working properly. Please install picotool from the following link: https://github.com/raspberrypi/picotool")
        exit(1)


def enter_boot_mode():
    pico_pid = "000a"

    try:
        logging.info("Searching for the Raspberry Pi Pico device...")
        ports = [
            port for port in list_ports.comports()
            if pico_pid.lower() in port.hwid.lower()
        ]
        if not ports:
            raise Exception("Raspberry Pi Pico not found.")

        pico_port = ports[0].device
        logging.info(f"Pico found on port {pico_port}. Connecting...")
        with serial.Serial(pico_port, baudrate=115200, timeout=1) as ser:
            ser.write(b'reset\n')
            logging.info("Sent reset command to Pico.")
        sleep(3)

    except Exception as e:
        logging.error(f"Failed to enter boot mode: {e}")
        raise


def load_firmware():
    firmware_path = get_absolute_path("build/3-key.uf2")

    if not os.path.exists(firmware_path):
        raise FileNotFoundError(f"Firmware file not found at {firmware_path}")

    try:
        logging.info("Loading firmware to Pico...")
        subprocess.run(["picotool", "load", "-f", firmware_path], check=True)
        logging.info("Firmware loaded successfully.")

        logging.info("Rebooting the Pico to run the firmware...")
        subprocess.run(["picotool", "reboot"], check=True)
        logging.info("Pico rebooted successfully.")
    except subprocess.CalledProcessError as e:
        logging.error(f"Failed to load or reboot firmware: {e}")
        raise


def run_build_and_run_unit_tests():
    """Build and run unit tests."""
    test_dir = get_absolute_path("tests")
    build_dir = os.path.join(test_dir, "build")

    # Clean the build directory if it exists
    if os.path.exists(build_dir):
        logging.info(f"Removing existing test build directory: {build_dir}")
        shutil.rmtree(build_dir)

    os.makedirs(build_dir, exist_ok=True)

    try:
        logging.info("Running cmake for unit tests...")
        subprocess.run(["cmake", ".."], cwd=build_dir, check=True)
        logging.info("Building unit tests...")
        subprocess.run(["make", "-j8"], cwd=build_dir, check=True)
        logging.info("Running unit tests...")
        subprocess.run(["ctest", "--output-on-failure"], cwd=build_dir, check=True)
        logging.info("Unit tests completed successfully.")
    except subprocess.CalledProcessError as e:
        logging.error(f"Unit test process failed: {e}")
        raise


def send_erase_command():
    """Send the erase command to the device via UART."""
    pico_port = find_pico_device()
    try:
        logging.info(f"Connecting to Pico on port {pico_port} to send erase command...")
        with serial.Serial(pico_port, baudrate=UART_BAUD_RATE, timeout=1) as ser:
            ser.write(b'erase\n') 
            logging.info("Erase command sent successfully.")
    except Exception as e:
        logging.error(f"Failed to send erase command: {e}")
        raise


def main():
    configure_logging()

    check_picotool_installed()

    parser = argparse.ArgumentParser(description="Firmware build and load script for Raspberry Pi Pico.")
    parser.add_argument("-c", "--clean", action="store_true", help="Clean build directory before building firmware.")
    parser.add_argument("-t", "--type", choices=["Debug", "Release"], default="Release", help="Build type (Debug or Release). Default is Release.")

    parser.add_argument("-l", "--load", action="store_true", help="Load firmware to the device.")
    parser.add_argument("-e", "--erase", action="store_true", help="Erase device storage before loading firmware (optional).")

    parser.add_argument("-u", "--unittest", action="store_true", help="Build and run unit tests.")

    args = parser.parse_args()

    try:
        if args.clean:
            logging.info("Cleaning build directory.")
            clean_build_directory(get_absolute_path("build"))

        if args.unittest:
            logging.info("Building and running unit tests.")
            run_build_and_run_unit_tests()
            return

        logging.info(f"Building firmware with type: {args.type}.")
        build_firmware(clean=False, build_type=args.type)

        if args.load:
            if args.erase:
                logging.info("Erasing device storage before loading firmware.")
                send_erase_command()

            logging.info("Entering boot mode to load firmware.")
            enter_boot_mode()
            load_firmware()

    except Exception as e:
        logging.error(f"Script execution failed: {e}")
        exit(1)


if __name__ == "__main__":
    main()
