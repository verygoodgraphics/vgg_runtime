# VGG Runtime

VGG Runtime is a design-as-code engine for rendering and running [VGG Daruma](https://verygoodgraphics.com/daruma) files.

## Build Instructions

This project can be built with CMake using common practice, however, third-party libraries should be prepared first in order to compile it.

### 1. Build Requirements

- C++ compiler supports C++17 or higher
- Make, CMake and Ninja
- Python3

### 2. Third-party Libraries

- Fetch `vgg_contrib` submodule by running `git submodule update --init`
- We use system/user-provided SDL installation
- ```Nodejs``` and ```Skia``` would be automatically downloaded and compiled.


### 3. Build Examples

#### Linux/macOS building example

```bash
mkdir build
cd build
cmake ..
cmake --build . --parallel
```

> Note: For release build, please add `-DCMAKE_BUILD_TYPE=Release` to the first `cmake` command. Currently in linux environment, VGG runtime can be built on Arch but not on Ubuntu.

#### WebAssembly building example

[Emscripten SDK](https://github.com/emscripten-core/emscripten) is required to build WebAssembly version. You should [install and activate](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended) at least one version of emsdk before proceeding.

```bash
mkdir build.wasm
cd build.wasm
source /path/to/emsdk/emsdk_env.sh
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

> Note: We build release version by defalut as it's hard to debug in WebAssembly environment. Currently, the WebAssembly version can only be built on macOS.

#### Windows building example

Compiled on windows need some extra efforts. It will be released once being stable.

#### Specify skia manually

You can also override skia by specifying SKIA_DIR:

```bash
...

cmake .. -DSKIA_DIR=/path/to/your/skia

...

```

You could download Skia from the [official website](https://skia.org/docs/user/download/). We use our skia fork[vgg/m116](https://github.com/verygoodgraphics/skia/tree/vgg/m116) branch for building, which has some modifications and fixes for our scenario. We don't assure other versions could be successfully compiled using our CMake script.

### 4. Unit test

#### Linux/macOS unit test
```bash
cd build
cmake .. -DENABLE_UNIT_TEST=ON
cmake --build . --parallel -t unit_tests
ctest
```

## LICENSE

VGG Runtime is licensed under [AGPL](./LICENSE). Those who need a commercial version can visit the official [VGG website](https://verygoodgraphics.com/).
