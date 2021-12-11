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
