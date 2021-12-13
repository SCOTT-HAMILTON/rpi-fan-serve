let
  drogon1_7_2Pkgs = import (fetchTarball {
    url = "https://github.com/NixOS/NixPkgs/archive/cd0fa6156f486c583988d334202946ffa4b9ebe8.tar.gz";
    sha256 = "003vg8gz99spbmdvff06y36icn4by2yv4kb3s1m73q5z73bb2dy7";
  }) {};
  localShamilton = import ~/GIT/nur-packages { };
  patchedDrogon = with drogon1_7_2Pkgs; drogon.overrideAttrs (old: {
    patches = (old.patches or []) ++ [
      (fetchpatch {
        url = "https://github.com/drogonframework/drogon/pull/1094/commits/52c4dcc1bda865a924a112249fd845ac5ea9c9a7.patch";
        sha256 = "09rbh31lwmkv8pjysvd11vz9qnrmga7iw9jn3f9i39q0y1yvrfw6";
      })
    ];
  });
  patchedMeson = with drogon1_7_2Pkgs; meson.overrideAttrs (old: rec {
    pname = "patched-meson";
    version = "0.58.1";
    name = "${pname}-${version}";
    src = python3Packages.fetchPypi {
      inherit (old) pname;
      inherit version;
      sha256 = "0padn0ykwz8azqiwkhi8p97bl742y8lsjbv0wpqpkkrgcvda6i1i";
    };
  });
in with drogon1_7_2Pkgs; mkShell {
  buildInputs = [
    brotli
    c-ares
    clang
    cmake
    patchedDrogon 
    patchedMeson
    jsoncpp
    libuuid
    openssl
    pkg-config
    sqlite
    localShamilton.argparse
    localShamilton.sdbusplus
    localShamilton.sdbusplus-tools
    cppzmq
    zeromq
    tbb
    systemd
    libconfig
    libsForQt5.qtbase
  ];
  shellHook = ''
    export CXX=clang++
    update_dbus_files(){
      sdbus++ interface server-cpp org.scotthamilton.RpiFanServe > org/scotthamilton/RpiFanServe/server.cpp
      sdbus++ interface server-header org.scotthamilton.RpiFanServe > org/scotthamilton/RpiFanServe/server.hpp
      sdbus++ error exception-cpp org.scotthamilton.RpiFanServe > org/scotthamilton/RpiFanServe/error.cpp
      sdbus++ error exception-header org.scotthamilton.RpiFanServe > org/scotthamilton/RpiFanServe/error.hpp
    }
    run(){
      sudo ./build/rpi-fan-serve -p 8888 -l test/rpi-fan/rpi-fan.log -j 4
    }
    run_dbus(){
      sudo ./build/dbus/rpi-fan-serve-dbus
    }
    echo_meson() {
      echo ${patchedMeson}/bin/meson
    }
    compile(){
      meson compile -C build
    }
    debug(){
      sudo gdb ./build/rpi-fan-serve \
        -ex 'handle SIGINT pass' \
        -ex 'run -p 8888 -l test/rpi-fan/rpi-fan.log -j 4'
    }
    debug_dbus(){
      sudo gdb ./build/dbus/rpi-fan-serve-dbus \
        -ex 'handle SIGINT pass' \
        -ex 'run'
    }
    send_dbus(){
      sudo busctl set-property org.scotthamilton.RpiFanServe /org/scotthamilton/rpifanserver org.scotthamilton.RpiFanServe CacheLifeExpectancy x $1
    }
    send_dbus2(){
      sudo dbus-send --system --dest=org.scotthamilton.RpiFanServe \
        --print-reply /org/scotthamilton/rpifanserver \
        org.freedesktop.DBus.Properties.Set \
          string:org.scotthamilton.RpiFanServe \
          string:CacheLifeExpectancy \
          variant:int64:$1
    }
    introspect() {
      dbus-send --system --dest=org.scotthamilton.RpiFanServe \
        --print-reply /org/scotthamilton/rpifanserver \
        org.freedesktop.DBus.Introspectable.Introspect
    }
    dbus_monitor() {
      sudo busctl monitor org.scotthamilton.RpiFanServe \
        --full --verbose
    }
  '';
}
