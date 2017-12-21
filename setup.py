import os, os.path, glob, re
from distutils.core import setup, Extension

# Utility function to read the README.md file.
# Used for the long_description.  It"s nice, because now 1) we have a top level
# README.md file and 2) it"s easier to type in the README.md file than to put a raw
# string in below ...
def read(fname):
    return open(os.path.join(os.path.dirname(__file__), fname)).read()

ver_regex = re.compile("so.(\d+)[.]*(\d*)")

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

pylikwid = Extension("pylikwid",
                    include_dirs = ["/usr/local/include"],
                    libraries = [get_highest_version(["/usr/local/lib"], "liblikwid.so")],
                    #libraies = ["likwid"] links with liblikwid.so
                    #libraies = [":liblikwid.so.4.3"] links with liblikwid.so.4.3
                    library_dirs = ["/usr/local/lib"],
                    sources = ["pylikwid.c"])

setup(
    name = "pylikwid",
    version = "0.2.2",
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
