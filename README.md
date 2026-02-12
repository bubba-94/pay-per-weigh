# Takeover

## Description

Take over the Raspberry Monitor opening a terminal that renderes graphics [SDL2 Library](https://wiki.libsdl.org/SDL3/FrontPage).

**Options:**
- [] Boot terminal when Raspberry Pi starts
- [x] Fullscreen / borderless
- [x] Render a picture 
- [x] Render a QR code if possible, start with JPEG and later retrive endpoint from an API
- [x] Colors
- [x] Text
- [] Intergration with Swish API

## Building 

### Prerequisites

- Sysroots of target headers
- Pre installed libraries SDL2, SDL2 image, SDL2 ttf.

build.sh will build and compile executables for both PC and Pi 5 and gather them under separate architecture folders under bin/

## Testing

### Testing on the PC 

- `./run.sh` from root repo to run the application (requires preinstalled libraries)

- [Dockerfile here](Dockerfile) to run a container with installed libraries. 
- Docker command after succesful build ([or Docker Hub image found here](https://hub.docker.com/r/moodin/pay-per-weigh)): 
```bash
docker pull moodin/pay-per-weigh:v1.0

docker run -e DISPLAY=$DISPLAY \
           -v /tmp/.X11-unix:/tmp/.X11-unix \
           --name pay-per-weigh \
           pay-per-weigh:v1.0
```


- To create a virtual pair of serial ports.
```bash
socat \
  pty,raw,echo=0,link=/tmp/ttyRS232_A \
  pty,raw,echo=0,link=/tmp/ttyRS232_B &'
```

### Test program for writing analog value to the Pi via Serial 

- Use for testing the rendering. 

```cpp
#include <Arduino.h>

constexpr int POT = A0;

constexpr int ANALOG_MIN = 0;
constexpr int ANALOG_MAX = 1023;
constexpr int WEIGHT_MIN = 0;
constexpr int WEIGHT_MAX = 15000;

int readValue = 0;
int convValue = 0;

int readPot();

void setup()
{
    Serial.begin(9600);
    while (!Serial){}
}

void loop()
{
    readValue = readPot();
    convValue = map(readValue, ANALOG_MIN, ANALOG_MAX, WEIGHT_MIN, WEIGHT_MAX);

    Serial.println(convValue);
    delay(16);
}

int readPot() {
    long sum = 0;
    const int samples = 100;

    for (int i = 0; i < samples; i++) {
        sum += analogRead(POT);
        delayMicroseconds(300);
    }

    return sum / samples;
}
```

### Running on Pi

- Binary with the library is needed
- Run `scp -r bin/aarch64 user@pi:Programs/pay-per-weigh/` to copy binary and library to Pi.
- Turn screen 90 degrees `wlr-randr --output HDMI-A-1 --transform 90`.