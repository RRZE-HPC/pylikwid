#!/usr/bin/env python

import pylikwid


pylikwid.markerinit()
pylikwid.markerthreadinit()
liste = []
pylikwid.markerstartregion("liste")
for i in range(0,1000000):
    liste.append(i)

pylikwid.markerstopregion("liste")
nr_events, elist, time, count = pylikwid.markergetregion("liste")
for i, e in enumerate(elist):
    print(i, e)
pylikwid.markerclose()
