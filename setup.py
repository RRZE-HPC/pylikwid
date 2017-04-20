import os
from distutils.core import setup, Extension

# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

pylikwid = Extension("pylikwid",
                    include_dirs = ["/usr/local/include"],
                    libraries = ["likwid"],
                    library_dirs = ["/usr/local/lib"],
                    sources = ["pylikwid.c"])

setup(
    name = "pylikwid",
    version = "0.2.1",
    author = "Thomas Roehl",
    author_email = "thomas.roehl@gmail.com",
    description = ("A Python module to access the function of the LIKWID library"),
    long_description=read("README.rst"),
    license = "GPLv2",
    keywords = "hpc performance benchmark analysis",
    url = "https://github.com/RRZE-HPC/pylikwid",
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering",
        "Topic :: Software Development",
        "Topic :: Utilities",
        "License :: OSI Approved :: GNU General Public License v2 (GPLv2)",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 2.7",
    ],
    package_data = {
        "pylikwid" : ["pylikwid.c", "README.rst", "LICENSE"],
        "tests" : ["tests/*.py"]
    },
    ext_modules = [pylikwid]
)
