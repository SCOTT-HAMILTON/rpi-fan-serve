let
  pkgs = import (fetchTarball {
    url = "https://github.com/NixOS/NixPkgs/archive/cd0fa6156f486c583988d334202946ffa4b9ebe8.tar.gz";
    sha256 = "003vg8gz99spbmdvff06y36icn4by2yv4kb3s1m73q5z73bb2dy7";
  }) {};
in with pkgs; mkShell {
  buildInputs = [
    brotli
    c-ares
    clang
    cmake
    drogon
    jsoncpp
    libuuid
    meson
    openssl
    pkg-config
    sqlite
  ];
  shellHook = ''
    export CXX=clang++
  '';
}
