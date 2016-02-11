#!/usr/bin/env python

import likwid


likwid.markerinit()
likwid.markerthreadinit()
liste = []
likwid.markerstartregion("liste")
for i in range(0,1000000):
    liste.append(i)

likwid.markerstopregion("liste")
nr_events, elist, time, count = likwid.markergetregion("liste")
for i, e in enumerate(elist):
    print(i, e)
likwid.markerclose()
