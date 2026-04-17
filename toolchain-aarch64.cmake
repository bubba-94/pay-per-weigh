# Configuration for arm64

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)

# Incoming evnironment variable for sysroot
set(MY_SYSROOT "$ENV{ENV_SYSROOT}")

# Path to sysroot
set(SYSROOT "${MY_SYSROOT}/rpi-aarch64")

# Force the compiler to look inside SYSROOT
set(CMAKE_SYSROOT "${SYSROOT}")
set(CMAKE_SYSROOT_COMPILE "${SYSROOT}")
set(CMAKE_SYSROOT_LINK "${SYSROOT}")

# Force flags
set(RPI TRUE)
set(CMAKE_C_FLAGS "--sysroot=${SYSROOT} ${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "--sysroot=${SYSROOT} ${CMAKE_CXX_FLAGS}")

# Find libraries in the sysroot
set(CMAKE_FIND_ROOT_PATH 
    "${SYSROOT}"
)

# Search only in sysroot for programs, libraries, includes
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Coloring and status
set(CMAKE_COLOR_DIAGNOSTICS ON CACHE INTERNAL "")
string(ASCII 27 Esc)
set(GREEN   "${Esc}[1;32m" CACHE INTERNAL "")
set(GREENRESET       "${Esc}[0m" CACHE INTERNAL "")
set(STARTGREEN  "${GREEN}------------------------------" CACHE INTERNAL "")
set(ENDGREEN    "------------------------------${GREENRESET}" CACHE INTERNAL "")
