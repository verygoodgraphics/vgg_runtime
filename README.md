# VGG Runtime

A design engine capable of loading design drafts as well as running design as an emulated app.

## Features

- Game-engine-like ECS architecture targeting high-performance interactive applications
- First-class support for Sketch to load and render Sketch designs
- Cross-platform support for running on Linux, macOS and in Browsers
- WebAssembly support with emscripten compatibility
- Built-in scripting ability for programming upon design drafts

## How to build

This project can be built with CMake using common practice. All dependency is batteries-included (except for SDL) so no extra submodules are needed.

### Linux-version building example

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j8
```

### WebAssembly-version building example

[Emscripten SDK](https://github.com/emscripten-core/emscripten) is required to build WebAssembly version. You should at least [install and activate](https://emscripten.org/docs/getting_started/downloads.html#installation-instructions-using-the-emsdk-recommended) one version of emsdk before proceeding.

```bash
mkdir build.wasm
cd build.wasm
source /path/to/emsdk/emsdk_env.sh
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j8
```

## How to use as standalone app

The design draft could be created or imported from sketch by our [VGG editor](https://verygoodgraphics.com/). And runtime is capable of loading Sketch or VGG file, and running VGG file if scripts are programmed in.

```bash
./runtime -l /path/to/sketch_or_vgg_file
```

Note that you should put fonts under the same directory for runtime to use.

### Built-in basic shortcuts

- Ctrl+Q to exit

- Ctrl+MouseDrag to pan

- Ctrl+MouseWheel to zoom

## How to embed into your own app

Currently supported embedding environments:

- [React](https://github.com/verygoodgraphics/vgg_react)

## Sketch compatibility

TODO

## LICENSE

VGG Runtime is licensed under [AGPL](./LICENSE). Those who need a commercial version can visit the official [VGG website](https://verygoodgraphics.com/).
