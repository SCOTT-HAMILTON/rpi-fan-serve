project('rpi-fan-serve', 'cpp', default_options : ['cpp_std=c++17'])

cares = dependency('libcares', required: true)
openssl = dependency('openssl', required: true)
sqlite3 = dependency('sqlite3', required: true)
libbrotlidec = dependency('libbrotlidec', required: true)
libbrotlienc = dependency('libbrotlienc', required: true)
uuid = dependency('uuid', required: true)
libdl = meson.get_compiler('cpp').find_library('dl', required: true)
zlib = meson.get_compiler('cpp').find_library('z', required: true)
cppzmq = dependency('cppzmq', required: true)
tbb = dependency('tbb', required: true)
libconfigpp = dependency('libconfig++', required: true)
drogon = dependency('Drogon', method: 'cmake',
  modules: ['Drogon::Drogon'],required: true, static: false)
libstdcppfs = meson.get_compiler('cpp').find_library('stdc++fs', required: true)

argparse = dependency('argparse', required: get_option('USE_SYSTEM_ARGPARSE'))
if not argparse.found()
  cmake = import('cmake')
  argparse_proj = cmake.subproject('argparse')
  argparse = argparse_proj.dependency('argparse')
endif

qt5 = import('qt5')
qt5_dep = dependency('qt5',
  modules: [ 'Core', 'DBus' ], required: true)

subdir('tools')

source_files = [
  'main.cc',
  'TempsCtrl.cc',
]

executable('rpi-fan-serve', source_files,
  dependencies : [
	zlib,
	libdl,
	uuid,
	libbrotlienc,
	libbrotlidec,
	sqlite3,
	openssl,
	cares,
	drogon,
	argparse,
	cppzmq,
	tbb,
	libconfigpp,
	libstdcppfs
  ],
  install : true)

datadir = get_option('datadir')
subdir('dbus')
subdir('test')
