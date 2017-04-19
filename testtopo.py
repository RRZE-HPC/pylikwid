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

print("\nCPU topology:")
print("ID\tCore\tThread\tPackage")
for t in topodict["threadPool"]:
    print("%d\t%d\t%d\t%d" % (topodict["threadPool"][t]["apicId"], topodict["threadPool"][t]["coreId"],topodict["threadPool"][t]["threadId"], topodict["threadPool"][t]["packageId"],))

pylikwid.finalizetopology()
