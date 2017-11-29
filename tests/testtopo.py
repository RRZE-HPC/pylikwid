#!/usr/bin/env python

import pylikwid

pylikwid.inittopology()
infodict = pylikwid.getcpuinfo()
for k in infodict:
    print("{}: {}".format(k, infodict[k]))
topodict = pylikwid.getcputopology()
for k in topodict:
    if not isinstance(topodict[k], dict):
        print("{}: {}".format(k, topodict[k]))
print()
print("CPU topology:")
print("ID\tCore\tThread\tPackage")
for t in topodict["threadPool"]:
    print("{}\t{}\t{}\t{}".format(topodict["threadPool"][t]["apicId"],
                                  topodict["threadPool"][t]["coreId"],
                                  topodict["threadPool"][t]["threadId"],
                                  topodict["threadPool"][t]["packageId"]))

pylikwid.finalizetopology()
