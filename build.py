#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess

BUILD_CMD = (
    ["cmake", "-G", "MinGW Makefiles", ".."] if os.name == "nt" else ["cmake", ".."]
)

EXE_PATH = (
    os.path.join("build", "sudu.exe")
    if os.name == "nt"
    else os.path.join("build", "sudu")
)

def clean():
    """Cleans the build directory and removes python artifacts."""
    print("Cleaning build artifacts...")

    if os.path.exists("build"):
        shutil.rmtree("build")
    if os.path.exists("__pycache__"):
        shutil.rmtree("__pycache__")

    print("Clean complete.")


def build():
    """Creates the build directory and uses Cmake to build the program."""
    print("Building...")
    os.makedirs("build", exist_ok=True)

    subprocess.check_call(BUILD_CMD, cwd="build")
    subprocess.check_call(["make"], cwd="build")

    print("Build complete.")


def run():
    """Looks for the program and runs it."""
    if not os.path.exists(EXE_PATH):
        print(f"sudu executable was not found!")
        return
    subprocess.check_call([EXE_PATH])

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="suduc build")
    parser.add_argument("-c", "--clean", action="store_true")
    parser.add_argument("-b", "--build", action="store_true")
    parser.add_argument("-r", "--run", action="store_true")
    args = parser.parse_args()

    if args.clean:
        clean()

    if args.build:
        build()

    if args.run:
        run()
