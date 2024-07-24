# VGG Runtime

VGG Runtime is an implementation of [VGG Specs](https://docs.verygoodgraphics.com/specs/overview) with cross-platform vector graphics rendering and user interaction capabilities. It accepts any file conforming to the VGG Specs as input.

>  NOTE A `.daruma` file is one such type of input that you can convert from other design files using the [VGG Command-line tool](https://github.com/verygoodgraphics/vgg_cli).

## Cross Platform Support

| Platform\\Arch | X86  | ARM  | RISC-V | WASM |
| -------------- | ---- | ---- | ------ | ---- |
| Linux          | ✅    | ✅    | ✅      | N/A  |
| Android        | ❌    | ⛏️    | ⭕️      | N/A  |
| Harmony        | ❌    | ⛏️    | ⭕️      | N/A  |
| iOS            | N/A  | ✅    | N/A    | N/A  |
| macOS          | ✅    | ✅    | N/A    | N/A  |
| Windows        | ✅    | ❌    | N/A    | N/A  |
| WASM           | N/A  | N/A  | N/A    | ✅    |

✅ Supported ⛏️ Working in process ⭕️ Not supported (but planned) ❌ Not supported (no official plan)

### Tested embedded devices on Linux

- __ARM__: Macbook M2, Raspberry Pi 4B
- __RISC-V__: Lichee Pi 4A

## How To Build

This project can be built straightforwardly with CMake using common practices, though the dependencies might be a bit complicated.

### 1. Build Requirements

- C++ compiler supports C++20 or higher
- CMake >= 3.19
- [Ninja](https://ninja-build.org/) and Python3 is required for building Skia and Nodejs
- [Netwide Assembler (NASM)](https://nasm.us/) for building Nodejs under Windows

> NOTE For the Python 3.12, you have to `pip install setuptools` when building Nodejs

### 2. Dependent Libraries

- Use `git submodule update --init --recursive` to fetch VGG submodules.
- Libraries that will be automatically downloaded and built
  - Nodejs
  - Skia
  - (windows only) SDL2
- System/user-provided libraries
  - (except for windows) SDL >= 2.26
  - (optional) Vulkan SDK with SPIR-V tools

#### Optional: Specify Skia manually

You can also use your own Skia by specifying `SKIA_DIR`:

```bash
cmake .. -DSKIA_DIR=/path/to/your/skia
```

Skia can be downloaded from the [official website](https://skia.org/docs/user/download/). We use our Skia fork [vgg/m116](https://github.com/verygoodgraphics/skia/tree/vgg/m116) branch for building, which has some modifications and fixes for our scenario. We don't assure other versions could be successfully compiled using our CMake script.

### 3. Build Examples

#### Linux/macOS building example

```bash
mkdir build
cd build
cmake ..
cmake --build . --parallel
```

> Note: For release build, please add `-DCMAKE_BUILD_TYPE=Release` to the first `cmake` command.

#### Windows building example

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug -G "Visual Studio 17 2022"
cmake --build . --parallel --config Debug -t sdl_runtime
```

> Note: For release build, just replace `Debug` with `Release`.

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

Build & install `vgg_container` libraries for [vgg_ios](https://github.com/verygoodgraphics/vgg_ios).

```bash
mkdir build.ios
cd build.ios
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/vgg.ios.toolchain.cmake -DVGG_VAR_TARGET="iOS"
# cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../cmake/vgg.ios.toolchain.cmake -DVGG_VAR_TARGET="iOS-simulator"
cmake --build . --parallel -t vgg_container
cmake --install . --prefix <path/to/vgg_ios/VggRuntime/external>
```

#### Qt building example

Build & install `vgg_container` libraries for [vgg_qt](https://github.com/verygoodgraphics/vgg_qt).

```bash
mkdir build.qt
cd build.qt
cmake .. -DVGG_CONTAINER_FOR_QT=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel -t vgg_container --config Release
cmake --install . --component container --config Release --prefix <path/to/vgg_qt/VggContainer/external>
```

> Note: For debug build, please replace `Release` with `Debug` to the `cmake` commands.

### 4. Unit test

#### Linux/macOS unit test

```bash
cd build
cmake .. -DENABLE_UNIT_TEST=ON
cmake --build . --parallel -t unit_tests
ctest
```

## How To Run

Make sure you have built the `sdl_runtime` target. Then in the build directory, run it with

```bash
./sdl_runtime /path/to/your/file.daruma
```

where `file.daruma` is a file conforming to [VGG Specs](https://docs.verygoodgraphics.com/specs/overview), which can be generated using [VGG Command-line tool](https://github.com/verygoodgraphics/vgg_cli). An example could be downloaded [here](https://verygoodgraphics.com/vgg.daruma).

### Running with custom font configuration

VGG Runtime uses fonts in system directories by default, but you can assign extra font folders in a configuration file as follows

```bash
./sdl_runtime /path/to/your/file.daruma -c /path/to/your/config.json
```

where an example of `config.json` is provided in [asset/etc/config.json](https://github.com/verygoodgraphics/vgg_runtime/blob/main/asset/etc/config.json).

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=verygoodgraphics/vgg_runtime&type=Date)](https://star-history.com/#verygoodgraphics/vgg_runtime&Date)

## LICENSE

VGG Runtime is licensed under [VGG License](./LICENSE), which includes a royalty fee under certain conditions.

You can find a simplified explanation in VGG [FAQ](https://docs.verygoodgraphics.com/start/faq) if you don't want to read the tedious license.

And you are welcome to contribute to this project under VGG's [Contributor Reward Program](https://docs.verygoodgraphics.com/community/contributor-reward-program).
