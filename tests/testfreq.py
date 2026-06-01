import os

import pytest
import pylikwid

pytestmark = pytest.mark.skipif(
    os.geteuid() != 0,
    reason="Frequency tests require root privileges",
)


@pytest.fixture(scope="module")
def topology():
    pylikwid.inittopology()
    topo = pylikwid.getcputopology()
    yield topo
    pylikwid.finalizetopology()


def test_getavailfreqs(topology):
    freqlist = pylikwid.getavailfreqs(0)
    assert freqlist is not None
    assert len(freqlist) > 0
    print(f"Available frequencies for CPU core 0:\n{freqlist}\n")


def test_getavailgovs(topology):
    govlist = pylikwid.getavailgovs(0)
    assert govlist is not None
    assert len(govlist) > 0
    print(f"Available CPU governors for CPU core 0:\n{govlist}\n")


def test_getcpuclock(topology):
    minfreq = pylikwid.getcpuclockmin(0)
    maxfreq = pylikwid.getcpuclockmax(0)
    current = pylikwid.getcpuclockcurrent(0)
    assert minfreq > 0
    assert maxfreq >= minfreq
    assert current > 0
    for idx in topology["threadPool"]:
        cpu = topology["threadPool"][idx]["apicId"]
        print(f"CPU {cpu} : {pylikwid.getcpuclockcurrent(cpu)} Hz (min: {pylikwid.getcpuclockmin(cpu)}, max: {pylikwid.getcpuclockmax(cpu)}, gov: {pylikwid.getgovernor(cpu)})")


def test_getgovernor(topology):
    gov = pylikwid.getgovernor(0)
    assert isinstance(gov, str)
    assert len(gov) > 0


def test_set_and_reset_frequency(topology):
    minfreq = int(float(pylikwid.getcpuclockmin(1)) / 1e3)
    maxfreq = int(float(pylikwid.getcpuclockmax(1)) / 1e3)

    print(f"\nSet frequency of CPU 1 to minimum {int(minfreq / 1e3)} MHz:")
    pylikwid.setcpuclockmin(1, minfreq)
    pylikwid.setcpuclockmax(1, minfreq)
    assert int(float(pylikwid.getcpuclockmin(1)) / 1e3) == minfreq
    print(f"CPU 1 : {pylikwid.getcpuclockcurrent(1)} Hz (min: {pylikwid.getcpuclockmin(1)}, max: {pylikwid.getcpuclockmax(1)}, gov: {pylikwid.getgovernor(1)})")

    print("\nReset frequency of CPU 1:")
    pylikwid.setcpuclockmin(1, minfreq)
    pylikwid.setcpuclockmax(1, maxfreq)
    assert int(float(pylikwid.getcpuclockmax(1)) / 1e3) == maxfreq
    print(f"CPU 1 : {pylikwid.getcpuclockcurrent(1)} Hz (min: {pylikwid.getcpuclockmin(1)}, max: {pylikwid.getcpuclockmax(1)}, gov: {pylikwid.getgovernor(1)})")


def test_set_and_reset_governor(topology):
    govlist = pylikwid.getavailgovs(0).split()
    current_gov = pylikwid.getgovernor(1)
    other_gov = next((g for g in govlist if g != current_gov), None)
    if other_gov is None:
        pytest.skip("Only one governor available")

    print(f"\nSet governor of CPU 1 to {other_gov}:")
    pylikwid.setgovernor(1, other_gov)
    assert pylikwid.getgovernor(1) == other_gov
    print(f"CPU 1 : {pylikwid.getcpuclockcurrent(1)} Hz (min: {pylikwid.getcpuclockmin(1)}, max: {pylikwid.getcpuclockmax(1)}, gov: {pylikwid.getgovernor(1)})")

    print(f"\nReset governor of CPU 1 to {current_gov}:")
    pylikwid.setgovernor(1, current_gov)
    assert pylikwid.getgovernor(1) == current_gov
    print(f"CPU 1 : {pylikwid.getcpuclockcurrent(1)} Hz (min: {pylikwid.getcpuclockmin(1)}, max: {pylikwid.getcpuclockmax(1)}, gov: {pylikwid.getgovernor(1)})")


def test_uncore_frequencies(topology):
    print("\nUncore frequencies:")
    for socket in range(topology["numSockets"]):
        minunc = pylikwid.getuncoreclockmin(socket)
        maxunc = pylikwid.getuncoreclockmax(socket)
        assert minunc > 0
        assert maxunc >= minunc
        print(f"Socket {socket} : min: {minunc} MHz, max: {maxunc} MHz")
