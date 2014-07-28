import sysconfig
from setuptools import setup, Extension

libs = [ 'sonotopy', 'fftw3' ]
lang = 'c++'
swig_opts = ['-c++']


setup(
    name = "sonotopy",
    description = "Python bindings for the sonotopy library.",
    version = 0.1,
    url = "https://github.com/alex-berman/sonotopy",
    license = "GNU General Public License Version 3",
    keywords = [ "Audio" ],
    maintainer = "Peter Bouda",
    maintainer_email = "pbouda@cidles.eu",
    author = "Peter Bouda",
    author_email = "pbouda@cidles.eu",
    classifiers = [
    'Development Status :: 4 - Beta',
    'Intended Audience :: Developers',
    'Intended Audience :: Information Technology',
    'Intended Audience :: Science/Research',
    'License :: OSI Approved :: GNU General Public License v3 (GPLv3)',
    'Operating System :: OS Independent',
    'Programming Language :: Python :: 2.7',
    'Programming Language :: Python :: 3.3',
    'Programming Language :: Python :: 3.4',
    'Topic :: Artistic Software',
    'Topic :: Multimedia :: Sound/Audio',
    'Topic :: Multimedia :: Sound/Audio :: Analysis',
    'Topic :: Scientific/Engineering',
    'Topic :: Scientific/Engineering :: Visualization'
    ],
    ext_modules=[Extension('_sonotopy', ['sonotopy.i'], libraries=libs, swig_opts=swig_opts)],
    py_modules=['sonotopy']
)
