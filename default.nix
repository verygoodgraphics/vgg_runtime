{ nixpkgs ? (import <nixpkgs> {}) }: with nixpkgs;
stdenv.mkDerivation rec {
  pname = "vgg";
  version = "0.1";
  src = ./.;
  nativeBuildInputs = [ cmake ];
  buildInputs = [ libGL SDL2 ];
}
