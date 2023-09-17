# NOTE It is for linux deployment build only. Use it with `nix-shell --pure`
# NOTE if you failed to run vulkan, please check you are in the "video" user group
{ pkgs ? import (fetchTarball "https://github.com/verygoodgraphics/nixpkgs/archive/refs/tags/unstable-20230917.tar.gz") {}
, nixgl ? import (fetchTarball "https://github.com/verygoodgraphics/nixGL/archive/master.tar.gz") {}
, nvidiaGpu ? false
}:
with pkgs;
  mkShell {
    buildInputs = [
      # build tools
      clang_15
      cmake
      cmakeCurses
      ninja
      python3
      glslang # or shaderc

      # libs
      SDL2
      vulkan-headers
      vulkan-loader
      nixgl.nixGLIntel
      nixgl.nixVulkanIntel

      # other tools
      vim
      less
      which
      vulkan-tools # for vulkaninfo
      mesa-demos   # for eglinfo
    ]
    ++ lib.optionals nvidiaGpu [
      nixgl.auto.nixGLDefault
      nixgl.auto.nixVulkanNvidia
    ]
    ;

    shellHook = ''
      export CC=${clang_15}/bin/clang
      export CXX=${clang_15}/bin/clang++
    '';
  }
