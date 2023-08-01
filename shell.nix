# NOTE It is for linux deployment build only. Use it with `nix-shell --pure`
{pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/nixos-23.05.tar.gz") {}}:
with pkgs;
  mkShell {
    nativeBuildInputs = [clang cmake ninja python3];
    buildInputs = [libGL SDL2];
  }
