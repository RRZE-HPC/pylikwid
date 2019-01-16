import os, os.path, glob, re
from distutils.core import setup, Extension


# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


ver_regex = re.compile(b"so.(\d+)[.]*(\d*)")


def find_file(path_iterator):
    path = None
    path_to_explore = None
    is_searching = True
    if path_iterator:
        while is_searching:
            try:
                path_to_explore = next(path_iterator)
                if os.path.exists(path_to_explore):
                    path = path_to_explore
                is_searching = False
            except StopIteration:
                is_searching = False
    return path


def get_hierarchy():
    print("Searching for LIKWID installation")
    include_path = None
    library_path = None
    library = None
    prefix = os.getenv('LIKWID_PREFIX', None)
    library_pattern = 'lib*/liblikwid.so*'
    if prefix is not None:
        iterator = glob.iglob(prefix + library_pattern, recursive=True)
        library = find_file(iterator)
        if library is not None:
            library_path = os.path.dirname(library)
    else:
        paths_iterator = iter(os.environ["PATH"].split(":"))
        is_searching = True
        while is_searching:
            try:
                path = next(paths_iterator)
                prefix = os.path.abspath(path + '../')
                iterator = glob.iglob(prefix + library_pattern,
                                      recursive=True)
                library = find_file(iterator)
                if library is not None:
                    library_path = os.path.dirname(library)
                    is_searching = False
            except StopIteration:
                is_searching = False
            if library_path is None:
                prefix = '/usr/local'
                iterator = glob.iglob(prefix + library_pattern,
                                      recursive=True)
                library = find_file(iterator)
                if library is not None:
                    library_path = os.path.dirname(library)
                    is_searching = False

    include_path = os.path.join(prefix, 'include')
    if not os.path.exists(os.path.join(include_path, 'likwid.h')):
        iterator = glob.iglob(prefix + '**/likwid.h', recursive=True)
        include_path = find_file(iterator)
    if prefix is None or not os.path.exists(prefix):
        raise Exception('Error the likwid prefix directory was not found')
    if library is None or not os.path.exists(library):
        raise Exception('Error the likwid library was not found')
    if library_path is None or not os.path.exists(library_path):
        raise Exception('Error the likwid library directory was not found')
    if include_path is None or not os.path.exists(include_path):
        raise Exception('Error the likwid include directory was not found')
    print("Using LIKWID library at {!s}".format(library_path))
    return prefix, library_path, library, include_path


LIKWID_PREFIX, LIKWID_LIBPATH, LIKWID_LIB, LIKWID_INCPATH = get_hierarchy()

pylikwid = Extension("pylikwid",
                     include_dirs=[LIKWID_INCPATH],
                     libraries=[LIKWID_LIB],
                     library_dirs=[LIKWID_LIBPATH],
                     sources=["pylikwid.c"])

setup(
    name="pylikwid",
    version="0.2.9",
    author="Thomas Roehl",
    author_email="thomas.roehl@googlemail.com",
    description="A Python module to access the function of the LIKWID library",
    long_description=read("README.rst"),
    license="GPLv2",
    keywords="hpc performance benchmark analysis",
    url="https://github.com/RRZE-HPC/pylikwid",
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
    package_data={
        "pylikwid": ["pylikwid.c", "README.rst", "LICENSE"],
        "tests": ["tests/*.py"]
    },
    ext_modules=[pylikwid]
)
