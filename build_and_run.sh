#!/bin/bash

# Reset terminal
reset

# Configure CMake
cmake -S . -B cmake_build

# Build and capture output, warnings, and errors
cmake --build cmake_build 2>&1 | tee build_output.log | tee >(grep -i "warning:" > warnings.txt) >(grep -i "error:" > errors.txt) > /dev/null

# Run the executable with the specified arguments
./cmake_build/AssetToYamlCPP -e /home/laptopanon/GIT/lighthouse/Lighthouse/bin/assets.bin ./cmake_build/assettest
