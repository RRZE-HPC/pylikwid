import pytest
import pylikwid


@pytest.fixture(scope="module")
def affinity():
    aff = pylikwid.initaffinity()
    yield aff
    pylikwid.finalizeaffinity()


def test_initaffinity_returns_dict(affinity):
    assert isinstance(affinity, dict)
    for k in affinity:
        if isinstance(affinity[k], int):
            print(f"{k}: {affinity[k]}")


def test_affinity_has_domains(affinity):
    assert "domains" in affinity
    assert len(affinity["domains"]) > 0


def test_affinity_domain_entries(affinity):
    for d in affinity["domains"]:
        domain = affinity["domains"][d]
        assert "tag" in domain
        assert "processorList" in domain
        print(f"Domain {domain['tag']}:")
        print("\t" + " ".join(str(x) for x in domain["processorList"]))
        print()


@pytest.mark.parametrize("sel", ["S0:0-3", "N:3-1", "1,2,3", "E:N:2:1:2"])
def test_cpustr_to_cpulist(affinity, sel):
    result = pylikwid.cpustr_to_cpulist(sel)
    assert isinstance(result, list)
    assert len(result) > 0
    print(f"CPU string {sel} results in {','.join(str(x) for x in result)}")
