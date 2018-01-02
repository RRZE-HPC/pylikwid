import os, os.path, glob, re, subprocess
from distutils.core import setup, Extension

DEF_LIKWID_PREFIX = "/usr/local"


# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

ver_regex = re.compile("so.(\d+)[.]*(\d*)")

def get_prefix():
    path = None
    print("Searching for LIKWID installation")
    for p in os.environ["PATH"].split(":"):
        cmd = "find %s/.. -type f -name \"liblikwid.so*\" 2>&1 | grep \"lib/\"" % (p,)
        ps = subprocess.Popen(cmd, shell=True, close_fds=True, stdout=subprocess.PIPE)
        sout, serr = ps.communicate()
        if sout:
            if len(sout) > 0:
                path = "/".join(os.path.normpath(sout.strip()).split("/")[:-2])
                break
    return path


def get_highest_version(paths, lib):
    max_lib = ""
    max_path = ""
    for p in paths:
        if os.path.exists(p):
            libs = glob.glob(os.path.join(p, lib)+"*")
            for l in libs:
                if ver_regex.search(l):
                    if len(l) > len(max_lib):
                        max_lib = l
                        max_path = p
                        continue
                    if len(max_lib) > 0 and len(l) == len(max_lib):
                        m = ver_regex.search(l)
                        max_m = ver_regex.search(max_lib)
                        for i in range(len(max_m.groups())):
                            if int(m.group(i+1)) > int(max_m.group(i+1)):
                                max_lib = l
                                max_path = p
                                break
    return ":"+max_lib.replace(max_path, "").strip("/")

LIKWID_PREFIX = get_prefix()
if not LIKWID_PREFIX:
    LIKWID_PREFIX = DEF_LIKWID_PREFIX

pylikwid = Extension("pylikwid",
                    include_dirs = [os.path.join(LIKWID_PREFIX, "include")],
                    libraries = [get_highest_version([os.path.join(LIKWID_PREFIX, "lib")], "liblikwid.so")],
                    #libraies = ["likwid"] links with liblikwid.so
                    #libraies = [":liblikwid.so.4.3"] links with liblikwid.so.4.3
                    library_dirs = [os.path.join(LIKWID_PREFIX, "lib")],
                    sources = ["pylikwid.c"])

setup(
    name = "pylikwid",
    version = "0.2.4",
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
