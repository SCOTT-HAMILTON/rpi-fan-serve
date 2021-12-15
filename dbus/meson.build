thread_dep = dependency('threads')

dependencies = [ qt5_dep, libstdcppfs, thread_dep, tbb, cppzmq ]

dbus_gen_py = find_program('dbus-gen.py')
qdbus_files = custom_target('qdbus_files',
  input : [ 'org.scotthamilton.RpiFanServe.xml' ],
  output : [
	'dbus_proxy.h',
	'dbus_adaptor.h',
	'dbus_adaptor.cpp',
	'dbus_proxy.cpp',
  ],
  command : [
	dbus_gen_py,
	'@INPUT@',
])
generated_qdbus_headers_deps = []
generated_qdbus_sources_deps = []
foreach p : qdbus_files.to_list()
  if p.full_path().endswith('.h')
	generated_qdbus_headers_deps += p
  else
	generated_qdbus_sources_deps += p
  endif
endforeach

moc_files = qt5.preprocess(moc_headers : 
	   generated_qdbus_headers_deps + [
		 'RpiFanServeObject.h',
		 'zmq_pair_qthread.hpp',
		 'zmq_dbus_client.hpp',
		 'dbus_server.h',
		 'SignalHandler.h',
	   ],
	   dependencies: dependencies,
)
sources = [
  'RpiFanServeObject.cpp',
  'dbus_server.cpp',
  'SignalHandler.cpp',
  'main.cpp',
]

executable('rpi-fan-serve-dbus', 
	  [generated_qdbus_sources_deps] + sources,
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
