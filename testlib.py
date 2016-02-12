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
