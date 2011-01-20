env = Environment()

platform = env['PLATFORM']

Prefix = ARGUMENTS.get('Prefix','/usr/local')

env = Environment(CCFLAGS = '-Wall -pedantic -O3 -ffast-math -fPIC ')
env.MergeFlags(ARGUMENTS.get('CCFLAGS', '').split())

CPPPATH = ['include']
env.Append(CPPPATH = CPPPATH)

LIBS = [["m", "math.h"],
		["fftw3", "fftw3.h"]]

# pkg-config
if platform  == 'posix':
	PKG_CONFIG = ARGUMENTS.get('PKGConfig', 'pkg-config')
elif platform == 'darwin':
	PKG_CONFIG = ARGUMENTS.get('PKGConfig', '/opt/local/bin/pkg-config')

try:
	env.MergeFlags(['!%s --cflags --libs fftw3' % PKG_CONFIG])
except:
	pass

# check the availability of libraries
if not GetOption('clean'):
	conf = Configure(env)
	for (lib, headers) in LIBS:
		if not conf.CheckLibWithHeader(lib, headers, 'c'):
			print "error: '%s' must be installed!" % lib
			Exit(1)
	env = conf.Finish()


if GetOption('clean'):
	targets = ['src', 'unittests', 'examples']
else:
	targets = ['src']
	if COMMAND_LINE_TARGETS:
		targets.extend(COMMAND_LINE_TARGETS)

SConscript(dirs = targets, exports = ['env', 'platform',
	'PKG_CONFIG', 'Prefix'])

env.Install(Prefix + '/include', 'include/sonotopy')
env.Alias('install', Prefix)
