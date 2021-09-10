let
  pkgs = import (fetchTarball {
    url = "https://github.com/NixOS/NixPkgs/archive/cd0fa6156f486c583988d334202946ffa4b9ebe8.tar.gz";
    sha256 = "003vg8gz99spbmdvff06y36icn4by2yv4kb3s1m73q5z73bb2dy7";
  }) {};
in
with pkgs; callPackage (
{ lib
, stdenv
, fetchFromGitHub
, meson
, ninja
, cmake
, pkg-config
, c-ares
, drogon
, jsoncpp
, openssl
}:

stdenv.mkDerivation rec {
  pname = "rpi-fan-serve";
  version = "unstable";

  # src = fetchFromGitHub {
  #   owner = "p-ranav";
  #   repo = "argparse";
  #   rev = "v${version}";
  #   sha256 = "09wp0i835g6a80w67a0qa2mlsn81m4661adlccyrkj7rby5hnz3c";
  # };

  src = ./.;

  nativeBuildInputs = [ meson ninja cmake pkg-config ];
  buildInputs = [ drogon jsoncpp c-ares openssl ];

  meta = with lib; {
    description = "Argument Parser for Modern C++";
    license = licenses.mit;
    homepage = "https://github.com/p-ranav/argparse";
    maintainers = [ "Scott Hamilton <sgn.hamilton+nixpkgs@protonmail.com>" ];
    platforms = platforms.linux;
  };
}) {}
