# VGG Runtime

VGG Runtime is an implementation of [VGG Specs](https://docs.verygoodgraphics.com/specs/overview) with cross-platform rendering and scripting capabilities.

VGG Runtime only supports reading and rendering of `.daruma` files, which
- you can use [VGG Daruma](https://verygoodgraphics.com/daruma) to convert from other design files online,
- or you can use [VGG Sketch Parser](https://github.com/verygoodgraphics/vgg_sketch_parser) to convert from Sketch file locally.

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

#### Optinal: Specify Skia manually

You can also use your own skia by specifying `SKIA_DIR`:

```bash
cmake .. -DSKIA_DIR=/path/to/your/skia
```

Skia can be download from the [official website](https://skia.org/docs/user/download/). We use our Skia fork [vgg/m116](https://github.com/verygoodgraphics/skia/tree/vgg/m116) branch for building, which has some modifications and fixes for our scenario. We don't assure other versions could be successfully compiled using our CMake script.

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

#### iOS building example
Build & install vgg_container libraries for [vgg_ios](https://github.com/verygoodgraphics/vgg_ios).

```bash
mkdir build.ios
cd build.ios
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/vgg.ios.toolchain.cmake -DVGG_VAR_TARGET="iOS"
# cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/vgg.ios.toolchain.cmake -DVGG_VAR_TARGET="iOS-simulator"
cmake --build . --parallel -t vgg_container
cmake --install . --prefix <path/to/vgg_ios/VggRuntime/external>
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

## How to run

Make sure you have built `sdl_runtime` target. Then in the build directory, running it with

```bash
./sdl_runtime /path/to/your/file.daruma
```

where `file.daruma` is the file conforming to [VGG Specs](https://docs.verygoodgraphics.com/specs/overview), which can be generated using [VGG Daruma](https://verygoodgraphics.com/daruma) or [VGG Sketch Parser](https://github.com/verygoodgraphics/vgg_sketch_parser).

### Running with custom font configuration

VGG Runtime uses fonts in system directories by default. But you can assign extra font folders in a configuration file as follows

```bash
./sdl_runtime /path/to/your/file.daruma -c config.json
```

where an example of `config.json` is provided in [asset/etc/config.json](https://github.com/verygoodgraphics/vgg_runtime/blob/feature-readme/asset/etc/config.json).

## LICENSE

VGG Runtime is licensed under [VGG License](./LICENSE).
