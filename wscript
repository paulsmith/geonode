import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')
  opt.add_option('-D', '--debug', action='store_true', default=False, dest='debug')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')

  conf.env['USE_DEBUG'] = Options.options.debug

  geos_config = conf.find_program('geos-config', var='GEOS_CONFIG', mandatory=True)
  geos_libdir = popen("%s --ldflags" % geos_config).readline().strip()[2:]
  conf.env.append_value("LIBPATH_GEOS", geos_libdir)
  conf.env.append_value("LIB_GEOS", "geos_c")
  geos_includedir = popen("%s --includes" % geos_config).readline().strip()
  conf.env.append_value("CPPPATH_GEOS", geos_includedir)

  if conf.env['USE_DEBUG']:
    conf.env.append_value('CCFLAGS', ['-DDEBUG', '-g', '-O0', '-Wall'])
    conf.env.append_value('CXXFLAGS', ['-DDEBUG', '-g', '-O0', '-Wall'])

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = '_geonode'
  obj.source = "_geonode.cc"
  obj.uselib = "GEOS"
    
def shutdown():
  # HACK to get _geonode.node out of build directory.
  # better way to do this?
  if Options.commands['clean']:
    if exists('_geonode.node'): unlink('_geonode.node')
  elif Options.commands['build']:
    if exists('build/default/_geonode.node') and not exists('_geonode.node'):
      symlink('build/default/_geonode.node', '_geonode.node')
