# asset_to_yaml

## Overview
`asset_to_yaml` is a project primarily written in C++ designed to convert banjo kazooie assets, with meta info into YAML format. This repository includes necessary files and scripts to facilitate this conversion.

## Features
- High performance asset processing
- Easy conversion to YAML format
- Cmake for easy compilation cross platform (no options yet, but easy enough to add i hope)

## Requirements
- C++ compiler
- CMake

## Installation
1. Clone the repository:
    ```bash
    git clone --recursive https://github.com/snowboundmage2/asset_to_yaml.git
    cd asset_to_yaml
    ```

2. Build the project using CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## Usage
To use the tool, run the following command:
```bash
./asset_to_yaml -e <asset.bin> <output_folder>
```

## Contributing
Contributions are welcome! Please fork the repository and submit your pull requests.

## Contact
For any questions or issues, please open an issue on this repository, or @ me on the you know where.
