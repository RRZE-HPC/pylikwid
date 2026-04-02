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


def test_getavailgovs(topology):
    govlist = pylikwid.getavailgovs(0)
    assert govlist is not None
    assert len(govlist) > 0


def test_getcpuclock(topology):
    minfreq = pylikwid.getcpuclockmin(0)
    maxfreq = pylikwid.getcpuclockmax(0)
    current = pylikwid.getcpuclockcurrent(0)
    assert minfreq > 0
    assert maxfreq >= minfreq
    assert current > 0


def test_getgovernor(topology):
    gov = pylikwid.getgovernor(0)
    assert isinstance(gov, str)
    assert len(gov) > 0


def test_set_and_reset_frequency(topology):
    minfreq = int(float(pylikwid.getcpuclockmin(1)) / 1e3)
    maxfreq = int(float(pylikwid.getcpuclockmax(1)) / 1e3)

    pylikwid.setcpuclockmin(1, minfreq)
    pylikwid.setcpuclockmax(1, minfreq)
    assert int(float(pylikwid.getcpuclockmin(1)) / 1e3) == minfreq

    # Reset
    pylikwid.setcpuclockmin(1, minfreq)
    pylikwid.setcpuclockmax(1, maxfreq)
    assert int(float(pylikwid.getcpuclockmax(1)) / 1e3) == maxfreq


def test_set_and_reset_governor(topology):
    govlist = pylikwid.getavailgovs(0).split()
    current_gov = pylikwid.getgovernor(1)
    other_gov = next((g for g in govlist if g != current_gov), None)
    if other_gov is None:
        pytest.skip("Only one governor available")

    pylikwid.setgovernor(1, other_gov)
    assert pylikwid.getgovernor(1) == other_gov

    pylikwid.setgovernor(1, current_gov)
    assert pylikwid.getgovernor(1) == current_gov


def test_uncore_frequencies(topology):
    for socket in range(topology["numSockets"]):
        minunc = pylikwid.getuncoreclockmin(socket)
        maxunc = pylikwid.getuncoreclockmax(socket)
        assert minunc > 0
        assert maxunc >= minunc
