import os, os.path, glob, re, sys
from distutils.core import setup, Extension


# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()


ver_regex = re.compile(b"so.(\d+)[.]*(\d*)")


def generic_iglob(pattern):
    result = None
    if sys.version_info.major == 3 and sys.version_info.minor > 4:
        result = glob.iglob(pattern, recursive=True)
    else:
        result = glob.iglob(pattern)
    return result


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
    prefix_path = os.getenv('LIKWID_PREFIX', None)
    library_pattern = '/lib*/liblikwid.so*'
    if prefix_path is not None:
        iterator = generic_iglob(prefix_path + library_pattern)
        library = find_file(iterator)
        if library is not None:
            library_path = os.path.dirname(library)
    if library is None:
        paths_iterator = iter(os.environ["PATH"].split(":"))
        is_searching = True
        while is_searching:
            try:
                path = next(paths_iterator)
                prefix_path = os.path.abspath(path + '../')
                iterator = generic_iglob(prefix_path + library_pattern)
                library = find_file(iterator)
                if library is not None:
                    library_path = os.path.dirname(library)
                    is_searching = False
            except StopIteration:
                is_searching = False
        if library is None:
            prefix_path = '/usr/local'
            iterator = generic_iglob(prefix_path + library_pattern)
            library = find_file(iterator)
            if library is not None:
                library_path = os.path.dirname(library)

    include_path = os.path.join(prefix_path, 'include')
    if not os.path.exists(os.path.join(include_path, 'likwid.h')):
        iterator = generic_iglob(prefix_path + '**/likwid.h')
        include_path = find_file(iterator)
    if prefix_path is None or not os.path.exists(prefix_path):
        raise Exception('Error the likwid prefix directory was not found')
    if library is None or not os.path.exists(library):
        raise Exception('Error the likwid library was not found')
    if library_path is None or not os.path.exists(library_path):
        raise Exception('Error the likwid library directory was not found')
    if include_path is None or not os.path.exists(include_path):
        raise Exception('Error the likwid include directory was not found')
    print("Using LIKWID prefix directory at {!s}".format(prefix_path))
    print("Using LIKWID include directory at {!s}".format(include_path))
    print("Using LIKWID library directory at {!s}".format(library_path))
    print("Using LIKWID library named {!s}".format(library))
    m = re.match("lib(.*)\.so", os.path.basename(library))
    if m:
        library = m.group(1)
    return prefix_path, library_path, library, include_path

try:
    LIKWID_PREFIX, LIKWID_LIBPATH, LIKWID_LIB, LIKWID_INCPATH = get_hierarchy()
    print(LIKWID_PREFIX, LIKWID_LIBPATH, LIKWID_LIB, LIKWID_INCPATH)
except Exception as e:
    print(e)
    sys.exit(1)


pylikwid = Extension("pylikwid",
                     include_dirs=[LIKWID_INCPATH],
                     libraries=[LIKWID_LIB],
                     library_dirs=[LIKWID_LIBPATH],
                     sources=["pylikwid.c"])

setup(
    name="pylikwid",
    version="0.3.1",
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
