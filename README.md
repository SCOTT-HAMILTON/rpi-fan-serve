<p align="center">
      <a href="https://scott-hamilton.mit-license.org/"><img alt="MIT License" src="https://img.shields.io/badge/License-MIT-525252.svg?labelColor=292929&logo=creative%20commons&style=for-the-badge" /></a>
</p>
<h1 align="center">rpi-fan-serve - A web service to access rpi-fan data</h1>

## Description
rpi-fan-serve provides a simple RESTful API to access temperatures
data logged by the rpi-fan service.

## Cli Usage
```shell_session
rpi-fan-serve <port> <log_file>
```

The port argument is the port where rpi-fan-serve will listen on.

The log_file argument is the base log file that it will analyze and serve.

Any rotated log file (prefixed with .1, .2, etc) located in the same directory
as the base log file will also be analyzed and served.

Example: 

base_file: /var/log/rpi-fan/rpi-fan.log

rotated1: /var/log/rpi-fan/rpi-fan.log.1

rotated2: /var/log/rpi-fan/rpi-fan.log.2

...

## Client-Side Usage

`/temps?dayOffset=0`: ask for today's temperature data.

Example: 
```shell_session
curl '127.0.0.1:8085/temps?dayOffset=0'
```

`/temps?dayOffset=1`: ask for yesterday's temperature data.

Example: 
```shell_session
curl '127.0.0.1:8085/temps?dayOffset=1'
```

## Building
This project is configured with meson.
It depends on:
 - drogon
 - argparse (if not provided, bundled lib will be used)
 - cares
 - openssl
 - sqlite3
 - libbrotlidec
 - libbrotlienc
 - uuid
 - libdl
 - zlib

```shell_session
meson setup build .
meson compile -C build -j4
```

## License
rpi-fan-serve is delivered as it is under the well known MIT License.

**References that helped**
 - [drogon documentation] : <https://drogon.docsforge.com/master/overview/>

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [drogon documentation]: <https://drogon.docsforge.com/master/overview/>
