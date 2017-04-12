#!/usr/bin/env python

import pylikwid

pylikwid.inittopology()
infodict = pylikwid.getcpuinfo()
for k in infodict:
    print "%s: %s" % (str(k), str(infodict[k]),)
topodict = pylikwid.getcputopology()
for k in topodict:
    if not isinstance(topodict[k], dict):
        print "%s: %s" % (str(k), str(topodict[k]),)

pylikwid.finalizetopology()
