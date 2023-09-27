# VGG Runtime

VGG Runtime is a design-as-code engine for rendering and running [VGG Daruma](https://verygoodgraphics.com/daruma) files.

## Build Instructions

This project can be built with CMake using common practice.

### 1. Build Requirements

- C++ compiler supports C++17 or higher
- Make, CMake and Ninja
- Python3

### 2. Dependent Libraries

- Use `git submodule update --init` to fetch VGG submodules.
- `Nodejs` and `Skia` would be automatically prepared during building.
- System/user-provided installation of the following libraries
  - Vulkan SDK (with SPIR-V tools)
  - SDL >= 2.26

#### Specify Skia manually

You can also use your own skia by specifying `SKIA_DIR`:

```bash
cmake .. -DSKIA_DIR=/path/to/your/skia
```

You could download Skia from the [official website](https://skia.org/docs/user/download/). We use our Skia fork [vgg/m116](https://github.com/verygoodgraphics/skia/tree/vgg/m116) branch for building, which has some modifications and fixes for our scenario. We don't assure other versions could be successfully compiled using our CMake script.

### 3. Build Examples

#### Linux/macOS building example

```bash
mkdir build
cd build
cmake ..
cmake --build . --parallel
```

> Note: For release build, please add `-DCMAKE_BUILD_TYPE=Release` to the first `cmake` command.

#### WebAssembly building example

[Emscripten SDK](https://github.com/emscripten-core/emscripten) is required to build WebAssembly version. You should [install and activate](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended) at least one version of emsdk before proceeding.

```bash
mkdir build.wasm
cd build.wasm
source /path/to/emsdk/emsdk_env.sh
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

#### Windows building example

Compiling on Windows need extra efforts. It will be released once being stable.

### 4. Unit test

#### Linux/macOS unit test
```bash
cd build
cmake .. -DENABLE_UNIT_TEST=ON
cmake --build . --parallel -t unit_tests
ctest
```

## LICENSE

VGG Runtime is licensed under [AGPL](./LICENSE).
