#!/usr/bin/env python

import pylikwid

def do_work():
    l = []
    for i in range(1000000):
        l += [i]
    print("Running on CPU {}".format(pylikwid.getprocessorid()))


pylikwid.inittopology()
cputopo = pylikwid.getcputopology()
for t in cputopo["threadPool"]:
    pylikwid.pinprocess(cputopo["threadPool"][t]["apicId"])
    do_work()

pylikwid.finalizetopology()
