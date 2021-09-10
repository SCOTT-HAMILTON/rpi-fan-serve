project('rpi-fan-serve', 'cpp', default_options : ['cpp_std=c++17'])

cares = dependency('libcares', required: true)
openssl = dependency('openssl', required: true)
sqlite3 = dependency('sqlite3', required: true)
libbrotlidec = dependency('libbrotlidec', required: true)
libbrotlienc = dependency('libbrotlienc', required: true)
uuid = dependency('uuid', required: true)
libdl = meson.get_compiler('cpp').find_library('dl', required: true)
zlib = meson.get_compiler('cpp').find_library('z', required: true)
drogon = dependency('Drogon', method: 'cmake',
  modules: ['Drogon::Drogon'],required: true, static: false)

argparse = dependency('argparse', required: get_option('USE_SYSTEM_ARGPARSE'))
if not argparse.found()
  cmake = import('cmake')
  argparse_proj = cmake.subproject('argparse')
  argparse = argparse_proj.dependency('argparse')
endif

executable('rpi-fan-serve', 'main.cc', 'TempsCtrl.cc',
  dependencies : [ zlib, libdl, uuid, libbrotlienc, libbrotlidec, sqlite3, openssl, cares, drogon, argparse ],
  install : true)

# install_data('docs/FakeMicWavPlayer.1',
#   install_dir: get_option('datadir')+'/man/man1')
