#!/usr/bin/env python

import pylikwid


pylikwid.inittopology()
topodict = pylikwid.getcputopology()
freqlist = pylikwid.getavailfreqs(0)
govlist = pylikwid.getavailgovs(0)
print("Available frequencies for CPU core 0:\n{}\n".format(freqlist))
minfreq = float(pylikwid.getcpuclockmin(0))
maxfreq = float(pylikwid.getcpuclockmax(0))
print("Available CPU governors for CPU core 0:\n{}\n".format(govlist))



for idx in topodict["threadPool"]:
    cpu = topodict["threadPool"][idx]["apicId"]
    print("CPU {} : {} Hz (min: {}, max: {}, gov: {})".format(cpu, pylikwid.getcpuclockcurrent(cpu),
                                                          pylikwid.getcpuclockmin(cpu),
                                                          pylikwid.getcpuclockmax(cpu),
                                                          pylikwid.getgovernor(cpu)))
minunc = int(pylikwid.getuncoreclockmin(0)/1E6)
maxunc = int(pylikwid.getuncoreclockmax(0)/1E6)
print("\nUncore frequencies:")
for socket in range(topodict["numSockets"]):
    print("Socket {} : min: {} MHz, max: {} MHz".format(socket, pylikwid.getuncoreclockmin(socket),
                                                                pylikwid.getuncoreclockmax(socket)))

print("\nSet frequency of CPU 1 to minimum {} MHz:".format(int(float(minfreq)/1E6)))
pylikwid.setcpuclockmin(1, int(float(minfreq)/1E3))
pylikwid.setcpuclockmax(1, int(float(minfreq)/1E3))
print("CPU {} : {} Hz (min: {}, max: {}, gov: {})".format(1, pylikwid.getcpuclockcurrent(1),
                                                          pylikwid.getcpuclockmin(1),
                                                          pylikwid.getcpuclockmax(1),
                                                          pylikwid.getgovernor(1)))

print("\nReset frequency of CPU 1:")
pylikwid.setcpuclockmin(1, int(float(minfreq)/1E3))
pylikwid.setcpuclockmax(1, int(float(maxfreq)/1E3))
print("CPU {} : {} Hz (min: {}, max: {}, gov: {})".format(1, pylikwid.getcpuclockcurrent(1),
                                                          pylikwid.getcpuclockmin(1),
                                                          pylikwid.getcpuclockmax(1),
                                                          pylikwid.getgovernor(1)))

gov = pylikwid.getgovernor(1)
oldgov = gov
for g in govlist.split(" "):
    if g != gov:
        gov = g
        break

print("\nSet governor of CPU 1 to {}:".format(gov))
pylikwid.setgovernor(1, gov)
print("CPU {} : {} Hz (min: {}, max: {}, gov: {})".format(1, pylikwid.getcpuclockcurrent(1),
                                                          pylikwid.getcpuclockmin(1),
                                                          pylikwid.getcpuclockmax(1),
                                                          pylikwid.getgovernor(1)))
print("\nReset governor of CPU 1 to {}:".format(oldgov))
pylikwid.setgovernor(1, oldgov)
print("CPU {} : {} Hz (min: {}, max: {}, gov: {})".format(1, pylikwid.getcpuclockcurrent(1),
                                                          pylikwid.getcpuclockmin(1),
                                                          pylikwid.getcpuclockmax(1),
                                                          pylikwid.getgovernor(1)))

print("\nSet Uncore frequency of socket 0 to minimum {}:".format(minunc))
pylikwid.setuncoreclockmin(0, minunc)
pylikwid.setuncoreclockmax(0, minunc)
print("Socket {} : min: {} MHz, max: {} MHz".format(0, pylikwid.getuncoreclockmin(0),
                                                       pylikwid.getuncoreclockmax(0)))

print("\nReset Uncore frequency of socket 0:")
pylikwid.setuncoreclockmin(0, minunc)
pylikwid.setuncoreclockmax(0, maxunc)
print("Socket {} : min: {} MHz, max: {} MHz".format(0, pylikwid.getuncoreclockmin(0),
                                                       pylikwid.getuncoreclockmax(0)))


pylikwid.finalizetopology()
