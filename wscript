import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

  geos_config = conf.find_program('geos-config', var='GEOS_CONFIG', mandatory=True)
  geos_libdir = popen("%s --ldflags" % geos_config).readline().strip()[2:]
  conf.env.append_value("LIBPATH_GEOS", geos_libdir)
  conf.env.append_value("LIB_GEOS", "geos_c")
  geos_includedir = popen("%s --includes" % geos_config).readline().strip()
  conf.env.append_value("CPPPATH_GEOS", geos_includedir)

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = '_geode'
  obj.source = "_geode.cc"
  obj.uselib = "GEOS"

def shutdown():
  # HACK to get _geode.node out of build directory.
  # better way to do this?
  if Options.commands['clean']:
    if exists('_geode.node'): unlink('_geode.node')
  else:
    if exists('build/default/geode.node') and not exists('geode.node'):
      symlink('build/default/geode.node', 'geode.node')
