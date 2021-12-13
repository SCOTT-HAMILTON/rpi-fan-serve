#!/usr/bin/env python3

import sys
import subprocess
import os
import shutil

xml_file = sys.argv[1]
basename = os.path.basename(xml_file)
print(f"xml file = `{xml_file}`")
print(f"xml file basename = `{basename}`")

subprocess.check_call(
        [ "qdbusxml2cpp", "-p", "dbus_proxy", xml_file ]
)
subprocess.check_call(
        [ "qdbusxml2cpp", "-a", "dbus_adaptor", xml_file ]
)
shutil.move("dbus_proxy.cpp", "dbus/dbus_proxy.cpp")
shutil.move("dbus_proxy.h", "dbus/dbus_proxy.h")
shutil.move("dbus_adaptor.cpp", "dbus/dbus_adaptor.cpp")
shutil.move("dbus_adaptor.h", "dbus/dbus_adaptor.h")
