let
  withCustomDaemon = false;
  drogon1_7_2 = fetchTarball {
    url = "https://github.com/NixOS/NixPkgs/archive/cd0fa6156f486c583988d334202946ffa4b9ebe8.tar.gz";
    sha256 = "003vg8gz99spbmdvff06y36icn4by2yv4kb3s1m73q5z73bb2dy7";
  };
  drogon1_7_2Pkgs = import drogon1_7_2 { };
  nixpkgs = drogon1_7_2;
  lib = drogon1_7_2Pkgs.lib;
  shamilton = import (fetchTarball {
    url = "https://github.com/SCOTT-HAMILTON/nur-packages/archive/7d08863134c05cf237964819ac040badda8bf9ce.tar.gz";
    sha256 = "0rpyxn81v267ldsq8lvrgmb5z2ddpdcwrhis64h2zap08d1zy4r0";
  }) {};

  rpi-fan-serve = import ../.;

  # dbusVerboseOverlay = self: super: {
  #   dbus = super.dbus.overrideAttrs (old: {
  #     patches = (old.patches or []) ++ [
  #       /tmp/good.patch
  #     ];
  #     postPatch = (old.postPatch or "") + ''
  #       sed -i '/^\[Service\]$/a Environment="DBUS_VERBOSE=1"' bus/dbus.service.in
  #       sed -i '/^\[Service\]$/a Environment="DBUS_VERBOSE=1"' bus/systemd-user/dbus.service.in 
  #       cat bus/dbus.service.in
  #       cat bus/systemd-user/dbus.service.in
  #     '';
  #     configureFlags = (old.configureFlags or []) ++ [
  #       "--enable-debug=yes"
  #       "--enable-verbose-mode"
  #     ];
  #   });
  # };
  runRpiFanServe = drogon1_7_2Pkgs.writeScriptBin "run-rpi-fan-serve" ''
    #!${drogon1_7_2Pkgs.stdenv.shell}
    ${rpi-fan-serve}/bin/rpi-fan-serve -p 8888 -l \
      ${rpi-fan-serve}/share/tests/rpi-fan.log \
      -j 4 &
  '';

  runDbusSendCall = drogon1_7_2Pkgs.writeScriptBin "dbus-send-call" (''
    #!${drogon1_7_2Pkgs.stdenv.shell}
  '' + lib.optionalString withCustomDaemon ''
    export DBUS_SYSTEM_BUS_ADDRESS=$(cat /home/dbus-daemon.log \
      | grep 'unix:path=' \
      | sed -E 's!^(unix:path=.*),.*!\1!g')
    echo "DBUS_SYSTEM_BUS_ADDRESS is \`$DBUS_SYSTEM_BUS_ADDRESS\`"
  '' + ''
    dbus-send --system --dest=org.scotthamilton.RpiFanServe \
      --print-reply /org/scotthamilton/rpifanserver \
      org.freedesktop.DBus.Properties.Set \
        string:org.scotthamilton.RpiFanServe \
        string:CacheLifeExpectancy \
        variant:int64:7200 1>&2 || exit 0
  '');
  runDbusDaemon = drogon1_7_2Pkgs.writeScriptBin "run-dbus-daemon" ''
    #!${drogon1_7_2Pkgs.stdenv.shell}
    dbus-daemon --system --print-address --fork 2>&1 | tee /home/dbus-daemon.log
    command -v tee
    cat /home/dbus-daemon.log
    ps -faux | grep 'dbus-daemon .*print-address' \
        | grep -v "grep "
    PID=$(ps -faux | grep 'dbus-daemon .*print-address' \
        | grep -v "grep "\
        | sed -E 's=message\+ *([0-9]*).*=\1=g')
    echo "PID IS $PID"
    echo "$PID" > dbus-daemon.pid
  '';
  stopDbusDaemon = drogon1_7_2Pkgs.writeScriptBin "stop-dbus-daemon" ''
    #!${drogon1_7_2Pkgs.stdenv.shell}
    PID=$(cat dbus-daemon.pid)
    echo "KILLING PID $PID"
    kill $PID
    echo "DBUS-DAEMON LOGS"
    cat /home/dbus-daemon.log
    echo "----------------"
  '';

in
  import "${nixpkgs}/nixos/tests/make-test-python.nix" ({ pkgs, ...}: {
    system = "x86_64-linux";

    nodes.machine = { nodes, config, pkgs, ... }:
    {
      # nixpkgs.overlays = [ dbusVerboseOverlay ];
      users = {
        users = {
          alice = {
            isNormalUser = true;
            description = "Alice Foobar";
            password = "foobar";
            uid = 1000;
            extraGroups = [ "rpi-fan-serve" ];
          };
          bob = {
            isNormalUser = true;
            description = "Bob Foobar";
            password = "foobar";
            uid = 1001;
          };
        };
        groups.rpi-fan-serve = {};
      };

      environment.systemPackages = with pkgs; [
        systemd
        runRpiFanServe
        runDbusSendCall
        runDbusDaemon
        stopDbusDaemon
        killall
      ];
      services.dbus.packages = [ rpi-fan-serve ];

    };
    testScript = let
      runVerbose = cmd: "machine.succeed(\"${cmd} 1>&2\")";
      runAsAlice = cmd: runVerbose "su - alice -c '${cmd}'";
      runAsBob = cmd: runVerbose "su - bob -c '${cmd}'";
    in ''
      start_all()

      ${runVerbose "journalctl -xe"}
      ${runAsAlice "id"}
      ${runVerbose "run-rpi-fan-serve"}
    '' + lib.optionalString withCustomDaemon ''
      ${runVerbose "run-dbus-daemon"}
      ${runVerbose "sleep 5"}
    '' + ''
      ${runAsAlice "dbus-send-call"}
      ${runAsBob "dbus-send-call"}
      ${runVerbose "sleep 5"}
      ${runVerbose "killall rpi-fan-serve"}
    '' + lib.optionalString withCustomDaemon ''
      ${runVerbose "stop-dbus-daemon"}
    '';
})
