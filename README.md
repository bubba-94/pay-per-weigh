# pay-per-weigh

## Overview

`pay-per-weigh` is an internship project and a cross-platform graphical application targeting Raspberry Pi 5 devices. It uses SDL2 to render a fullscreen kiosk-style interface, integrates with a payment request API, and supports Pi-specific GPIO and serial device handling.

## What it does

- Renders graphics and text. 
- Displays weight/status information on screen.
- Possibility to interact with a payment service.
- Supports Raspberry Pi GPIO and device I/O.

## Features

- Fullscreen / borderless display
- Image rendering
- QR code rendering(no logic) / endpoint rendering support
- Color and text rendering
- Payment API integration lightweight [mock Swish API](https://github.com/bubba-94/swish-mock)
- Performance profiling support

## Repository structure
```bash
pay-per-weigh
├── Doxyfile                # Config for doxygen
├── Makefile                # Doxygen makefile
├── assets                  # PNGs and TTF files
├── bin                     # Output 
├── build.py                # Script for building the application
├── docs                    # Collection of documents
├── include                 # Header files
├── src                     # Source files
├── main.cpp                # Application 
└── toolchain-aarch64.cmake # Toolchain for building 
```

## Build requirements

- CMake 3.15 or newer
- C++17-capable compiler
- **Featured C/C++ libraries**: 
  - SDL2 
  - SDL2_image
  - SDL2_ttf
  - nlohmann_json
  - PkgConfig
  - libcamera (not implemented, only compiles)
- For Raspberry Pi cross-build: sysroot and toolchain files

## Build instructions

### Using the provided script

`./build.py`


The resulting binary is placed in:

- `bin/aarch64/pay-per-weigh`

### Cross-build notes

`build.py` relies on environment variables and toolchain files in your home directory:

I have used my own ["SDK"](https://github.com/bubba-94/moody) where i store the sysroot and related libraries locally and scripts/configuartions remotely.

## Running the application

Copy the ARM output to the Pi (binary and archive):

```bash
scp -r bin/ user@pi:Programs/pay-per-weigh/
```

>! 

Then run the binary on the Pi.

If the screen is rotated, you can adjust output orientation with:

```bash
wlr-randr --output HDMI-A-1 --transform 90
```

## Docker

A published Docker image is available at `moodin/pay-per-weigh:v1.0`.

```bash
docker pull moodin/pay-per-weigh:v1.0

docker run -e DISPLAY=$DISPLAY \
  -it \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --name pay-per-weigh \
  moodin/pay-per-weigh:v1.0
```

## Testing and profiling

- Test against the payment mock server in the linked Swish mock repository.
- Use `perf report` to inspect ARM performance data after recording.

## Notes

- `main.cpp` creates an `Application` and a `Client` instance.
- Graphics and font rendering depend on SDL2 and its image/font extensions.

## License

See `LICENSE` for license terms.