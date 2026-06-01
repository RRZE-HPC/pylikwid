import pytest
import pylikwid


@pytest.fixture(scope="module")
def topology():
    pylikwid.inittopology()
    topo = pylikwid.getcputopology()
    yield topo
    pylikwid.finalizetopology()


def test_getcpuinfo(topology):
    info = pylikwid.getcpuinfo()
    assert isinstance(info, dict)
    assert len(info) > 0
    for k in info:
        print(f"{k}: {info[k]}")


def test_getcputopology_has_threadpool(topology):
    assert "threadPool" in topology


def test_getcputopology_has_threads(topology):
    assert len(topology["threadPool"]) > 0


def test_threadpool_entries(topology):
    for k in topology:
        if not isinstance(topology[k], dict):
            print(f"{k}: {topology[k]}")
    print()
    print("CPU topology:")
    print("ID\tCore\tThread\tPackage")
    for t in topology["threadPool"]:
        entry = topology["threadPool"][t]
        assert "apicId" in entry
        assert "coreId" in entry
        assert "threadId" in entry
        assert "packageId" in entry
        print(f"{entry['apicId']}\t{entry['coreId']}\t{entry['threadId']}\t{entry['packageId']}")
