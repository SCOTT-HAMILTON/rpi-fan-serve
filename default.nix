let
  drogon1_7_2Pkgs = import (fetchTarball {
    url = "https://github.com/NixOS/NixPkgs/archive/cd0fa6156f486c583988d334202946ffa4b9ebe8.tar.gz";
    sha256 = "003vg8gz99spbmdvff06y36icn4by2yv4kb3s1m73q5z73bb2dy7";
  }) {};
  shamilton = import (fetchTarball {
    url = "https://github.com/SCOTT-HAMILTON/nur-packages/archive/7d08863134c05cf237964819ac040badda8bf9ce.tar.gz";
    sha256 = "0rpyxn81v267ldsq8lvrgmb5z2ddpdcwrhis64h2zap08d1zy4r0";
  }) {};
  patchedDrogon = with drogon1_7_2Pkgs; drogon.overrideAttrs (old: {
    patches = (old.patches or []) ++ [
      (fetchpatch {
        url = "https://github.com/drogonframework/drogon/pull/1094/commits/52c4dcc1bda865a924a112249fd845ac5ea9c9a7.patch";
        sha256 = "09rbh31lwmkv8pjysvd11vz9qnrmga7iw9jn3f9i39q0y1yvrfw6";
      })
    ];
  });
in
with drogon1_7_2Pkgs; callPackage (
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
, sdbusplus
, systemd
, nix-gitignore
}:

stdenv.mkDerivation rec {
  pname = "rpi-fan-serve";
  version = "unstable";

  src = nix-gitignore.gitignoreSource [] ./.;

  nativeBuildInputs = [ meson ninja cmake pkg-config ];
  buildInputs = [ drogon jsoncpp c-ares openssl sdbusplus systemd ];
}) {
  inherit (shamilton) sdbusplus;
  drogon = patchedDrogon;
}
