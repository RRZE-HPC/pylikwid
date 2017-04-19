pylikwid
========

Python interface for the C API of LIKWID
(https://github.com/RRZE-HPC/likwid)


.. image:: https://travis-ci.org/RRZE-HPC/pylikwid.svg?branch=master
    :target: https://travis-ci.org/RRZE-HPC/pylikwid?branch=master

Installation
============

I added a setup.py script for the installation. It builds the C module
and copies it to the proper destination.

::

    $ tar -xjf likwid-python-api.tar.bz2
    $ cd likwid-python-api
    # Build C interface
    $ python setup.py build_ext -I <include path for likwid> -L <library path for likwid> -R <library path for likwid>
    # Install module to the proper location
    $ python setup.py install (--prefix=<where to install>)
    # Testing
    $ python -c "import pylikwid"
    $ ./testlib.py

Functions
=========

After ``import pylikwid`` you can call the following functions:

Marker API
----------

-  ``pylikwid.init()``: Initialize the Marker API of the LIKWID library.
   Must be called previous to all other functions.
-  ``pylikwid.threadinit()``: Add the current thread to the Marker API.
   Since Python is commonly single-threaded simply call it directly
   after ``pylikwid.init()``
-  ``rr = pylikwid.registerregion(regiontag)``: Register a region to the
   Marker API. This is an optional function to reduce the overhead of
   region registration at ``pylikwid.startregion``. If you don’t call
   ``pylikwid.registerregion(regiontag)``, the registration is done at
   ``pylikwid.startregion(regiontag)``. On success, 0 is return. If you
   havn’t called ``pylikwid.init()``, a negative number is returned.
-  ``err = pylikwid.startregion(regiontag)``: Start measurements under
   the name ``regiontag``. On success, 0 is return. If you havn’t called
   ``pylikwid.init()``, a negative number is returned.
-  ``err = pylikwid.stopregion(regiontag)``: Stop measurements under the
   name ``regiontag`` again. On success, 0 is return. If you havn’t
   called ``pylikwid.init()``, a negative number is returned.
-  ``num_events, events[], time, count = pylikwid.getregion(regiontag)``:
   Get the intermediate results of the region identified by
   ``regiontag``. On success, it returns the number of events in the
   current group, a list with all the aggregated event results, the
   measurement time for the region and the number of calls.
-  ``pylikwid.nextgroup()``: Switch to the next event set in a
   round-robin fashion. If you have set only one event set on the
   command line, this function performs no operation.
-  ``pylikwid.close()``: Close the connection to the LIKWID Marker API
   and write out measurement data to file. This file will be evaluated
   by ``likwid-perfctr``.
-  ``pylikwid.getprocessorid()``: Returns the ID of the currently
   executing CPU
-  ``pylikwid.pinprocess(cpuid)``: Pins the current process to the CPU
   given as ``cpuid``.
-  ``pylikwid.pinthread(cpuid)``: Pins the current thread to the CPU
   given as ``cpuid``.

Topology
--------

-  ``pylikwid.inittopology()``: Initialize the topology module (reads in
   system topology)
-  ``infodict = pylikwid.getcpuinfo()``: Return a dict with general
   information about the system (CPU model, CPU family, …)

   -  ``osname``: Name of the CPU retrieved from the CPUID leafs
   -  ``name``: Name of the micro architecture
   -  ``short_name``: Short name of the micro architectur