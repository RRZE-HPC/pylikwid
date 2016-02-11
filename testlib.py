#!/usr/bin/env python

import likwid

liste = []
cpus = [0,1]

likwid.init(cpus)
group = likwid.addeventset("INSTR_RETIRED_ANY:FIXC0")
likwid.setup(group)
likwid.start()
for i in range(0,1000000):
    liste.append(i)
likwid.stop()
for thread in range(0,len(cpus)):
    print("Result CPU %d : %f" % (cpus[thread], likwid.getresult(group,0,thread)))
likwid.finalize()
