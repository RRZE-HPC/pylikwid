# likwid-python-api
Python interface for the Marker API of LIKWID (https://github.com/RRZE-HPC/likwid)

# Installation
Adjust paths and python version in Makefile and type make.
Copy the resulting `likwid.so` somewhere in your `PYTHONPATH` or extend the `PYTHONPATH` by the folder where `likwid.so` resides.

# Functions
After `import likwid` you can call the following functions:

- ` likwid.init()`:
Initialize the Marker API of the LIKWID library. Must be called previous to all other functions.
- `likwid.threadinit()`:
Add the current thread to the Marker API. Since Python is commonly single-threaded simply call it directly after `likwid.init()`
- `rr = likwid.registerregion(regiontag)`:
Register a region to the Marker API. This is an optional function to reduce the overhead of region registration at `likwid.startregion`. If you don't call `likwid.registerregion(regiontag)`, the registration is done at `likwid.startregion(regiontag)`. On success, 0 is return. If you havn't called `likwid.init()`, a negative number is returned.
- `err = likwid.startregion(regiontag)`:
Start measurements under the name `regiontag`. On success, 0 is return. If you havn't called `likwid.init()`, a negative number is returned.
- `err = likwid.stopregion(regiontag)`:
Stop measurements under the name `regiontag` again. On success, 0 is return. If you havn't called likwid.init(), a negative number is returned.
- `num_events, events[], time, count = likwid.getregion(regiontag)`:
Get the intermediate results of the region identified by `regiontag`. On success, it returns the number of events in the current group, a list with all the aggregated event results, the measurement time for the region and the number of calls.
- `likwid.nextgroup()`:
Switch to the next event set in a round-robin fashion. If you have set only one event set on the command line, this function performs no operation.
- `likwid.close()`:
Close the connection to the LIKWID Marker API and write out measurement data to file. This file will be evaluated by `likwid-perfctr`.
- `likwid.getprocessorid()`:
Returns the ID of the currently executing CPU
- `likwid.pinprocess(cpuid)`:
Pins the current process to the CPU given as `cpuid`.
- `likwid.pinthread(cpuid)`:
Pins the current thread to the CPU given as `cpuid`.

# Usage
Here is a small example Python script how to use the LIKWID Marker API in Python:
```
#!/usr/bin/env python

import likwid

likwid.init()
likwid.threadinit()
liste = []
likwid.startregion("listappend")
for i in range(0,1000000):
    liste.append(i)
likwid.stopregion("listappend")
nr_events, eventlist, time, count = likwid.getregion("listappend")
for i, e in enumerate(eventlist):
    print(i, e)
likwid.close()
```
This code simply measures the hardware performance counters for appending 1000000 elements to a list. First the API is initialized with `likwid.init()` and `likwid.threadinit()`. Afterwards it creates an empty list, starts the measurements with `likwid.startregion("listappend")` and executes the appending loop. When the loop has finished, we stop the measurements again using `likwid.stopregion("listappend")`. Just for the example, we get the values inside our script using `likwid.getregion("listappend")` and print out the results. Finally, we close the connection to the LIKWID Marker API.

You always have to use `likwid-perfctr` to program the hardware performance counters and specify the CPUs that should be measured. Since Python is commonly single-threaded, the cpu set only contains one entry:
`likwid-perfctr -C 0 -g <EVENTSET> -m <PYTHONSCRIPT>`
This pins the Python interpreter to CPU 0 and measures `<EVENTSET>` for all regions in the Python script. You can set multiple event sets by adding multiple `-g <EVENTSET>` to the command line. Please see the LIKWID page for further information how to use `likwid-perfctr`. Link: https://github.com/rrze-likwid/likwid

# Example
Using the above Python script we can measure the L2 to L3 cache data volume:
```
$ likwid-perfctr -C 0 -g L3 -m ./test.py
--------------------------------------------------------------------------------
CPU name:	Intel(R) Core(TM) i7-4770 CPU @ 3.40GHz
CPU type:	Intel Core Haswell processor
CPU clock:	3.39 GHz
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
```
At first a header with the current system type and clock is printed. Afterwards the output of the Python script lists the results of the measurements we got internally with `likwid.getregion`. The next output is the region results evaluated by `likwid-perfctr` and prints at first a headline stating the measured eventset, here `L3` and the region name `listappend`. Afterwards 2 or 3 tables are printed. At first some basic information about the region like run time (or better measurement time) and the number of calls of the region. The next table contains the raw values for each event in the eventset. These numbers are similar to the ones we got internally with `likwid.getregion`. If you have set an performance group (here `L3`) instead of a custom event set, the raw results are derived to commonly used metrics, here the `CPI` (Cycles per instruction, lower is better) and different bandwidths and data volumes. You can see, that the load bandwidth for the small loop is 857 MByte/s and the evict (write) bandwidth is 645 MByte/s. In total we have a bandwidth of 1502 MByte/s.

# Further comments
Please be aware that Python is a high-level language and your simple code is translated to a lot of Assembly instructions. The `CPI` value is commonly low (=> good) for high-level languages because they have to perform type-checking and similar stuff that can be executed fast in comparison to the CPU clock. If you would compare the results to a lower level language like C or Fortran, the `CPI` will be worse for them but the performance will be higher as no type-checking and transformations need to be done.
