import os

env = Environment()

platform = env['PLATFORM']

Prefix = ARGUMENTS.get('Prefix','/usr/local')

env = Environment(CCFLAGS = '-Wall -pedantic -ffast-math -fPIC ')

DEBUG = int(ARGUMENTS.get('DEBUG', '0'))
if DEBUG:
	CCFLAGS = '-ggdb2 -O0 -DDEBUG=1 '
else:
	CCFLAGS = '-g0 -O3 '
env.Append(CCFLAGS = CCFLAGS)

CPPPATH = ['include']
env.Append(CPPPATH = CPPPATH)

env.MergeFlags(ARGUMENTS.get('CCFLAGS', ''))

if 'CROSS' in os.environ:
	cross = os.environ['CROSS']
	env.Append(CROSS = cross)
	env.Replace(CC = cross + 'gcc')
	env.Replace(CXX = cross + 'g++')
	env.Replace(LD = cross + 'ld')

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
	targets = ['src', 'unittests', 'uilib', 'examples', 'lab', 'performanceTest']
	env.Clean('build', 'build')
else:
	targets = ['src']
	targets.extend([t for t in COMMAND_LINE_TARGETS
			if t in ['unittests', 'uilib', 'examples', 'lab', 'performanceTest']])

variant_dir = ['release', 'debug'][DEBUG]

for t in targets:
	SConscript(dirs = t, exports = ['env', 'platform',
		'PKG_CONFIG', 'Prefix'], variant_dir = 'build/%s/%s' % (variant_dir, t), duplicate = 0)

env.Install(Prefix + '/include/sonotopy', Glob('include/sonotopy/*.hpp'))
env.Alias('install', Prefix)

