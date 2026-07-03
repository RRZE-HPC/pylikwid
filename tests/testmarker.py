import os

import pytest
import pylikwid

pytestmark = pytest.mark.skipif(
    "LIKWID_MODE" not in os.environ,
    reason="Marker API requires running under likwid-perfctr (LIKWID_MODE not set)",
)


def test_marker_region():
    pylikwid.markerinit()
    pylikwid.markerthreadinit()

    liste = []
    pylikwid.markerstartregion("liste")
    for i in range(1_000_000):
        liste.append(i)
    pylikwid.markerstopregion("liste")

    nr_events, elist, time, count = pylikwid.markergetregion("liste")
    assert nr_events >= 0
    assert isinstance(elist, list)
    assert time >= 0
    assert count >= 0

    pylikwid.markerclose()

def test_profile_decorator():
    pylikwid.markerinit()
    pylikwid.markerthreadinit()

    @pylikwid.profile
    def my_work():
        return list(range(100_000))

    result = my_work()
    assert len(result) == 100_000

    nr_events, elist, time, count = pylikwid.markergetregion("my_work")
    assert nr_events >= 0
    assert time >= 0
    assert count >= 0

    pylikwid.markerclose()


def test_profile_decorator_with_custom_region_name():
    pylikwid.markerinit()
    pylikwid.markerthreadinit()

    @pylikwid.profile(region_name="custom")
    def my_work():
        return list(range(100_000))

    result = my_work()
    assert len(result) == 100_000

    nr_events, elist, time, count = pylikwid.markergetregion("custom")
    assert nr_events >= 0
    assert time >= 0
    assert count >= 0

    pylikwid.markerclose()
