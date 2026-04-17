#!/usr/bin/env python3
import os
import shutil
import subprocess
import sys

from colorama import Fore, Style

PRINT_PREFIX = f"-- {Fore.YELLOW}build.py >>> "

# If true, the script will attempt to establish an SSH connection to the target device before building. 
# This ensures that the build process only proceeds if the target device is reachable and can receive the built binaries.
ssh_connection = True

# OS Specific paths
ROOT = os.path.dirname(os.path.abspath(__file__))
SDK = os.path.join(os.path.expanduser("~"), "moody")

# Repository specific directories
BUILD_DIR = os.path.join(ROOT, "build")
BIN_DIR = os.path.join(ROOT, "bin")


# SDK dirs and files
SYSROOTS = os.path.join(SDK, "sysroots")
INSTALL_DIR = os.path.join(SYSROOTS, "rpi-aarch64")
TOOLCHAIN_FILE = os.path.join(ROOT, "toolchain-aarch64.cmake")

os.putenv("ENV_SYSROOT", SYSROOTS)

# Pkgconfig
PKG_CONFIG_SYSROOT_DIR = INSTALL_DIR
PKG_CONFIG_PATH = os.path.join(INSTALL_DIR, "usr", "lib", "aarch64-linux-gnu", "pkgconfig")
PKG_CONFIG_LIBDIR = PKG_CONFIG_PATH

# Export shared environment variables for pkg-config
os.putenv("PKG_CONFIG_PATH", PKG_CONFIG_PATH)
os.putenv("PKG_CONFIG_SYSROOT_DIR", PKG_CONFIG_SYSROOT_DIR)
os.putenv("PKG_CONFIG_LIBDIR", PKG_CONFIG_LIBDIR)


def build():
    cout("Starting build process.")
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)
        cout("Creating build directory.")
    if not os.path.exists(BIN_DIR):
        os.makedirs(BIN_DIR)
        cout("Creating bin directory")


    setup_cmake()

    print_target()

    subprocess.run([
        "cmake",
        "--build", BUILD_DIR,
        "--parallel", str(os.cpu_count())
    ], check=True)

def setup_cmake():
    cout("Setting up CMake")
    CMAKE_FLAGS = [
        f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN_FILE}",
        f"-DCMAKE_INSTALL_PREFIX={INSTALL_DIR}",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    ]
    subprocess.run([
        "cmake",
        "-S", ROOT,
        "-B", BUILD_DIR] 
        + CMAKE_FLAGS, 
        check=True)

def clean():
    if os.path.exists(BUILD_DIR):
        shutil.rmtree(BUILD_DIR)
    if os.path.exists(BIN_DIR):
        shutil.rmtree(BIN_DIR)

def cout(cmd):
    print(f"{PRINT_PREFIX}{cmd}{Style.RESET_ALL}")

def print_target():
    cout(f"Target:            Aarch64 Linux (Raspberry Pi)")
    cout(f"Sysroot:           {INSTALL_DIR}")
    cout(f"Toolchain File:    {TOOLCHAIN_FILE}")
    cout(f"Output dir:        {BIN_DIR}")
    cout(f"Build cache:       {BUILD_DIR}")

def copy_binary_to(user, device):
    cout(f"Copying binary to: {device}")

    remote_path = f"/home/{user}/Programs/pay-per-weigh/"

    if not os.path.exists(BIN_DIR):
        cout(f"Local binary directory does not exist: {BIN_DIR}")
        sys.exit(1)

    sources = [os.path.join(BIN_DIR, name) for name in os.listdir(BIN_DIR)]
    if not sources:
        cout(f"No files found in {BIN_DIR} to copy.")
        sys.exit(1)

    scp_command = ["scp", "-r"] + sources + [f"{user}@{device}:{remote_path}"]

    try:
        subprocess.run(scp_command, check=True)
        cout("Binaries copied successfully.")
    except subprocess.CalledProcessError as e:
        cout(f"Failed to copy binary: {e}")
        sys.exit(1)

def check_connection(user, device):
    if len(sys.argv) != 3:
        cout("Provide username and device/IP-address as arguments for the target device.")
        sys.exit(1)
    try:
        ssh_cmd = ["ssh", "-o", "BatchMode=yes", "-o", "ConnectTimeout=5", f"{user}@{device}", "exit"]
        subprocess.run(ssh_cmd, check=True)
        cout("Successfully connected to device. Proceeding with build.")

    except subprocess.CalledProcessError as e:
            cout(f"Failed to connect to device: {e}")
            cout("Format: ./build.py <username> <device_ip>")
            cout("Info: A successful ssh connection with username and device/IP-address are required to copy the binary to the target device after building.")
            cout("Example: ./build.py joe@device")
            sys.exit(1)

# If true, the script will attempt to establish an SSH connection to the target device before building. 
# This ensures that the build process only proceeds if the target device is reachable and can receive the built binaries.
ssh_connection = True

if __name__ == "__main__":
    # Expect and check incoming target environment
    if ssh_connection == True:
        check_connection(sys.argv[1], sys.argv[2])
        user = sys.argv[1]
        device = sys.argv[2]
    

    try:
        clean()
        build()
        
        if ssh_connection == True:
            copy_binary_to(user, device)
    except subprocess.CalledProcessError as e:
        cout(f"Build failed: {e}\n")
        sys.exit(1)