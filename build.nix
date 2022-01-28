#!/usr/bin/env nix-build

{ nixpkgs ? (import <nixpkgs> {}) }: with nixpkgs;
stdenv.mkDerivation rec {
  pname = "vgg";
  version = "0.1";
  src = ./.;
  nativeBuildInputs = [ cmake ];
  buildInputs = [ freetype libGL SDL2 ];
}
