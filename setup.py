import os
from distutils.core import setup, Extension

# Utility function to read the README.md file.
# Used for the long_description.  It's nice, because now 1) we have a top level
# README.md file and 2) it's easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

pylikwid = Extension('pylikwid',
                    include_dirs = ["/usr/local/include"],
                    libraries = ['likwid'],
                    library_dirs = ["/usr/local/lib"],
                    sources = ['likwidmodule.c'])

setup(
    name = "likwid_python_api",
    version = "0.1",
    author = "Thomas Roehl",
    author_email = "thomas.roehl@gmail.com",
    description = ("A Python module to access the function of the LIKWID library"),
    license = "GPL",
    keywords = "hpc performance benchmark analysis",
    url = "https://github.com/TomTheBear/likwid-python-api",
    long_description=read('README.md'),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Topic :: Utilities",
        "License :: OSI Approved :: GPL License",
    ],
    package_data = {
        "pylikwid" : ["likwidmodule.c", "README.md", "LICENSE"],
        "tests" : ["testlib.py", "testmarker.py", "testmarker.sh"]
    },
    ext_modules = [pylikwid]
)
