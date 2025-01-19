import subprocess
import os
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")


def build_firmware():
    """Build firmware in the Docker container."""
    logging.info("Building firmware inside Docker container...")
    docker_cmd = [
        "docker", "run", "--rm",
        "-v", f"{os.getcwd()}:/app",
        "-v", f"{os.getcwd()}:/output",
        "-w", "/app", "3key", "python", "build.py", "--clean"
    ]
    subprocess.run(docker_cmd, check=True)
    logging.info("Firmware build process completed.")

def main():
    try:
        build_firmware()
    except subprocess.CalledProcessError as e:
        logging.error(f"Error during the process: {e}")
        exit(1)

if __name__ == "__main__":
    main()
