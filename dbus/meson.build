qt5 = import('qt5')
qt5_dep = dependency('qt5',
  modules: [ 'Core', 'DBus' ], required: true)

thread_dep = dependency('threads')

dependencies = [ qt5_dep, libstdcppfs, thread_dep, tbb, cppzmq ]

moc_files = qt5.preprocess(moc_headers : 
	   [ 'dbus_adaptor.h', 
		 'dbus_proxy.h',
		 'RpiFanServeObject.h',
		 'zmq_pair_qthread.hpp',
		 'zmq_dbus_client.hpp',
		 'dbus_server.h',
		 'SignalHandler.h'
	   ],
	   dependencies: dependencies,
)

sources = [
  'dbus_adaptor.cpp',
  'dbus_proxy.cpp',
  'RpiFanServeObject.cpp',
  'dbus_server.cpp',
  'SignalHandler.cpp',
  'main.cpp',
]

executable('rpi-fan-serve-dbus', 
	  sources,
	  moc_files,
	  install: true,
	  dependencies: dependencies,
)

conf_data = configuration_data()
conf_data.set('bindir', get_option('prefix') / get_option('bindir'))
desktop_file = configure_file(
	  input : 'org.scotthamilton.RpiFanServe.service.in',
	  output : 'org.scotthamilton.RpiFanServe.service',
	  configuration : conf_data,
	  install: true,
	  install_dir: join_paths(datadir, 'dbus-1', 'system-services'))

install_data('org.scotthamilton.RpiFanServe.conf',
	install_dir: join_paths(datadir, 'dbus-1', 'system.d'))
