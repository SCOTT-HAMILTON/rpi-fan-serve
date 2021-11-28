project('rpi-fan-serve', 'cpp', default_options : ['cpp_std=c++17'])

cares = dependency('libcares', required: true)
openssl = dependency('openssl', required: true)
sqlite3 = dependency('sqlite3', required: true)
libbrotlidec = dependency('libbrotlidec', required: true)
libbrotlienc = dependency('libbrotlienc', required: true)
uuid = dependency('uuid', required: true)
libdl = meson.get_compiler('cpp').find_library('dl', required: true)
zlib = meson.get_compiler('cpp').find_library('z', required: true)
sdbusplus = dependency('sdbusplus', required: true)
cppzmq = dependency('cppzmq', required: true)
tbb = dependency('tbb', required: true)
drogon = dependency('Drogon', method: 'cmake',
  modules: ['Drogon::Drogon'],required: true, static: false)

argparse = dependency('argparse', required: get_option('USE_SYSTEM_ARGPARSE'))
if not argparse.found()
  cmake = import('cmake')
  argparse_proj = cmake.subproject('argparse')
  argparse = argparse_proj.dependency('argparse')
endif

source_files = [ 'main.cc', 'TempsCtrl.cc' ]
dbus_source_files = [
  'org/scotthamilton/RpiFanServe/error.cpp',
  'org/scotthamilton/RpiFanServe/server.cpp'
]

executable('rpi-fan-serve', dbus_source_files + source_files,
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
	sdbusplus,
	cppzmq,
	tbb
  ],
  install : true)

datadir = get_option('datadir')
conf_data = configuration_data()
conf_data.set('bindir', get_option('prefix') / get_option('bindir'))
desktop_file = configure_file(
	  input : 'dbus/org.scotthamilton.rpifanserve.service.in',
	  output : 'org.scotthamilton.rpifanserve.service',
	  configuration : conf_data,
	  install: true,
	  install_dir: join_paths(datadir, 'dbus-1', 'services'))
