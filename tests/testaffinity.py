#!/usr/bin/env python

import pylikwid

def int_join(l, sep):
    if l and len(l) > 0:
        return sep.join([str(x) for x in l])
    return "\"\""

aff = pylikwid.initaffinity()

for k in aff:
    if isinstance(aff[k], int):
        print("{}: {}".format(k, aff[k]))
print("")

for d in aff["domains"]:
    print("Domain {}:".format(aff["domains"][d]["tag"]))
    print("\t"+int_join(aff["domains"][d]["processorList"], " "))
    print("")

for sel in ["S0:0-3", "N:3-1", "1,2,3", "E:N:2:1:2"]:
    l = pylikwid.cpustr_to_cpulist(sel)
    print("CPU string {} results in {}".format(sel, int_join(l, ",")))

pylikwid.finalizeaffinity()
