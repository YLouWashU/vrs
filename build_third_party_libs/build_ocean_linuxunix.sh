#!/usr/bin/env bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set -e

if [[ $(uname -s) == "Darwin" ]]; then
  VRS_PLATFORM="macos"
elif [[ $(uname -s) == "Linux" ]]; then
  VRS_PLATFORM="linux"
elif [[ $(uname -s) == *"MINGW"* ]]; then
  VRS_PLATFORM="windows"
else
  echo "ERROR: Unsupported operating system: $(uname -s)" >&2
  exit 1
fi

# Exit if Windows
echo "Detected platform: $VRS_PLATFORM"
if [[ "$VRS_PLATFORM" == "windows" ]]; then
    echo "This script is intended for Unix-like systems (Linux/macOS)."
    exit 1
fi


################################################################################
# Build OCEAN
################################################################################

# Define the directory where OCEAN will be downloaded and built
INSTALL_PATH="$HOME/vrs_third_party_libs/ocean"
BUILD_DIR="$INSTALL_PATH/build"
OCEAN_3P_PATH="$HOME/vrs_third_party_libs/ocean/third-party"

# Check if OCEAN is already installed
if [ -d "${INSTALL_PATH}" ] && [ "$(ls -A "${INSTALL_PATH}")" ]; then
    echo "OCEAN is already installed at $INSTALL_PATH"
    exit 0
fi

# Create directories if it doesn't exist
mkdir -p "$BUILD_DIR"
mkdir -p "${OCEAN_3P_PATH}/install"
mkdir -p "${OCEAN_3P_PATH}/build"
cd "$BUILD_DIR"

# Clone the OCEAN repository if it doesn't exist
# TODO: switch to ocean main repo once --minimal is supported
if [ -d "./ocean" ]; then
  echo "OCEAN repository already exists."
else
  echo "Cloning OCEAN repository..."
  # git clone https://github.com/facebookresearch/ocean.git
  git clone --branch export-D72743344 https://github.com/YLouWashU/ocean.git
fi
cd ocean

# Build and install OCEAN according to platform
echo "Installing Ocean 3rd party libs into: ${OCEAN_3P_PATH}/install"
./build/cmake/build_thirdparty_linuxunix.sh -c release -l static -b "$OCEAN_3P_PATH/build" -i "${OCEAN_3P_PATH}/install"

echo "Installing Ocean lib into: ${INSTALL_PATH}"
./build/cmake/build_ocean_linuxunix.sh -c release -l static --minimal -b "${BUILD_DIR}" -i "${INSTALL_PATH}" -t "${OCEAN_3P_PATH}/install"

# Verify the installation
if [ -d "$INSTALL_PATH" ]; then
    echo "OCEAN installed successfully to $INSTALL_PATH"
else
    echo "OCEAN installation failed."
    exit 1
fi

# Clean up
echo "Cleaning up..."
cd "$HOME"
rm -rf "${OCEAN_3P_PATH}/build"
rm -rf "$BUILD_DIR"
echo "Ocean installed successfully to $INSTALL_PATH"
