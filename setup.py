import os, os.path, glob, re, subprocess
from distutils.core import setup, Extension

DEF_LIKWID_PREFIX = "/usr/local"


# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

ver_regex = re.compile(b"so.(\d+)[.]*(\d*)")

def get_prefix():
    path = None
    print("Searching for LIKWID installation")
    for p in os.environ["PATH"].split(":"):
        cmd = "find %s/.. -type f -name \"liblikwid.so*\" 2>&1 | grep \"lib/\" | head -n1" % (p,)
        try:
            ps = subprocess.Popen(cmd, shell=True, close_fds=True, stdout=subprocess.PIPE)
            sout, serr = ps.communicate()
            if sout:
                if len(sout) > 0:
                    path = b"/".join(os.path.normpath(sout.strip()).split(b"/")[:-2])
                    break
        except:
            pass
    print("Using LIKWID libray at {!s}".format(path.decode()))
    return bytes(path).decode().strip("\n").encode()

def get_lib_path(prefix):
    path = (prefix.decode()+"/lib").encode()
    lib = "likwid"
    cmd = "find %s -type f -name \"liblikwid.so*\" 2>&1 | sort | tail -n 1" % (prefix.decode(),)
    try:
        ps = subprocess.Popen(cmd, shell=True, close_fds=True, stdout=subprocess.PIPE)
        sout, serr = ps.communicate()

        if sout:
            inp = sout.decode().strip("\n").encode()
            path = os.path.dirname(inp)
            lib = b":"+os.path.basename(inp)
    except:
        pass
    return path, lib

def get_inc_path(prefix):
    path = (prefix.decode()+"/include").encode()
    cmd = "find %s -type f -name \"likwid.h\" 2>&1 | sort | tail -n 1" % (prefix.decode(),)
    try:
        ps = subprocess.Popen(cmd, shell=True, close_fds=True, stdout=subprocess.PIPE)
        sout, serr = ps.communicate()
        if sout:
            path = os.path.dirname(sout.decode().strip("\n").encode())
    except:
        pass
    return path


LIKWID_PREFIX = get_prefix()
if not LIKWID_PREFIX:
    LIKWID_PREFIX = str(DEF_LIKWID_PREFIX)
LIKWID_LIBPATH, LIKWID_LIB = get_lib_path(LIKWID_PREFIX)
LIKWID_INCPATH = get_inc_path(LIKWID_PREFIX)

pylikwid = Extension("pylikwid",
                    include_dirs = [LIKWID_INCPATH.decode()],
                    libraries = [LIKWID_LIB.decode()],
                    #libraies = ["likwid"] links with liblikwid.so
                    #libraies = [":liblikwid.so.4.3"] links with liblikwid.so.4.3
                    library_dirs = [LIKWID_LIBPATH.decode()],
                    sources = ["pylikwid.c"])

setup(
    name = "pylikwid",
    version = "0.2.8",
    author = "Thomas Roehl",
    author_email = "thomas.roehl@googlemail.com",
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
