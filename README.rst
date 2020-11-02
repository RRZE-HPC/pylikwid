pylikwid
========

Python interface for the C API of LIKWID
(https://github.com/RRZE-HPC/likwid)

.. image:: https://travis-ci.com/RRZE-HPC/pylikwid.svg?branch=master
    :target: https://travis-ci.com/RRZE-HPC/pylikwid?branch=master

Installation
============

I added a setup.py script for the installation. It builds the C module
and copies it to the proper destination.

::

    $ git clone https://github.com/RRZE-HPC/pylikwid.git
    $ cd pylikwid
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

-  ``pylikwid.markerinit()``: Initialize the Marker API of the LIKWID library.
   Must be called previous to all other functions.
-  ``pylikwid.markerthreadinit()``: Add the current thread to the Marker API.
   Since Python is commonly single-threaded simply call it directly
   after ``pylikwid.markerinit()``
-  ``rr = pylikwid.registerregion(regiontag)``: Register a region to the
   Marker API. This is an optional function to reduce the overhead of
   region registration at ``pylikwid.markerstartregion``. If you don't call
   ``pylikwid.registerregion(regiontag)``, the registration is done at
   ``pylikwid.markerstartregion(regiontag)``. On success, 0 is return. If you
   havn't called ``pylikwid.markerinit()``, a negative number is returned.
-  ``err = pylikwid.markerstartregion(regiontag)``: Start measurements under
   the name ``regiontag``. On success, 0 is return. If you havn't called
   ``pylikwid.markerinit()``, a negative number is returned.
-  ``err = pylikwid.markerstopregion(regiontag)``: Stop measurements under the
   name ``regiontag`` again. On success, 0 is return. If you havn't
   called ``pylikwid.markerinit()``, a negative number is returned.
-  ``num_events, events[], time, count = pylikwid.markergetregion(regiontag)``:
   Get the intermediate results of the region identified by
   ``regiontag``. On success, it returns the number of events in the
   current group, a list with all the aggregated event results, the
   measurement time for the region and the number of calls.
-  ``pylikwid.nextgroup()``: Switch to the next event set in a
   round-robin fashion. If you have set only one event set on the
   command line, this function performs no operation.
-  ``pylikwid.markerreset(regiontag)``: Reset the values stored using the region
   name ``regiontag``. On success, 0 is returned.
-  ``pylikwid.markerclose()``: Close the connection to the LIKWID Marker API
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
   information about the system (CPU model, CPU family, ...)

   -  ``osname``: Name of the CPU retrieved from the CPUID leafs
   -  ``name``: Name of the micro architecture
   -  ``short_name``: Short name of the micro architecture
   -  ``family``: ID of the CPU family
   -  ``model``: Vendor-specific model number of the CPU
   -  ``stepping``: Stepping (Revision) of the CPU
   -  ``perf_version``: Version number of the hardware performance
      monitoring capabilities
   -  ``perf_num_ctr``: Amount of general-purpose counter registers per
      hardware thread
   -  ``perf_num_fixed_ctr``: Amount of fixed-purpose counter registers
      per hardware thread
   -  ``perf_width_ctr``: Bit length of the counter registers
   -  ``clock``: CPU clock (only unequal to 0 if timer module is
      initialized)
   -  ``turbo``: Is turbo mode supported?
   -  ``isIntel``: Is it an Intel CPU?
   -  ``supportUncore``: Does the system have performance monitoring
      counters in the Uncore?
   -  ``features``: String with performance relevant CPU features (AVX,
      SSE, ...)
   -  ``featureFlags``: Bitmask for all available CPU features

-  ``topodict = pylikwid.getcputopology()``: Return a dict with the
   topology of the system. Here is a list of fields in the dict:

   -  ``numSockets``: Number of CPU sockets
   -  ``numHWThreads``: Number of hardware threads (physical +
      hyperthreading cores)
   -  ``activeHWThreads``: Number of active/usable hardware threads
   -  ``numCoresPerSocket``: Amount of hardware threads per CPU socket
   -  ``numThreadsPerCore``: Amount of hardware threads assembled in
      every physical CPU core
   -  ``numCacheLevels``: Amount of levels in cacheing hierarchy
   -  ``cacheLevels``: Dict with information about the cache levels,
      keys are the levels (1, 2, 3,...)

      -  ``level``: Level of the cache in the hierarchy
      -  ``lineSize``: Size of a cache line
      -  ``sets``: Amount of sets
      -  ``inclusive``: Is the cache inclusive or exclusive?\`
      -  ``threads``: Amount of threads attached to the cache
      -  ``associativity``: Associativity of the cache
      -  ``type``: data (= data cache), unified = (data + instruction
         cache)
      -  ``size``: Size of the cache in bytes

   -  ``threadPool``: Dict with information about the hardware threads.
      Keys are the os-generated ID of the hardware thread

      -  ``coreId``: ID of the corresponding physical core
      -  ``apicId``: ID set by the operating system
      -  ``threadId``: ID of the hardware thread in the physical core
      -  ``packageId``: ID of the CPU socket hosting the hardware thread

-  ``pylikwid.printsupportedcpus()``: Prints all supported micro
   architecture names to stdout
-  ``pylikwid.finalizetopology()``: Delete all information in the
   topology module

NUMA
----

-  ``numadict = pylikwid.initnuma()``: Initialize the NUMA module and
   return the gathered values

   -  ``numberOfNodes``: Amount of NUMA nodes in the system
   -  ``nodes``: Dict holding the information about the NUMA domains.
      Keys are the NUMA domain IDs

      -  ``id``: ID of the NUMA domain (should be equal to dict key)
      -  ``numberOfProcessors``: Number of hardware threads attached to
         the NUMA domain
      -  ``processors``: List of all CPU IDs attached to the NUMA domain
      -  ``freeMemory``: Amount of free memory in the NUMA domain (in
         Kbytes)
      -  ``totalMemory``: Amount of total memory in the NUMA domain (in
         Kbytes)
      -  ``numberOfDistances``: How many distances to self/other NUMA
         domains
      -  ``distances``: List with distances, NUMA domain IDs are the
         destination indexes in the list

-  ``pylikwid.finalizenuma()``: Delete all information in the NUMA
   module

Affinity
--------

-  ``affdict = pylikwid.initaffinity()``: Initialize the affinity domain
   module and return the gathered values

   -  ``numberOfAffinityDomains``: Amount of affinity domains
   -  ``numberOfSocketDomains``: Amount of CPU socket related affinity
      domains
   -  ``numberOfNumaDomains``: Amount of NUMA related affinity domains
   -  ``numberOfCacheDomains``: Amount of last level cache related
      affinity domains
   -  ``numberOfProcessorsPerSocket``: Amount of hardware threads per
      CPU socket
   -  ``numberOfCoresPerCache``: Amount of physical CPU cores per last
      level cache
   -  ``numberOfProcessorsPerCache``: Amount of hardware threads per
      last level cache
   -  ``domains``: Dict holding the information about the affinity
      domains

      -  ``tag``: Name of the affinity domain (N = node, SX = socket X,
         CY = cache Y, MZ = memory domain Z)
      -  ``numberOfProcessors``: Amount of hardware threads in the
         domain
      -  ``numberOfCores``: Amount of physical CPU cores in the domain
      -  ``processorList``: List holding the CPU IDs in the domain

-  ``pylikwid.finalizeaffinity()``: Delete all information in the
   affinity domain module
-  ``pylikwid.cpustr_to_cpulist()``: Transform a valid cpu string in
   LIKWID syntax into a list of CPU IDs

Timer
-----

-  ``pylikwid.getcpuclock()``: Return the CPU clock
-  ``t_start = pylikwid.startclock()``: Start the clock and return the
   current timestamp
-  ``t_end = pylikwid.stopclock()``: Stop the clock and return the
   current timestamp
-  ``t = pylikwid.getclock(t_start, t_end)``: Return the time in seconds
   between ``t_start`` and ``t_end``
-  ``c = pylikwid.getclockcycles(t_start, t_end)``: Return the amount of
   CPU cycles between ``t_start`` and ``t_end``

Temperature
-----------

-  ``pylikwid.inittemp(cpu)``: Initialize the temperature module for CPU
   ``cpu``
-  ``pylikwid.readtemp(cpu)``: Read the current temperature of CPU
   ``cpu``

Energy
------

-  ``pinfo = pylikwid.getpowerinfo()``: Initializes the energy module
   and returns gathered information. If it returns ``None``, there is no
   energy support

   -  ``minFrequency``: Minimal possible frequency of a CPU core
   -  ``baseFrequency``: Base frequency of a CPU core
   -  ``hasRAPL``: Are energy reading supported?
   -  ``timeUnit``: Time unit
   -  ``powerUnit``: Power unit
   -  ``domains``: Dict holding the information about the energy
      domains. Keys are PKG, PP0, PP1, DRAM

      -  ``ID``: ID of the energy domain
      -  ``energyUnit``: Unit to derive raw register counts to uJ
      -  ``supportInfo``: Is the information register available?
      -  ``tdp``: TDP of the domain (only if supportInfo == True)
      -  ``minPower``: Minimal power consumption by the domain (only if
         supportInfo == True)
      -  ``maxPower``: Maximal power consumption by the domain (only if
         supportInfo == True)
      -  ``maxTimeWindow``: Maximal time window between updates of the
         energy registers
      -  ``supportStatus``: Are energy readings from the domain are
         possible?
      -  ``supportPerf``: Is power capping etc. available?
      -  ``supportPolicy``: Can we set a power policy for the domain?

-  ``e_start = pylikwid.startpower(cpu, domainid)``: Return the start
   value for a cpu for the domain with ``domainid``. The ``domainid``
   can be found in ``pinfo["domains"][domainname]["ID"]``
-  ``e_stop = pylikwid.stoppower(cpu, domainid)``: Return the stop value
   for a cpu for the domain with ``domainid``. The ``domainid`` can be
   found in ``pinfo["domains"][domainname]["ID"]``
-  ``e = pylikwid.getpower(e_start, e_stop, domainid)``: Calculate the
   uJ from the values retrieved by ``startpower`` and ``stoppower``.

Configuration
-------------

-  ``pylikwid.initconfiguration()``: Read in config file from different
   places. Default is ``/etc/likwid.cfg``
-  ``config = pylikwid.getconfiguration()``: Get the dict with the
   configuration options

   -  ``configFileName``: Path to the config file
   -  ``topologyCfgFileName``: If a topology file was created with
      ``likwid-genTopoCfg`` and found by ``initconfiguration()``
   -  ``daemonPath``: Path to the access daemon executable
   -  ``groupPath``: Path to the base directory with the performance
      group files
   -  ``daemonMode``: Configured access mode (0=direct, 1=accessDaemon)
   -  ``maxNumThreads``: Maximal amount of hardware threads that can be
      handled by LIKWID
   -  ``maxNumNodes``: Maximal amount of CPU sockets that can be handled
      by LIKWID

-  ``pylikwid.destroyconfiguration()``: Destroy all information about
   the configuration

Access module
-------------

-  ``pylikwid.hpmmode(mode)``: Set access mode. For x86 there are two
   modes:

   -  ``mode = 0``: Access the MSR and PCI devices directly. May require
      root access
   -  ``mode = 1``: Access the MSR and PCI devices through access daemon
      instances

-  ``pylikwid.hpminit()``: Initialize the access functions according to
   the access mode
-  ``pylikwid.hpmaddthread(cpu)``: Add CPU ``cpu`` to the access layer
   (opens devices files or connection to an access daemon)
-  ``pylikwid.hpmfinalize()``: Unregister all CPUs from the access layer
   and close files/connections

Performance Monitoring
----------------------

-  ``pylikwid.init(cpus)``: Initialize the perfmon module for the CPUs
   given in list ``cpus``
-  ``pylikwid.getnumberofthreads()``: Return the number of threads
   initialized in the perfmon module
-  ``pylikwid.getnumberofgroups()``: Return the number of groups
   currently registered in the perfmon module
-  ``pylikwid.getgroups()``: Return a list of all available groups. Each
   list entry is a dict:

   -  ``Name``: Name of the performance group
   -  ``Short``: Short information about the performance group
   -  ``Long``: Long description of the performance group

-  ``gid = pylikwid.addeventset(estr)``: Add a performance group or a
   custom event set to the perfmon module. The ``gid`` is required to
   specify the event set later
-  ``pylikwid.getnameofgroup(gid)``: Return the name of the group
   identified by ``gid``. If it is a custom event set, the name is set
   to ``Custom``
-  ``pylikwid.getshortinfoofgroup(gid)``: Return the short information
   about a performance group
-  ``pylikwid.getlonginfoofgroup(gid)``: Return the description of a
   performance group
-  ``pylikwid.getnumberofevents(gid)``: Return the amount of events in
   the group
-  ``pylikwid.getnumberofmetrics(gid)``: Return the amount of derived
   metrics in the group. Always 0 for custom event sets.
-  ``pylikwid.getnameofevent(gid, eidx)``: Return the name of the event
   identified by ``gid`` and the index in the list of events
-  ``pylikwid.getnameofcounter(gid, eidx)``: Return the name of the
   counter register identified by ``gid`` and the index in the list of
   events
-  ``pylikwid.getnameofmetric(gid, midx)``: Return the name of a derived
   metric identified by ``gid`` and the index in the list of metrics
-  ``pylikwid.setup(gid)``: Program the counter registers to measure all
   events in group ``gid``
-  ``pylikwid.start()``: Start the counter registers
-  ``pylikwid.stop()``: Stop the counter registers
-  ``pylikwid.read()``: Read the counter registers (stop->read->start)
-  ``pylikwid.switch(gid)``: Switch to group ``gid``
   (stop->setup(gid)->start)
-  ``pylikwid.getidofactivegroup()`` Return the ``gid`` of the currently
   configured group
-  ``pylikwid.getresult(gid, eidx, tidx)``: Return the raw counter
   register result of all measurements identified by group ``gid`` and
   the indices for event ``eidx`` and thread ``tidx``
-  ``pylikwid.getlastresult(gid, eidx, tidx)``: Return the raw counter
   register result of the last measurement cycle identified by group
   ``gid`` and the indices for event ``eidx`` and thread ``tidx``
-  ``pylikwid.getmetric(gid, midx, tidx)``: Return the derived metric
   result of all measurements identified by group ``gid`` and the
   indices for metric ``midx`` and thread ``tidx``
-  ``pylikwid.getlastmetric(gid, midx, tidx)``: Return the derived
   metric result of the last measurement cycle identified by group
   ``gid`` and the indices for metric ``midx`` and thread ``tidx``
-  ``pylikwid.gettimeofgroup(gid)``: Return the measurement time for
   group identified by ``gid``
-  ``pylikwid.finalize()``: Reset all used registers and delete internal
   measurement results

Marker API result file reader
-----------------------------

-  ``pylikwid.markerreadfile(filename)``: Reads in the result file of an
   application run instrumented by the LIKWID Marker API
-  ``pylikwid.markernumregions()``: Return the number of regions in an
   application run
-  ``pylikwid.markerregiontag(rid)``: Return the region tag for the
   region identified by ``rid``
-  ``pylikwid.markerregiongroup(rid)``: Return the group name for the
   region identified by ``rid``
-  ``pylikwid.markerregionevents(rid)``: Return the amount of events for
   the region identified by ``rid``
-  ``pylikwid.markerregionthreads(rid)``: Return the amount of threads
   that executed the region identified by ``rid``
-  ``pylikwid.markerregiontime(rid, tidx)``: Return the accumulated
   measurement time for the region identified by ``rid`` and the thread
   index ``tidx``
-  ``pylikwid.markerregioncount(rid, tidx)``: Return the call count for
   the region identified by ``rid`` and the thread index ``tidx``
-  ``pylikwid.markerregionresult(rid, eidx, tidx)``: Return the call
   count for the region identified by ``rid``, the event index ``eidx``
   and the thread index ``tidx``
-  ``pylikwid.markerregionmetric(rid, midx, tidx)``: Return the call
   count for the region identified by ``rid``, the metric index ``midx``
   and the thread index ``tidx``

GPU Topology (if LIKWID is built with Nvidia interface)
-------------------------------------------------------

-  ``pylikwid.initgputopology()``: Initialize the topology module (reads in
   system topology)

-  ``topolist = pylikwid.getgputopology()``: Return a list with the
   GPU topology of the system. Each GPU is represented by a dict. The entries in
   the dicts are:

   -  ``devid``: Device identifier for the GPU
   -  ``numaNode``: The NUMA node identifier the GPU is attached at
   -  ``name``: Name of the device
   -  ``mem``: Memory capacity of the device
   -  ``ccapMajor``: Major number of the compute capability
   -  ``ccapMinor``: Minor number of the compute capability
   -  ``maxThreadsDim[3]``: Maximum sizes of each dimension of a block
   -  ``maxGridSize[3]``: Maximum sizes of each dimension of a grid
   -  ``maxThreadsPerBlock``: Maximam number of thread per block
   -  ``sharedMemPerBlock``: Total amount of shared memory available per block
   -  ``totalConstantMemory``: Total amount of constant memory available on the device
   -  ``simdWidth``: SIMD width of arithmetic units = warp size
   -  ``memPitch``: Maximum pitch allowed by the memory copy functions that involve memory regions allocated through cuMemAllocPitch()
   -  ``regsPerBlock``: Total number of registers available per block
   -  ``clockRatekHz``: Clock frequency in kilohertz
   -  ``textureAlign``: Alignment requirement
   -  ``surfaceAlign``: Alignment requirement for surfaces
   -  ``l2Size``: L2 cache in bytes. 0 if the device doesn't have L2 cache
   -  ``memClockRatekHz``: Peak memory clock frequency in kilohertz
   -  ``pciBus``: PCI bus identifier of the device
   -  ``pciDev``: PCI device (also known as slot) identifier of the device
   -  ``pciDom``: PCI domain identifier of the device
   -  ``maxBlockRegs``: Maximum number of 32-bit registers available to a thread block
   -  ``numMultiProcs``: Number of multiprocessors on the device
   -  ``maxThreadPerMultiProc``: Maximum resident threads per multiprocessor
   -  ``memBusWidth``: Global memory bus width in bits
   -  ``unifiedAddrSpace``: 1 if the device shares a unified address space with the host, or 0 if not
   -  ``ecc``: 1 if error correction is enabled on the device, 0 if error correction is disabled or not supported by the device
   -  ``asyncEngines``: Number of asynchronous engines
   -  ``mapHostMem``: 1 if the device can map host memory into the CUDA address space
   -  ``integrated``: 1 if the device is an integrated (motherboard) GPU and 0 if it is a discrete (card) component

-  ``pylikwid.finalizegputopology()``: Delete all information in the
   topology module


Performance Monitoring for Nvidia GPUs (if LIKWID is built with Nvidia interface)
---------------------------------------------------------------------------------

-  ``pylikwid.nvinit(gpus)``: Initialize the nvmon module for the GPUs
   given in list ``gpus``
-  ``pylikwid.nvgetnumberofgpus()``: Return the number of GPUs
   initialized in the nvmon module
-  ``pylikwid.nvgetnumberofgroups()``: Return the number of groups
   currently registered in the nvmon module
-  ``pylikwid.nvgetgroups()``: Return a list of all available groups. Each
   list entry is a dict:

   -  ``Name``: Name of the performance group
   -  ``Short``: Short information about the performance group
   -  ``Long``: Long description of the performance group

-  ``gid = pylikwid.nvaddeventset(estr)``: Add a performance group or a
   custom event set to the perfmon module. The ``gid`` is required to
   specify the event set later
-  ``pylikwid.nvgetnameofgroup(gid)``: Return the name of the group
   identified by ``gid``. If it is a custom event set, the name is set
   to ``Custom``
-  ``pylikwid.nvgetshortinfoofgroup(gid)``: Return the short information
   about a performance group
-  ``pylikwid.nvgetlonginfoofgroup(gid)``: Return the description of a
   performance group
-  ``pylikwid.nvgetnumberofevents(gid)``: Return the amount of events in
   the group
-  ``pylikwid.nvgetnumberofmetrics(gid)``: Return the amount of derived
   metrics in the group. Always 0 for custom event sets.
-  ``pylikwid.nvgetnameofevent(gid, eidx)``: Return the name of the event
   identified by ``gid`` and the index in the list of events
-  ``pylikwid.nvgetnameofcounter(gid, eidx)``: Return the name of the
   counter register identified by ``gid`` and the index in the list of
   events
-  ``pylikwid.nvgetnameofmetric(gid, midx)``: Return the name of a derived
   metric identified by ``gid`` and the index in the list of metrics
-  ``pylikwid.nvsetup(gid)``: Program the counter registers to measure all
   events in group ``gid``
-  ``pylikwid.nvstart()``: Start the counter registers
-  ``pylikwid.nvstop()``: Stop the counter registers
-  ``pylikwid.nvread()``: Read the counter registers (stop->read->start)
-  ``pylikwid.nvswitch(gid)``: Switch to group ``gid``
   (stop->setup(gid)->start)
-  ``pylikwid.nvgetidofactivegroup()`` Return the ``gid`` of the currently
   configured group
-  ``pylikwid.nvgetresult(gid, eidx, tidx)``: Return the raw counter
   register result of all measurements identified by group ``gid`` and
   the indices for event ``eidx`` and thread ``tidx``
-  ``pylikwid.nvgetlastresult(gid, eidx, tidx)``: Return the raw counter
   register result of the last measurement cycle identified by group
   ``gid`` and the indices for event ``eidx`` and thread ``tidx``
-  ``pylikwid.nvgetmetric(gid, midx, tidx)``: Return the derived metric
   result of all measurements identified by group ``gid`` and the
   indices for metric ``midx`` and thread ``tidx``
-  ``pylikwid.nvgetlastmetric(gid, midx, tidx)``: Return the derived
   metric result of the last measurement cycle identified by group
   ``gid`` and the indices for metric ``midx`` and thread ``tidx``
-  ``pylikwid.nvgettimeofgroup(gid)``: Return the measurement time for
   group identified by ``gid``
-  ``pylikwid.nvfinalize()``: Reset all used registers and delete internal
   measurement results

Nvmon Marker API (if LIKWID is built with Nvidia interface)
-----------------------------------------------------------

-  ``pylikwid.gpumarkerinit()``: Initialize the Nvmon Marker API of the LIKWID library.
   Must be called previous to all other functions.
-  ``rr = pylikwid.gpuregisterregion(regiontag)``: Register a region to the
   Nvmon Marker API. This is an optional function to reduce the overhead of
   region registration at ``pylikwid.markerstartregion``. If you don't call
   ``pylikwid.gpumarkerregisterregion(regiontag)``, the registration is done at
   ``pylikwid.gpumarkerstartregion(regiontag)``. On success, 0 is return. If you
   havn't called ``pylikwid.gpumarkerinit()``, a negative number is returned.
-  ``err = pylikwid.gpumarkerstartregion(regiontag)``: Start measurements under
   the name ``regiontag``. On success, 0 is return. If you havn't called
   ``pylikwid.gpumarkerinit()``, a negative number is returned.
-  ``err = pylikwid.gpumarkerstopregion(regiontag)``: Stop measurements under the
   name ``regiontag`` again. On success, 0 is return. If you havn't
   called ``pylikwid.gpumarkerinit()``, a negative number is returned.
-  ``num_gpus, num_events, events[][], time[], count[] = pylikwid.gpumarkergetregion(regiontag)``:
   Get the intermediate results of the region identified by
   ``regiontag``. On success, it returns the number of events in the
   current group, a list with all the aggregated event results per GPU, the
   measurement time for the region and the number of calls.
-  ``pylikwid.gpunextgroup()``: Switch to the next event set in a
   round-robin fashion. If you have set only one event set on the
   command line, this function performs no operation.
-  ``pylikwid.gpumarkerreset(regiontag)``: Reset the values stored using the region
   name ``regiontag``. On success, 0 is returned.
-  ``pylikwid.gpumarkerclose()``: Close the connection to the LIKWID Nvmon Marker API
   and write out measurement data to file. This file will be evaluated
   by ``likwid-perfctr``.


Usage
=====

Marker API
----------

Code
~~~~

Here is a small example Python script how to use the LIKWID Marker API
in Python:

::

    #!/usr/bin/env python

    import pylikwid

    pylikwid.markerinit()
    pylikwid.markerthreadinit()
    liste = []
    pylikwid.markerstartregion("listappend")
    for i in range(0,1000000):
        liste.append(i)
    pylikwid.markerstopregion("listappend")
    nr_events, eventlist, time, count = pylikwid.markergetregion("listappend")
    for i, e in enumerate(eventlist):
        print(i, e)
    pylikwid.markerclose()

This code simply measures the hardware performance counters for
appending 1000000 elements to a list. First the API is initialized with
``likwid.init()`` and ``likwid.threadinit()``. Afterwards it creates an
empty list, starts the measurements with
``likwid.startregion("listappend")`` and executes the appending loop.
When the loop has finished, we stop the measurements again using
``likwid.stopregion("listappend")``. Just for the example, we get the
values inside our script using ``likwid.getregion("listappend")`` and
print out the results. Finally, we close the connection to the LIKWID
Marker API.

You always have to use ``likwid-perfctr`` to program the hardware
performance counters and specify the CPUs that should be measured. Since
Python is commonly single-threaded, the cpu set only contains one entry:
``likwid-perfctr -C 0 -g <EVENTSET> -m <PYTHONSCRIPT>`` This pins the
Python interpreter to CPU 0 and measures ``<EVENTSET>`` for all regions
in the Python script. You can set multiple event sets by adding multiple
``-g <EVENTSET>`` to the command line. Please see the LIKWID page for
further information how to use ``likwid-perfctr``. Link:
https://github.com/rrze-likwid/likwid

Example
~~~~~~~

Using the above Python script we can measure the L2 to L3 cache data
volume:

::

    $ likwid-perfctr -C 0 -g L3 -m ./test.py
    --------------------------------------------------------------------------------
    CPU name:   Intel(R) Core(TM) i7-4770 CPU @ 3.40GHz
    CPU type:   Intel Core Haswell processor
    CPU clock:  3.39 GHz
    --------------------------------------------------------------------------------
    (0, 926208305.0)
    (1, 325539316.0)
    (2, 284626172.0)
    (3, 1219118.0)
    (4, 918368.0)
    Wrote LIKWID Marker API output to file /tmp/likwid_17275.txt
    --------------------------------------------------------------------------------
    ================================================================================
    Group 1 L3: Region listappend
    ================================================================================
    +-------------------+----------+
    |    Region Info    |  Core 0  |
    +-------------------+----------+
    | RDTSC Runtime [s] | 0.091028 |
    |     call count    |     1    |
    +-------------------+----------+

    +-----------------------+---------+--------------+
    |         Event         | Counter |    Core 0    |
    +-----------------------+---------+--------------+
    |   INSTR_RETIRED_ANY   |  FIXC0  | 9.262083e+08 |
    | CPU_CLK_UNHALTED_CORE |  FIXC1  | 3.255393e+08 |
    |  CPU_CLK_UNHALTED_REF |  FIXC2  | 2.846262e+08 |
    |    L2_LINES_IN_ALL    |   PMC0  | 1.219118e+06 |
    |     L2_TRANS_L2_WB    |   PMC1  | 9.183680e+05 |
    +-----------------------+---------+--------------+

    +-------------------------------+--------------+
    |             Metric            |    Core 0    |
    +-------------------------------+--------------+
    |      Runtime (RDTSC) [s]      |  0.09102752  |
    |      Runtime unhalted [s]     | 9.596737e-02 |
    |          Clock [MHz]          | 3.879792e+03 |
    |              CPI              | 3.514753e-01 |
    |  L3 load bandwidth [MBytes/s] | 8.571425e+02 |
    |  L3 load data volume [GBytes] |  0.078023552 |
    | L3 evict bandwidth [MBytes/s] | 6.456899e+02 |
    | L3 evict data volume [GBytes] |  0.058775552 |
    |    L3 bandwidth [MBytes/s]    | 1.502832e+03 |
    |    L3 data volume [GBytes]    |  0.136799104 |
    +-------------------------------+--------------+

At first a header with the current system type and clock is printed.
Afterwards the output of the Python script lists the results of the
measurements we got internally with ``likwid.getregion``. The next
output is the region results evaluated by ``likwid-perfctr`` and prints
at first a headline stating the measured eventset, here ``L3`` and the
region name ``listappend``. Afterwards 2 or 3 tables are printed. At
first some basic information about the region like run time (or better
measurement time) and the number of calls of the region. The next table
contains the raw values for each event in the eventset. These numbers
are similar to the ones we got internally with ``likwid.getregion``. If
you have set an performance group (here ``L3``) instead of a custom
event set, the raw results are derived to commonly used metrics, here
the ``CPI`` (Cycles per instruction, lower is better) and different
bandwidths and data volumes. You can see, that the load bandwidth for
the small loop is 857 MByte/s and the evict (write) bandwidth is 645
MByte/s. In total we have a bandwidth of 1502 MByte/s.

Full API
--------

Code
~~~~

::

    #!/usr/bin/env python

    import pylikwid

    liste = []
    cpus = [0,1]

    pylikwid.init(cpus)
    group = pylikwid.addeventset("INSTR_RETIRED_ANY:FIXC0")
    pylikwid.setup(group)
    pylikwid.start()
    for i in range(0,1000000):
        liste.append(i)
    pylikwid.stop()
    for thread in range(0,len(cpus)):
        print("Result CPU %d : %f" % (cpus[thread], pylikwid.getresult(group,0,thread)))
    pylikwid.finalize()

Example
~~~~~~~

::

    $ ./test.py
    Result CPU 0 : 87335.000000
    Result CPU 1 : 5222188.000000

Further comments
================

Please be aware that Python is a high-level language and your simple
code is translated to a lot of Assembly instructions. The ``CPI`` value
is commonly low (=> good) for high-level languages because they have to
perform type-checking and similar stuff that can be executed fast in
comparison to the CPU clock. If you would compare the results to a lower
level language like C or Fortran, the ``CPI`` will be worse for them but
the performance will be higher as no type-checking and transformations
need to be done.
