import subprocess
import os
import logging
import argparse
import platform

CONTAINER_REPOSITORY    = "dtrochow"
CONTAINER_NAME          = "3-key"
CONTAINER_VERSION       = "v0.1.0"

logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")


def get_container_name():
    """Get the full container name."""
    machine_arch = platform.machine()
    if "arm" in machine_arch:
        return f"{CONTAINER_REPOSITORY}/{CONTAINER_NAME}:arm.{CONTAINER_VERSION}"
    if "x86" in machine_arch or "i386" in machine_arch or "amd64" in machine_arch:
        return f"{CONTAINER_REPOSITORY}/{CONTAINER_NAME}:amd.{CONTAINER_VERSION}"
    else:
        logging.error(f"Unsupported architecture: {machine_arch}")
        exit(1)


def check_and_pull_image():
    """Check if the Docker image exists locally and pull it if not."""
    CONTAINER_FULL_NAME = get_container_name()
    try:
        result = subprocess.run(
            ["docker", "images", "-q", CONTAINER_FULL_NAME],
            capture_output=True,
            text=True,
            check=True
        )
        if not result.stdout.strip():
            logging.info(f"Image {CONTAINER_FULL_NAME} not found locally. Pulling from Docker Hub...")
            subprocess.run(["docker", "pull", CONTAINER_FULL_NAME], check=True)
        else:
            logging.info(f"Image {CONTAINER_FULL_NAME} found locally.")
    except subprocess.CalledProcessError as e:
        logging.error(f"Error checking or pulling Docker image: {e}")
        exit(1)


def run_docker_command(command, interactive=False):
    """Run a command inside the Docker container."""
    check_and_pull_image()

    docker_cmd = [
        "docker", "run", "--rm",
        "-v", f"{os.getcwd()}:/app",
        "-v", f"{os.getcwd()}:/output",
        "-w", "/app",
        "-p", "8000:8000",
    ]
    if interactive:
        docker_cmd.append("-it")
    
    docker_cmd.append(get_container_name())
    docker_cmd.extend(command)
    
    logging.info(f"Running Docker command: {' '.join(docker_cmd)}")
    
    subprocess.run(docker_cmd, check=True)


def build_firmware():
    """Build firmware in the Docker container."""
    logging.info("Building firmware inside Docker container...")
    run_docker_command(["python", "build.py", "-c"])
    logging.info("Firmware build process completed.")


def serve_docs():
    """Serve documentation using mkdocs inside the Docker container."""
    logging.info("Serving documentation with mkdocs...")
    run_docker_command(["mkdocs", "serve", "-a", "0.0.0.0:8000"])


def main():
    parser = argparse.ArgumentParser(description="Run Docker commands for 3-key project.")
    parser.add_argument("-d", "--doc", action="store_true", help="Serve the documentation using mkdocs.")
    parser.add_argument("-b", "--build", action="store_true", help="Build the firmware.")
    args = parser.parse_args()

    try:
        if args.doc:
            serve_docs()
        elif args.build:
            build_firmware()
        else:
            run_docker_command(["/bin/bash"], interactive=True)
    except subprocess.CalledProcessError as e:
        logging.error(f"Error during the process: {e}")
        exit(1)
    except KeyboardInterrupt:
        logging.info("Shutting down...")
        exit(0)


if __name__ == "__main__":
    main()
