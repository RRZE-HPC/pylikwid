import pytest
import pylikwid


@pytest.fixture(scope="module")
def topology():
    pylikwid.inittopology()
    topo = pylikwid.getcputopology()
    yield topo
    pylikwid.finalizetopology()


def test_pinprocess_and_getprocessorid(topology):
    for t in topology["threadPool"]:
        cpu = topology["threadPool"][t]["apicId"]
        pylikwid.pinprocess(cpu)
        assert pylikwid.getprocessorid() == cpu
