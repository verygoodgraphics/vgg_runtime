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
- We need user-provided Skia source code or precompiled binaries.
- Nodejs would be automatically downloaded and compiled.

You could download Skia from the [official website](https://skia.org/docs/user/download/). We recommend [chrome/m116](https://github.com/google/skia/tree/chrome/m116) branch for building. Since Skia's API is not stable between releases, we don't assure other versions could be successfully compiled using our CMake script.

```bash
# fetch skia
git clone https://skia.googlesource.com/skia.git
# then fetch skia's own dependencies
cd skia
python3 tools/git-sync-deps
```

## Build Examples

### Linux/macOS building example

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSKIA_EXTERNAL_PROJECT_DIR=/path/to/your/skia
make -j8
```

### WebAssembly building example

[Emscripten SDK](https://github.com/emscripten-core/emscripten) is required to build WebAssembly version. You should at least [install and activate](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended) one version of emsdk before proceeding.

```bash
mkdir build.wasm
cd build.wasm
source /path/to/emsdk/emsdk_env.sh
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DSKIA_EXTERNAL_PROJECT_DIR=/path/to/your/skia
emmake make -j8
```

### Windows building example

Compiled on windows need some extra efforts. It will be released once being stable.

## LICENSE

VGG Runtime is licensed under [AGPL](./LICENSE). Those who need a commercial version can visit the official [VGG website](https://verygoodgraphics.com/).
