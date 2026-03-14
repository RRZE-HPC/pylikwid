import glob
import os, os.path, glob, re, sys
from setuptools import setup, Extension, find_packages
import ctypes

ver_regex = re.compile(r"so.(\d+)[.]*(\d*)")


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
    print("Searching for LIKWID installation in all folders in $PATH, $LIKWID_PREFIX and /usr/local")
    print("If you can use LIKWID on the CLI but it is not found, use LIKWID_PREFIX=$(realpath $(dirname $(which likwid-topology))/..)")
    include_path = None
    library_path = None
    library = None
    prefix_path = os.getenv('LIKWID_PREFIX', None)
    library_pattern = 'lib*/liblikwid.so*'
    if prefix_path is not None:
        iterator = generic_iglob(os.path.join(prefix_path, library_pattern))
        library = find_file(iterator)
        if library is not None:
            library_path = os.path.dirname(library)
    if library is None:
        paths_iterator = iter(os.environ["PATH"].split(":"))
        is_searching = True
        while is_searching:
            try:
                path = next(paths_iterator)
                prefix_path = os.path.abspath(os.path.join(path, '..'))
                iterator = generic_iglob(os.path.join(prefix_path, library_pattern))
                library = find_file(iterator)
                if library is not None:
                    library_path = os.path.dirname(library)
                    is_searching = False
            except StopIteration:
                is_searching = False
        if library is None:
            prefix_path = '/usr/local'
            iterator = generic_iglob(os.path.join(prefix_path, library_pattern))
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
    m = re.match(r"lib(.*)\.so", os.path.basename(library))
    if m:
        library = m.group(1)
    return prefix_path, library_path, library, include_path

try:
    LIKWID_PREFIX, LIKWID_LIBPATH, LIKWID_LIB, LIKWID_INCPATH = get_hierarchy()
    print(LIKWID_PREFIX, LIKWID_LIBPATH, LIKWID_LIB, LIKWID_INCPATH)
except Exception as e:
    print(e)
    sys.exit(1)


def get_extra_compile_args():
    extra_args = []

    # Add nvmon definition if set to non-zero value in environment.
    if os.environ.get("LIKWID_NVMON") not in (None, "0"):
        extra_args.append("-DLIKWID_NVMON")

    # Query version directly from the library using the exported symbols
    libpath = os.path.join(LIKWID_LIBPATH, "liblikwid.so")
    lib = ctypes.CDLL(libpath)
    lib.likwid_getMajorVersion.restype = ctypes.c_int
    lib.likwid_getMinorVersion.restype = ctypes.c_int
    lib.likwid_getBugfixVersion.restype = ctypes.c_int

    major = lib.likwid_getMajorVersion()
    release = lib.likwid_getMinorVersion()
    minor = lib.likwid_getBugfixVersion()

    extra_args.extend([
        f"-DLIKWID_MAJOR={major}",
        f"-DLIKWID_RELEASE={release}",
        f"-DLIKWID_MINOR={minor}",
    ])
    return extra_args

def get_libraries():
    libs = [LIKWID_LIB]
    # bstrlib is bundled into liblikwid on some installs but separate on others
    bstrlib = glob.glob(os.path.join(LIKWID_LIBPATH, "libbstr*.so*"))
    if bstrlib:
        libs.append("bstr")
    return libs


def get_include_dirs():
    include_dirs = ["src/pylikwid", LIKWID_INCPATH]
    likwid_subdir = os.path.join(LIKWID_INCPATH, "likwid")
    if os.path.isdir(likwid_subdir):
        include_dirs.append(likwid_subdir)
    return include_dirs

pylikwid = Extension("pylikwid.pylikwid",
                     include_dirs=get_include_dirs(),
                     libraries=get_libraries(),
                     library_dirs=[LIKWID_LIBPATH],
                     runtime_library_dirs=[LIKWID_LIBPATH],
                     extra_compile_args=get_extra_compile_args(),
                     sources = ["src/pylikwid/pylikwid.c"],
                     py_limited_api=True,
)

setup(
    name="pylikwid",
    version="0.4.2",
    author="Thomas Gruber",
    author_email="thomas.roehl@googlemail.com",
    description="A Python module to access the functions of the LIKWID library",
    license="GPLv2",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    ext_modules=[pylikwid],
)
