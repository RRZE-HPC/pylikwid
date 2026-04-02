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
