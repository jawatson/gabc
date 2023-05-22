gabc_sources = [
  'main.c',
  'gabc-application.c',
  'gabc-window.c',
  'gabc-log-window.c',
  'gabc-prefs-window.c'
]


gabc_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.2'),
  dependency('gtksourceview-5', version: '>= 5.8'),
]

gabc_sources += gnome.compile_resources('gabc-resources',
  'gabc.gresource.xml',
  c_name: 'gabc'
)


settings_schemas = [ 'me.pm.m0dns.gabc.gschema.xml' ]

gabc_schemas = gnome.compile_schemas(depend_files: files(settings_schemas))

schemas_dir = get_option('prefix') / get_option('datadir') / 'glib-2.0' / 'schemas'

install_data(settings_schemas, install_dir: schemas_dir)

meson.add_install_script('glib-compile-schemas', schemas_dir)


executable('gabc', gabc_sources, gabc_schemas,
  dependencies: gabc_deps,
       install: true,
)
