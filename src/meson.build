gabc_sources = [
  'main.c',
  'gabc-application.c',
  'gabc-window.c',
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

executable('gabc', gabc_sources,
  dependencies: gabc_deps,
       install: true,
)
