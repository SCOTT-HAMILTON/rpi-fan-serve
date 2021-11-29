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
in with drogon1_7_2Pkgs; mkShell {
  buildInputs = [
    brotli
    c-ares
    clang
    cmake
    patchedDrogon 
    jsoncpp
    libuuid
    meson
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
      ./build/rpi-fan-serve -p 8888 -l test/rpi-fan/rpi-fan.log -j 4
    }
    run_dbus(){
      ./build/rpi-fan-serve --dbus
    }
    compile(){
      meson compile -C build
    }
    debug(){
      gdb ./build/rpi-fan-serve \
        -ex 'handle SIGINT pass' \
        -ex 'run -p 8888 -l test/rpi-fan/rpi-fan.log -j 4'
    }
    send_dbus(){
      dbus-send --session --dest=org.scotthamilton.RpiFanServe \
        --print-reply /org/scotthamilton/rpifanserver \
        org.freedesktop.DBus.Properties.Set \
          string:org.scotthamilton.RpiFanServe \
          string:CacheLifeExpectancy \
          variant:int64:$1
    }
  '';
}
