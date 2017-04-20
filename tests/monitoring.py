#!/usr/bin/env python

import os, sys, re, time, math

try:
    import pylikwid
except ImportError:
    print("Cannot load LIKWID python module")
    sys.exit(1)

groups = ["L3"]
sleeptime = 1
if len(groups) == 0:
    print("You have to add a performance group name to 'groups' list")
    sys.exit(1)
if sleeptime <= 0:
    print("Sleep time must be greater than zero")
    sys.exit(1)


gids = {}
cpus = []

ret = pylikwid.hpminit()
if not ret:
    print('Failed to initialize access layer for LIKWID')
    sys.exit(1)


ret = pylikwid.inittopology()
if not ret:
    print('Failed to initialize LIKWID topology module')
    sys.exit(1)


topo = pylikwid.getcputopology()
for t in topo["threadPool"].keys():
    cpus.append(topo["threadPool"][t]["apicId"])


ret = pylikwid.init(cpus)
if ret != 0:
    print('Failed to initialize LIKWID perfmon module')
    sys.exit(1)
os.environ["LIKWID_FORCE"] = "1"

if pylikwid.setverbosity(0) != 0:
    print('Failed to set verbosity')
    sys.exit(1)


run = True
try:
    while run:
        for grp in groups:
            if not gids.has_key(grp):
                gid = pylikwid.addeventset(grp)
                if gid < 0:
                    print('Failed to add group {} to LIKWID perfmon module'.format(grp))
                    groups.remove(grp)
                    continue
                gids[grp] = gid
            gid = gids[grp]
            timestamp = time.time()
            pylikwid.setup(gid)
            pylikwid.start()
            time.sleep(sleeptime)
            pylikwid.stop()
            for i in range(len(cpus)):
                for m in range(pylikwid.getnumberofmetrics(gid)):
                    metricname = pylikwid.getnameofmetric(gid, m)
                    v = float(pylikwid.getlastmetric(gid, m, i))
                    if math.isnan(v) or str(v) == "nan":
                        print("Metric {} on CPU {} failed".format(metricname, cpus[i]))
                        run = False
                    elif i == 0:
                        print("{},cpu={} {} {}".format(metricname, cpus[i], v, timestamp))
except KeyboardInterrupt:
    pass

pylikwid.finalize()
