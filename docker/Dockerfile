FROM ubuntu:24.10

ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    python3 \
    python3-pip \
    python3-venv \
    cmake \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    git \
    vim \
    libusb-1.0-0-dev \
    openocd \
    gdb-multiarch \
    picocom \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/raspberrypi/pico-sdk.git /opt/pico-sdk \
    && cd /opt/pico-sdk \
    && git submodule update --init \
    && echo "export PICO_SDK_PATH=/opt/pico-sdk" >> /etc/profile \
    && export PICO_SDK_PATH=/opt/pico-sdk

RUN rm -rf /opt/picotool \
    && export PICO_SDK_PATH=/opt/pico-sdk \
    && git clone https://github.com/raspberrypi/picotool.git /opt/picotool \
    && cd /opt/picotool \
    && git fetch --tags \
    && git checkout 2.1.0 \
    && mkdir build \
    && cd build \
    && cmake .. -DPICOTOOL_FORCE_FETCH_FROM_GIT=ON \
    && make \
    && make install

# Set environment variables
ENV PICO_SDK_PATH=/opt/pico-sdk
WORKDIR /app

# Install dependencies
RUN python3 -m venv /opt/venv
ENV PATH="/opt/venv/bin:$PATH"
ENV PATH="/opt/venv/bin:/usr/local/bin:$PATH"
COPY requirements.txt /app/requirements.txt
RUN pip install --no-cache-dir -r /app/requirements.txt

# Default command to run the build
CMD ["python", "build.py", "--clean"]
