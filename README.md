# asset_to_yaml

## Overview
`asset_to_yaml` is a project primarily written in C++ designed to convert banjo kazooie assets into torch compatible YAML format. This repository includes necessary files and scripts to facilitate this conversion.

## Features
- High performance asset processing
- Easy conversion to YAML format
- Customizable via CMake

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
./asset_to_yaml -e <asset.bin> <output_yaml>
```

## Contributing
Contributions are welcome! Please fork the repository and submit your pull requests.

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact
For any questions or issues, please open an issue on this repository or contact `snowboundmage2`.

```
