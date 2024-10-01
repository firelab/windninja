try:
    from setuptools import setup
    from setuptools import Extension
    HAVE_SETUPTOOLS = True
except ImportError:
    from distutils.core import setup, Extension
include_dirs = ['../../src/ninja']

# Define the paths where the library and headers are installed
extension = Extension(
    '_WindNinjaPython',  # Name of the generated shared library (note the underscore)
    sources=['windninjaPYTHON_wrap.cpp'],  # Source files
    include_dirs=include_dirs,  # Paths to search for include
    libraries=['ninja'],  # Libraries to link against
    extra_compile_args=['-std=c++11']  # Additional compile arguments
)

# Define the setup function
setup(
    name='windninja',
    version='0.1',
    author='Your Name',
    description='A simple example module',
    ext_modules=[extension],
    py_modules=['windninja']
)
