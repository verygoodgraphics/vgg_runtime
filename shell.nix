# NOTE It is for linux deployment build only. Use it with `nix-shell --pure`
# NOTE if you want to use vulkan driver, you must be in the "video" user group
{ pkgs ? import (fetchTarball "https://s5.vgg.cool/nixos-23.05.tar.gz") {}
, nixgl ? import (fetchTarball "https://github.com/guibou/nixGL/archive/master.tar.gz") {}
, nvidiaGpu ? false
}:
with pkgs;
  mkShell {
    buildInputs = [
      # build tools
      clang
      cmake
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
  }
