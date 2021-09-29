project ('yawsnd', 'c', version: '0.0.1', meson_version: '>=0.56.0')

cc = meson.get_compiler ('c')

if cc.get_id()=='gcc' or cc.get_id()=='clang'

  warning_c_args = [
    '-Wduplicated-branches',
    '-Wimplicit-fallthrough',
    '-Wmisleading-indentation',
    '-Wstrict-prototypes',
    '-Wunused',
    '-Wunused-variable',
    '-Wdeclaration-after-statement',
    '-Wformat=2',
    '-Wimplicit-function-declaration',
    '-Winit-self',
    '-Wmissing-include-dirs',
    '-Wmissing-prototypes',
    '-Wpointer-arith',
    '-Wformat-nonliteral',
    '-Wenum-conversion',
    '-Wmaybe-uninitialized'
  ]

  add_project_arguments(cc.get_supported_arguments(warning_c_args), language: 'c')

endif

glib_dep = dependency('glib-2.0', version: '>=2.44', required:true)
gusb_dep = dependency('gusb', version:'>=0.3.7', required:true)
mosquitto_dep = dependency('libmosquitto', version:'>=2.0.0', required:true)

yawsnd_dependencies = [
  glib_dep,
  gusb_dep,
  mosquitto_dep
]

subdir('src')
