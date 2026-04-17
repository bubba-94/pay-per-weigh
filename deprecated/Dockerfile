FROM ubuntu:24.04

# Shippable testing environment for Dekstops. Old version

WORKDIR /pay-per-weigh

# Install runtime dependencies
RUN apt-get update && \
    apt-get install -y libsdl2-2.0-0 libsdl2-image-2.0-0 libsdl2-ttf-2.0-0 libgpiod2 && \
    rm -rf /var/lib/apt/lists/*

COPY assets/ ./assets/
COPY bin/x86/pay-per-weigh .
COPY bin/x86/lib_ppw-x86.a .

RUN chmod +x ./pay-per-weigh

# Set runtime directory for SDL
ENV XDG_RUNTIME_DIR=/tmp/runtime-dir
RUN mkdir -p $XDG_RUNTIME_DIR

CMD ["./pay-per-weigh"]
