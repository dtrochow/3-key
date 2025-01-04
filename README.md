# 3-key

This project is focused on the [3-Key Development Board from Waveshare](https://www.waveshare.com/rp2040-keyboard-3.htm).

It introduces the base firmware written in C++ using the Pico-SDK.

Version [v0.0.2](https://github.com/dtrochow/3-key/releases/tag/v0.0.2) of the firmware adds CTRL+C/CTRL+V capabilities and simple RGB color support (each button has its own color).

The [build.py](https://github.com/dtrochow/3-key/blob/main/build.py) script can be used only with the FW versions grater than **v0.1.0**.
All previous versions require [the manual firmware update process](https://3-key.dtrochow.ovh/development/#manually).

![3-key hardware](docs/assets/gifs/3-key.gif)

## Serial Terminal

To connect to the Serial Terminal the `connect_serial.py` script can be used.

``` terminal
python3 connect_serial.py
```

It finds the RPi Pico serial device name and connects to it using `picocom` tool.

All requirements are listed in `requirements.txt` file.

## Documentation

The detailed documentation is available here: [3-key Project Documentation](https://3-key.dtrochow.ovh/)
