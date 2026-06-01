import os

import pytest
import pylikwid

pytestmark = pytest.mark.skipif(
    not os.path.exists("/dev/cpu/0/msr"),
    reason="MSR device not available (run: sudo modprobe msr)",
)

CPUS = [0, 1]
EVENTSET = "INSTR_RETIRED_ANY:FIXC0"


@pytest.fixture(scope="module")
def perfmon():
    err = pylikwid.init(CPUS)
    if err != 0:
        pytest.skip(
            f"LIKWID perfmon init failed (err={err}); "
            "requires root or likwid-accessD with appropriate permissions"
        )
    yield
    pylikwid.finalize()


def test_addeventset(perfmon):
    gid = pylikwid.addeventset(EVENTSET)
    if gid < 0:
        pytest.skip(f"Event set {EVENTSET!r} not supported on this architecture")
    print(f"Eventset {EVENTSET} added with ID {gid}")


def test_measurement_cycle(perfmon):
    gid = pylikwid.addeventset(EVENTSET)
    if gid < 0:
        pytest.skip(f"Event set {EVENTSET!r} not supported on this architecture")

    assert pylikwid.setup(gid) >= 0
    assert pylikwid.start() >= 0

    result = list(range(1_000_000))

    assert pylikwid.stop() >= 0

    for thread in range(len(CPUS)):
        val = pylikwid.getresult(gid, 0, thread)
        assert val >= 0
        print(f"Result CPU {CPUS[thread]} : {val}")
