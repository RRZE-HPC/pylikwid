
#include <Python.h>

#include <likwid.h>

#define PYSTR(str) (Py_BuildValue("s", str))
#define PYINT(val) (Py_BuildValue("i", val))
#define PYUINT(val) (Py_BuildValue("I", val))

#ifndef NAN
#define NAN (0.0/0.0)
#endif

static int access_initialized = 0;
static int topo_initialized = 0;
CpuInfo_t cpuinfo = NULL;
CpuTopology_t cputopo = NULL;
static int config_initialized = 0;
Configuration_t configfile = NULL;
static int numa_initialized = 0;
NumaTopology_t numainfo = NULL;
static int affinity_initialized = 0;
AffinityDomains_t affinity = NULL;
static int power_initialized = 0;
PowerInfo_t power;
static int timer_initialized = 0;
static int perfmon_initialized = 0;

static PyObject *
likwid_lversion(PyObject *self, PyObject *args)
{
    int v = 0, r = 0, m = 0;
#ifdef LIKWID_MAJOR
    v = LIKWID_MAJOR;
    r = LIKWID_RELEASE;
    m = LIKWID_MINOR;
#else
    v = 4;
    r = 3;
    m = 4;
#endif
    return Py_BuildValue("iii", v, r, m);
}

/*
################################################################################
Compatibility definitions for nvmon naming scheme changes introduced in 5.3.
################################################################################
*/

#if defined LIKWID_NVMON && LIKWID_MAJOR == 5 && LIKWID_RELEASE >= 3
// Functions that were renamed.
#define gpustr_to_gpulist gpustr_to_gpulist_cuda
#define topology_gpu_init topology_cuda_init
#define topology_gpu_finalize topology_cuda_finalize
#define get_gpuTopology get_cudaTopology
#define likwid_gpuMarkerInit nvmon_markerInit
#define likwid_gpuMarkerNextGroup nvmon_markerNextGroup
#define likwid_gpuMarkerClose nvmon_markerClose
#define likwid_gpuMarkerStartRegion nvmon_markerStartRegion
#define likwid_gpuMarkerRegisterRegion nvmon_markerRegisterRegion
#define likwid_gpuMarkerStartRegion nvmon_markerStartRegion
#define likwid_gpuMarkerStopRegion nvmon_markerStopRegion
#define likwid_gpuMarkerResetRegion nvmon_markerResetRegion
#define likwid_gpuMarkerGetRegion nvmon_markerGetRegion
// Structs that were renamed.
#define GpuDevice CudaDevice
#define GpuTopology_t CudaTopology_t
#endif

/*
################################################################################
# Marker API related functions
################################################################################
*/

static PyObject *
likwid_markerinit(PyObject *self, PyObject *args)
{
    likwid_markerInit();
    Py_RETURN_NONE;
}

static PyObject *
likwid_markerthreadinit(PyObject *self, PyObject *args)
{
    likwid_markerThreadInit();
    Py_RETURN_NONE;
}

static PyObject *
likwid_markerregisterregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerRegisterRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_markerstartregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerStartRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_markerstopregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerStopRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_markergetregion(PyObject *self, PyObject *args)
{
    int i;
    int currentGroup = 0;
    const char *regiontag = NULL;
    int nr_events = 0;
    double* events = NULL;
    double time = 0;
    int count = 0;
    Py_ssize_t pyLen = 0;
    PyObject *pyList;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;
    currentGroup = perfmon_getIdOfActiveGroup();
    nr_events = perfmon_getNumberOfEvents(currentGroup);
    events = (double*) malloc(nr_events * sizeof(double));
    if (events == NULL)
    {
        return NULL;
    }
    for (i = 0; i < nr_events; i++)
    {
        events[i] = 0.0;
    }
    pyLen = (Py_ssize_t)nr_events;
    pyList = PyList_New(pyLen);
    likwid_markerGetRegion(regiontag, &nr_events, events, &time, &count);
    for (i=0; i< nr_events; i++)
    {
        PyList_SET_ITEM(pyList, (Py_ssize_t)i, Py_BuildValue("d", events[i]));
    }
    free(events);
    return Py_BuildValue("iOdi", nr_events, pyList, time, count);
}

static PyObject *
likwid_markernextgroup(PyObject *self, PyObject *args)
{
    likwid_markerNextGroup();
    Py_RETURN_NONE;
}

static PyObject *
likwid_markerresetregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerResetRegion(regiontag);
    return Py_BuildValue("i", ret);
}


static PyObject *
likwid_markerclose(PyObject *self, PyObject *args)
{
    likwid_markerClose();
    Py_RETURN_NONE;
}


static PyObject *
likwid_getprocessorid(PyObject *self, PyObject *args)
{
    return Py_BuildValue("i", likwid_getProcessorId());
}

static PyObject *
likwid_pinprocess(PyObject *self, PyObject *args)
{
    int cpuid, ret;
    if (!PyArg_ParseTuple(args, "i", &cpuid))
        return NULL;
    ret = likwid_pinProcess(cpuid);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_pinthread(PyObject *self, PyObject *args)
{
    int cpuid, ret;
    if (!PyArg_ParseTuple(args, "i", &cpuid))
        return NULL;
    ret = likwid_pinThread(cpuid);
    return Py_BuildValue("i", ret);
}

/*
################################################################################
# Misc functions
################################################################################
*/

static PyObject *
likwid_setverbosity(PyObject *self, PyObject *args)
{
    int verbosity;
    if (!PyArg_ParseTuple(args, "i", &verbosity))
        return NULL;
    if (verbosity >= DEBUGLEV_ONLY_ERROR && verbosity <= DEBUGLEV_DEVELOP)
    {
        perfmon_setVerbosity(verbosity);
        return Py_BuildValue("i", verbosity);
    }
    return Py_BuildValue("i", -1);
}

/*
################################################################################
# Access client related functions
################################################################################
*/

static PyObject *
likwid_hpmmode(PyObject *self, PyObject *args)
{
    int mode;
    if (!PyArg_ParseTuple(args, "i", &mode))
        return Py_False;
    if ((mode == ACCESSMODE_DIRECT) || (mode == ACCESSMODE_DAEMON))
    {
        HPMmode(mode);
        return Py_True;
    }
    return Py_False;
}

static PyObject *
likwid_hpminit(PyObject *self, PyObject *args)
{
    int err = HPMinit();
    if (err == 0)
    {
        access_initialized = 1;
        return Py_True;
    }
    return Py_False;
}

static PyObject *
likwid_hpmaddthread(PyObject *self, PyObject *args)
{
    int cpuid, ret;
    if (!PyArg_ParseTuple(args, "i", &cpuid))
        return Py_BuildValue("i", -1);
    if (access_initialized == 0)
    {
        return Py_BuildValue("i", -1);
    }
    ret = HPMaddThread(cpuid);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_hpmfinalize(PyObject *self, PyObject *args)
{
    HPMfinalize();
    access_initialized = 0;
    Py_RETURN_NONE;
}

/*
################################################################################
# Config file related functions
################################################################################
*/

static PyObject *
likwid_initconfiguration(PyObject *self, PyObject *args)
{
    if (config_initialized)
        return Py_True;
    int ret = init_configuration();
    if (ret == 0)
    {
        config_initialized = 1;
        return Py_True;
    }
    return Py_False;
}

static PyObject *
likwid_destroyconfiguration(PyObject *self, PyObject *args)
{
    if (!config_initialized)
        return Py_False;
    int ret = destroy_configuration();
    if (ret == 0)
    {
        config_initialized = 0;
        return Py_True;
    }
    return Py_False;
}

static PyObject *
likwid_getconfiguration(PyObject *self, PyObject *args)
{
    PyObject *d = PyDict_New();
    if (!config_initialized)
    {
        int ret = init_configuration();
        if (ret == 0)
        {
            config_initialized = 1;
            configfile = get_configuration();
        }
    }
    if ((config_initialized) && (configfile == NULL))
    {
        configfile = get_configuration();
    }
    PyDict_SetItem(d, PYSTR("configFileName"), PYSTR(configfile->configFileName));
    PyDict_SetItem(d, PYSTR("topologyCfgFileName"), PYSTR(configfile->topologyCfgFileName));
    PyDict_SetItem(d, PYSTR("daemonPath"), PYSTR(configfile->daemonPath));
    PyDict_SetItem(d, PYSTR("groupPath"), PYSTR(configfile->groupPath));
    PyDict_SetItem(d, PYSTR("daemonMode"), PYINT(configfile->daemonMode));
    PyDict_SetItem(d, PYSTR("maxNumThreads"), PYINT(configfile->maxNumThreads));
    PyDict_SetItem(d, PYSTR("maxNumNodes"), PYINT(configfile->maxNumNodes));
    return d;
}

static PyObject *
likwid_setgrouppath(PyObject *self, PyObject *args)
{
    int ret = 0;
    char* grouppath;
    if (!PyArg_ParseTuple(args, "s", &grouppath))
        return Py_False;
    ret = config_setGroupPath(grouppath);
    if (ret == 0)
    {
        return Py_True;
    }
    return Py_False;
}

/*
################################################################################
# CPU topology related functions
################################################################################
*/

static PyObject *
likwid_inittopology(PyObject *self, PyObject *args)
{
    int ret = topology_init();
    if (ret == 0)
    {
        topo_initialized = 1;
        return Py_True;
    }
    return Py_False;
}

static PyObject *
likwid_finalizetopology(PyObject *self, PyObject *args)
{
    topology_finalize();
    topo_initialized = 0;
    cputopo = NULL;
    cpuinfo = NULL;
    Py_RETURN_NONE;
}

static PyObject *
likwid_getcputopology(PyObject *self, PyObject *args)
{
    int i, ret;
    PyObject *d = PyDict_New();
    if (!topo_initialized)
    {
        ret = topology_init();
        if (ret == 0)
        {
            topo_initialized = 1;
        }
        else
        {
            return d;
        }
    }
    PyObject *threads = PyDict_New();
    PyObject *caches = PyDict_New();
    PyObject *tmp;
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (!numa_initialized)
    {
        if (numa_init() == 0)
        {
            numa_initialized = 1;
            numainfo = get_numaTopology();
        }
    }
    if ((numa_initialized) && (numainfo == NULL))
    {
        numainfo = get_numaTopology();
    }
    PyDict_SetItem(d, PYSTR("numHWThreads"), PYINT(cputopo->numHWThreads));
    PyDict_SetItem(d, PYSTR("activeHWThreads"), PYINT(cputopo->activeHWThreads));
    PyDict_SetItem(d, PYSTR("numSockets"), PYINT(cputopo->numSockets));
    PyDict_SetItem(d, PYSTR("numCoresPerSocket"), PYINT(cputopo->numCoresPerSocket));
    PyDict_SetItem(d, PYSTR("numThreadsPerCore"), PYINT(cputopo->numThreadsPerCore));
    PyDict_SetItem(d, PYSTR("numCacheLevels"), PYINT(cputopo->numCacheLevels));
    for (i = 0; i < (int)cputopo->numHWThreads; i++)
    {
        tmp = PyDict_New();
        PyDict_SetItem(tmp, PYSTR("threadId"), PYUINT(cputopo->threadPool[i].threadId));
        PyDict_SetItem(tmp, PYSTR("coreId"), PYUINT(cputopo->threadPool[i].coreId));
        PyDict_SetItem(tmp, PYSTR("packageId"), PYUINT(cputopo->threadPool[i].packageId));
        PyDict_SetItem(tmp, PYSTR("apicId"), PYUINT(cputopo->threadPool[i].apicId));
        PyDict_SetItem(threads, PYINT(i), tmp);
    }
    PyDict_SetItem(d, PYSTR("threadPool"), threads);
    for (i = 0; i < (int)cputopo->numCacheLevels; i++)
    {
        tmp = PyDict_New();
        PyDict_SetItem(tmp, PYSTR("level"), PYUINT(cputopo->cacheLevels[i].level));
        PyDict_SetItem(tmp, PYSTR("associativity"), PYUINT(cputopo->cacheLevels[i].associativity));
        PyDict_SetItem(tmp, PYSTR("sets"), PYUINT(cputopo->cacheLevels[i].sets));
        PyDict_SetItem(tmp, PYSTR("lineSize"), PYUINT(cputopo->cacheLevels[i].lineSize));
        PyDict_SetItem(tmp, PYSTR("size"), PYUINT(cputopo->cacheLevels[i].size));
        PyDict_SetItem(tmp, PYSTR("threads"), PYUINT(cputopo->cacheLevels[i].threads));
        PyDict_SetItem(tmp, PYSTR("inclusive"), PYUINT(cputopo->cacheLevels[i].inclusive));
        switch(cputopo->cacheLevels[i].type)
        {
            case DATACACHE:
                PyDict_SetItem(tmp, PYSTR("type"), PYSTR("data"));
                break;
            case INSTRUCTIONCACHE:
                PyDict_SetItem(tmp, PYSTR("type"), PYSTR("instruction"));
                break;
            case UNIFIEDCACHE:
                PyDict_SetItem(tmp, PYSTR("type"), PYSTR("unified"));
                break;
            case ITLB:
                PyDict_SetItem(tmp, PYSTR("type"), PYSTR("itlb"));
                break;
            case DTLB:
                PyDict_SetItem(tmp, PYSTR("type"), PYSTR("dtlb"));
                break;
            case NOCACHE:
                break;
        }
        PyDict_SetItem(caches, PYUINT(cputopo->cacheLevels[i].level), tmp);
    }
    PyDict_SetItem(d, PYSTR("cacheLevels"), caches);
    return d;
}

static PyObject *
likwid_getcpuinfo(PyObject *self, PyObject *args)
{
    int ret;
    PyObject *d = PyDict_New();
    if (!topo_initialized)
    {
        ret = topology_init();
        if (ret == 0)
        {
            topo_initialized = 1;
        }
        else
        {
            return d;
        }
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (!numa_initialized)
    {
        if (numa_init() == 0)
        {
            numa_initialized = 1;
            numainfo = get_numaTopology();
        }
    }
    if ((numa_initialized) && (numainfo == NULL))
    {
        numainfo = get_numaTopology();
    }
    CpuInfo_t info = get_cpuInfo();
    PyDict_SetItem(d, PYSTR("family"), PYUINT(info->family));
    PyDict_SetItem(d, PYSTR("model"), PYUINT(info->model));
    PyDict_SetItem(d, PYSTR("stepping"), PYUINT(info->stepping));
    PyDict_SetItem(d, PYSTR("clock"), Py_BuildValue("k", info->clock));
    if (info->turbo)
    {
        PyDict_SetItem(d, PYSTR("turbo"), Py_True);
    }
    else
    {
        PyDict_SetItem(d, PYSTR("turbo"), Py_False);
    }
    if (info->isIntel)
    {
        PyDict_SetItem(d, PYSTR("isIntel"), Py_True);
    }
    else
    {
        PyDict_SetItem(d, PYSTR("isIntel"), Py_False);
    }
    if (info->supportUncore)
    {
        PyDict_SetItem(d, PYSTR("supportUncore"), Py_True);
    }
    else
    {
        PyDict_SetItem(d, PYSTR("supportUncore"), Py_False);
    }
    PyDict_SetItem(d, PYSTR("osname"), PYSTR(info->osname));
    PyDict_SetItem(d, PYSTR("name"), PYSTR(info->name));
    PyDict_SetItem(d, PYSTR("short_name"), PYSTR(info->short_name));
    PyDict_SetItem(d, PYSTR("features"), PYSTR(info->features));
    PyDict_SetItem(d, PYSTR("featureFlags"), PYUINT(info->featureFlags));
    PyDict_SetItem(d, PYSTR("perf_version"), PYUINT(info->perf_version));
    PyDict_SetItem(d, PYSTR("perf_num_ctr"), PYUINT(info->perf_num_ctr));
    PyDict_SetItem(d, PYSTR("perf_width_ctr"), PYUINT(info->perf_width_ctr));
    PyDict_SetItem(d, PYSTR("perf_num_fixed_ctr"), PYUINT(info->perf_num_fixed_ctr));
#if (LIKWID_MAJOR == 5)
    PyDict_SetItem(d, PYSTR("architecture"), PYSTR(info->architecture));
#endif
    return d;
}


static PyObject *
likwid_printsupportedcpus(PyObject *self, PyObject *args)
{
    print_supportedCPUs();
    Py_RETURN_NONE;
}

/*
################################################################################
# NUMA related functions
################################################################################
*/

static PyObject *
likwid_initnuma(PyObject *self, PyObject *args)
{
    int i,j;
    if (!topo_initialized)
    {
        topology_init();
        topo_initialized = 1;
        cpuinfo = get_cpuInfo();
        cputopo = get_cpuTopology();
    }
    if ((topo_initialized) && (cpuinfo == NULL))
    {
        cpuinfo = get_cpuInfo();
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (numa_initialized == 0)
    {
        if (numa_init() == 0)
        {
            numa_initialized = 1;
            numainfo = get_numaTopology();
        }
        else
        {
            PyObject *d = PyDict_New();
            PyDict_SetItem(d, PYSTR("numberOfNodes"), PYINT(0));
            PyDict_SetItem(d, PYSTR("nodes"), PyDict_New());
            return d;
        }
    }
    if ((numa_initialized) && (numainfo == NULL))
    {
        numainfo = get_numaTopology();
    }
    if (affinity_initialized == 0)
    {
        affinity_init();
        affinity_initialized = 1;
        affinity = get_affinityDomains();
    }
    if ((affinity_initialized) && (affinity == NULL))
    {
        affinity = get_affinityDomains();
    }
    PyObject *d = PyDict_New();
    PyObject *nodes = PyDict_New();
    PyDict_SetItem(d, PYSTR("numberOfNodes"), PYINT(numainfo->numberOfNodes));
    for(i = 0;i < (int)numainfo->numberOfNodes; i++)
    {
        PyObject *n = PyDict_New();
        PyDict_SetItem(n, PYSTR("id"), PYINT(numainfo->nodes[i].id));
        PyDict_SetItem(n, PYSTR("totalMemory"), PYINT(numainfo->nodes[i].totalMemory));
        PyDict_SetItem(n, PYSTR("freeMemory"), PYINT(numainfo->nodes[i].freeMemory));
        PyDict_SetItem(n, PYSTR("numberOfProcessors"), PYINT(numainfo->nodes[i].numberOfProcessors));
        PyDict_SetItem(n, PYSTR("numberOfDistances"), PYINT(numainfo->nodes[i].numberOfDistances));
        PyObject *l = PyList_New(numainfo->nodes[i].numberOfProcessors);
        for(j = 0; j < (int)numainfo->nodes[i].numberOfProcessors; j++)
        {
            PyList_SET_ITEM(l, (Py_ssize_t)j, PYINT(numainfo->nodes[i].processors[j]));
        }
        PyDict_SetItem(n, PYSTR("processors"), l);
        PyObject *dist = PyList_New(numainfo->nodes[i].numberOfDistances);
        for(j = 0; j < (int)numainfo->nodes[i].numberOfDistances; j++)
        {
            PyList_SET_ITEM(dist, (Py_ssize_t)j, PYINT(numainfo->nodes[i].distances[j]));
        }
        PyDict_SetItem(n, PYSTR("distances"), dist);
        PyDict_SetItem(nodes, PYINT(i), n);
    }
    PyDict_SetItem(d, PYSTR("nodes"), nodes);
    return d;
}

static PyObject *
likwid_finalizenuma(PyObject *self, PyObject *args)
{
    if (numa_initialized)
    {
        numa_finalize();
        numainfo = NULL;
        numa_initialized = 0;
    }
    Py_RETURN_NONE;
}


/*
################################################################################
# Affinity related functions
################################################################################
*/

static PyObject *
likwid_initaffinity(PyObject *self, PyObject *args)
{
    int i,j;

    if (topo_initialized == 0)
    {
        topology_init();
        topo_initialized = 1;
        cpuinfo = get_cpuInfo();
        cputopo = get_cpuTopology();
    }
    if ((topo_initialized) && (cpuinfo == NULL))
    {
        cpuinfo = get_cpuInfo();
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (numa_initialized == 0)
    {
        if (numa_init() == 0)
        {
            numa_initialized = 1;
            numainfo = get_numaTopology();
        }
    }
    if ((numa_initialized) && (numainfo == NULL))
    {
        numainfo = get_numaTopology();
    }
    if (affinity_initialized == 0)
    {
        affinity_init();
        affinity_initialized = 1;
        affinity = get_affinityDomains();
    }
    if ((affinity_initialized) && (affinity == NULL))
    {
        affinity = get_affinityDomains();
    }
    PyObject *n = PyDict_New();
    if (affinity == NULL)
    {
        return n;
    }
    PyDict_SetItem(n, PYSTR("numberOfAffinityDomains"), PYINT(affinity->numberOfAffinityDomains));
    PyDict_SetItem(n, PYSTR("numberOfSocketDomains"), PYINT(affinity->numberOfSocketDomains));
    PyDict_SetItem(n, PYSTR("numberOfNumaDomains"), PYINT(affinity->numberOfNumaDomains));
    PyDict_SetItem(n, PYSTR("numberOfProcessorsPerSocket"), PYINT(affinity->numberOfProcessorsPerSocket));
    PyDict_SetItem(n, PYSTR("numberOfCacheDomains"), PYINT(affinity->numberOfCacheDomains));
    PyDict_SetItem(n, PYSTR("numberOfCoresPerCache"), PYINT(affinity->numberOfCoresPerCache));
    PyDict_SetItem(n, PYSTR("numberOfProcessorsPerCache"), PYINT(affinity->numberOfProcessorsPerCache));
    PyObject *doms = PyDict_New();
    for(i = 0; i < (int)affinity->numberOfAffinityDomains; i++)
    {
        PyObject *a = PyDict_New();
#if (LIKWID_MAJOR == 5 && LIKWID_RELEASE >= 4)
        PyDict_SetItem(a, PYSTR("tag"), PYSTR(affinity->domains[i].tag));
#else
        PyDict_SetItem(a, PYSTR("tag"), PYSTR(bdata(affinity->domains[i].tag)));
#endif
        PyDict_SetItem(a, PYSTR("numberOfProcessors"), PYINT(affinity->domains[i].numberOfProcessors));
        PyDict_SetItem(a, PYSTR("numberOfCores"), PYINT(affinity->domains[i].numberOfCores));
        PyObject *l = PyList_New(affinity->domains[i].numberOfProcessors);
        for(j = 0;j < (int)affinity->domains[i].numberOfProcessors; j++)
        {
            PyList_SET_ITEM(l, (Py_ssize_t)j, PYINT(affinity->domains[i].processorList[j]));
        }
        PyDict_SetItem(a, PYSTR("processorList"), l);
        PyDict_SetItem(doms, PYINT(i), a);
    }
    PyDict_SetItem(n, PYSTR("domains"), doms);
    return n;
}

static PyObject *
likwid_finalizeaffinity(PyObject *self, PyObject *args)
{
    if (affinity_initialized)
    {
        affinity_finalize();
        affinity_initialized = 0;
        affinity = NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *
likwid_cpustr_to_cpulist(PyObject *self, PyObject *args)
{
    int ret = 0, j = 0;
    const char *cpustr;
    if (!PyArg_ParseTuple(args, "s", &cpustr))
    {
        Py_RETURN_NONE;
    }
    if (configfile == NULL)
    {
        init_configuration();
        configfile = get_configuration();
    }
    int* cpulist = (int*) malloc(configfile->maxNumThreads * sizeof(int));
    if (!cpulist)
    {
        Py_RETURN_NONE;
    }
    ret = cpustr_to_cpulist((char *)cpustr, cpulist, configfile->maxNumThreads);
    if (ret < 0)
    {
        free(cpulist);
        Py_RETURN_NONE;
    }
    PyObject *l = PyList_New(ret);
    for(j=0;j<ret;j++)
    {
        PyList_SET_ITEM(l, (Py_ssize_t)j, PYINT(cpulist[j]));
    }
    free(cpulist);
    return l;
}



#if LIKWID_MAJOR == 5 && defined LIKWID_NVMON
static PyObject *
likwid_gpustr_to_gpulist(PyObject *self, PyObject *args)
{
    int ret = 0, j = 0;
    const char *gpustr;
    if (!PyArg_ParseTuple(args, "s", &gpustr))
    {
        Py_RETURN_NONE;
    }
    if (configfile == NULL)
    {
        init_configuration();
        configfile = get_configuration();
    }
    int* gpulist = (int*) malloc(configfile->maxNumThreads * sizeof(int));
    if (!gpulist)
    {
        Py_RETURN_NONE;
    }
    ret = gpustr_to_gpulist((char *)gpustr, gpulist, configfile->maxNumThreads);
    if (ret < 0)
    {
        free(gpulist);
        Py_RETURN_NONE;
    }
    PyObject *l = PyList_New(ret);
    for(j=0;j<ret;j++)
    {
        PyList_SET_ITEM(l, (Py_ssize_t)j, PYINT(gpulist[j]));
    }
    free(gpulist);
    return l;
}
#endif

/*
################################################################################
# Timer related functions
################################################################################
*/

static PyObject *
likwid_getCpuClock(PyObject *self, PyObject *args)
{
    if (timer_initialized == 0)
    {
        timer_init();
        timer_initialized = 1;
    }
    return PYINT(timer_getCpuClock());
}

static PyObject *
likwid_startClock(PyObject *self, PyObject *args)
{
    TimerData timer;
    if (timer_initialized == 0)
    {
        timer_init();
        timer_initialized = 1;
    }
    timer_start(&timer);
    return Py_BuildValue("K", timer.start.int64);
}

static PyObject *
likwid_stopClock(PyObject *self, PyObject *args)
{
    TimerData timer;
    if (timer_initialized == 0)
    {
        timer_init();
        timer_initialized = 1;
    }
    timer_stop(&timer);
    return Py_BuildValue("K", timer.stop.int64);
}

static PyObject *
likwid_getClockCycles(PyObject *self, PyObject *args)
{
    TimerData timer;
    uint64_t start,stop;
    if (!PyArg_ParseTuple(args, "KK", &start, &stop))
    {
        Py_RETURN_NONE;
    }
    if (timer_initialized == 0)
    {
        timer_init();
        timer_initialized = 1;
    }
    timer.start.int64 = start;
    timer.stop.int64 = stop;
    return Py_BuildValue("K", timer_printCycles(&timer));
}

static PyObject *
likwid_getClock(PyObject *self, PyObject *args)
{
    TimerData timer;
    uint64_t start,stop;
    if (!PyArg_ParseTuple(args, "KK", &start, &stop))
    {
        Py_RETURN_NONE;
    }
    if (timer_initialized == 0)
    {
        timer_init();
        timer_initialized = 1;
    }
    timer.start.int64 = start;
    timer.stop.int64 = stop;
    return Py_BuildValue("d", timer_print(&timer));
}

/*
################################################################################
# Temperature related functions
################################################################################
*/

static PyObject *
likwid_initTemp(PyObject *self, PyObject *args)
{
    int cpuid;
    if (PyArg_ParseTuple(args, "i", &cpuid))
    {
        thermal_init(cpuid);
    }
    Py_RETURN_NONE;
}

static PyObject *
likwid_readTemp(PyObject *self, PyObject *args)
{
    int cpuid;
    unsigned int data = 0;
    if (PyArg_ParseTuple(args, "i", &cpuid))
    {
        thermal_read(cpuid, &data);
    }
    return PYUINT(data);
}

/*
################################################################################
# Power/Energy related functions
################################################################################
*/

static PyObject *
likwid_getPowerInfo(PyObject *self, PyObject *args)
{
    int i;
    int power_hasRAPL = 0;
    if (topo_initialized == 0)
    {
        topology_init();
        topo_initialized = 1;
        cpuinfo = get_cpuInfo();
        cputopo = get_cpuTopology();
    }
    if ((topo_initialized) && (cpuinfo == NULL))
    {
        cpuinfo = get_cpuInfo();
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (power_initialized == 0)
    {
        power_hasRAPL = power_init(0);
        if (power_hasRAPL)
        {
            power_initialized = 1;
            power = get_powerInfo();
        }
        else
        {
            Py_RETURN_NONE;
        }
    }
    PyObject *n = PyDict_New();
    PyDict_SetItem(n, PYSTR("hasRAPL"), PYINT(power_hasRAPL));
    PyDict_SetItem(n, PYSTR("baseFrequency"), Py_BuildValue("d", power->baseFrequency));
    PyDict_SetItem(n, PYSTR("minFrequency"), Py_BuildValue("d", power->minFrequency));
    PyDict_SetItem(n, PYSTR("powerUnit"), Py_BuildValue("d", power->powerUnit));
    PyDict_SetItem(n, PYSTR("timeUnit"), Py_BuildValue("d", power->timeUnit));

    PyObject *l = PyList_New(power->turbo.numSteps);
    for (i=0; i<power->turbo.numSteps; i++)
    {
        PyList_SET_ITEM(l, (Py_ssize_t)i, Py_BuildValue("d", power->turbo.steps[i]));
    }
    PyDict_SetItem(n, PYSTR("turbo"), l);
    PyObject *d = PyDict_New();
    for(i=0;i<NUM_POWER_DOMAINS;i++)
    {
        PyObject *pd = PyDict_New();
        PyDict_SetItem(pd, PYSTR("ID"), Py_BuildValue("I", power->domains[i].type));
        PyDict_SetItem(pd, PYSTR("energyUnit"), Py_BuildValue("d", power->domains[i].energyUnit));
        if (power->domains[i].supportFlags & POWER_DOMAIN_SUPPORT_STATUS)
        {
            PyDict_SetItem(pd, PYSTR("supportStatus"), Py_True);
        }
        else
        {
            PyDict_SetItem(pd, PYSTR("supportStatus"), Py_False);
        }
        if (power->domains[i].supportFlags & POWER_DOMAIN_SUPPORT_PERF)
        {
            PyDict_SetItem(pd, PYSTR("supportPerf"), Py_True);
        }
        else
        {
            PyDict_SetItem(pd, PYSTR("supportPerf"), Py_False);
        }
        if (power->domains[i].supportFlags & POWER_DOMAIN_SUPPORT_POLICY)
        {
            PyDict_SetItem(pd, PYSTR("supportPolicy"), Py_True);
        }
        else
        {
            PyDict_SetItem(pd, PYSTR("supportPolicy"), Py_False);
        }
        if (power->domains[i].supportFlags & POWER_DOMAIN_SUPPORT_LIMIT)
        {
            PyDict_SetItem(pd, PYSTR("supportLimit"), Py_True);
        }
        else
        {
            PyDict_SetItem(pd, PYSTR("supportLimit"), Py_False);
        }
        if (power->domains[i].supportFlags & POWER_DOMAIN_SUPPORT_INFO)
        {
            PyDict_SetItem(pd, PYSTR("supportInfo"), Py_True);
            PyDict_SetItem(pd, PYSTR("tdp"), Py_BuildValue("d", power->domains[i].tdp));
            PyDict_SetItem(pd, PYSTR("minPower"), Py_BuildValue("d", power->domains[i].minPower));
            PyDict_SetItem(pd, PYSTR("maxPower"), Py_BuildValue("d", power->domains[i].maxPower));
            PyDict_SetItem(pd, PYSTR("maxTimeWindow"), Py_BuildValue("d", power->domains[i].maxTimeWindow));
        }
        else
        {
            PyDict_SetItem(pd, PYSTR("supportInfo"), Py_False);
        }
        PyDict_SetItem(d, PYSTR(power_names[i]), pd);
    }
    PyDict_SetItem(n, PYSTR("domains"), d);
    return n;
}


static PyObject *
likwid_putPowerInfo(PyObject *self, PyObject *args)
{
    if (power_initialized)
    {
        power_finalize();
        power_initialized = 0;
        power = NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *
likwid_startPower(PyObject *self, PyObject *args)
{
    PowerData pwrdata;
    pwrdata.before = 0;
    int cpuId;
    PowerType type;
    if (PyArg_ParseTuple(args, "iI", &cpuId, &type))
    {
        pwrdata.domain = type;
        power_start(&pwrdata, cpuId, type);
    }
    return Py_BuildValue("I", pwrdata.before);
}

static PyObject *
likwid_stopPower(PyObject *self, PyObject *args)
{
    PowerData pwrdata;
    pwrdata.after = 0;
    int cpuId;
    PowerType type;
    if (PyArg_ParseTuple(args, "iI", &cpuId, &type))
    {
        pwrdata.domain = type;
        power_stop(&pwrdata, cpuId, type);
    }
    return Py_BuildValue("I", pwrdata.after);
}

static PyObject *
likwid_getPower(PyObject *self, PyObject *args)
{
    PowerData pwrdata;
    double energy = 0.0;
    pwrdata.before = 0;
    pwrdata.after = 0;
    if (PyArg_ParseTuple(args, "III", &pwrdata.before, &pwrdata.after, &pwrdata.domain))
    {
        energy = power_printEnergy(&pwrdata);
    }
    return Py_BuildValue("d", energy);
}

/*
################################################################################
# Perfmon related functions
################################################################################
*/

static PyObject *
likwid_init(PyObject *self, PyObject *args)
{
    int ret, i;
    int nrThreads = 0;
    PyObject * pyList;

    if (topo_initialized == 0)
    {
        topology_init();
        topo_initialized = 1;
        cpuinfo = get_cpuInfo();
        cputopo = get_cpuTopology();
    }
    if ((topo_initialized) && (cpuinfo == NULL))
    {
        cpuinfo = get_cpuInfo();
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    if (numa_initialized == 0)
    {
        numa_init();
        numa_initialized = 1;
        numainfo = get_numaTopology();
    }
    if ((numa_initialized) && (numainfo == NULL))
    {
        numainfo = get_numaTopology();
    }
    ret = PyArg_ParseTuple(args, "O!", &PyList_Type, &pyList);
    if (pyList == NULL || ret == 0)
    {
        printf("No or wrong function argument: List required\n");
        return PYINT(1);
    }
    if (!PyList_Check(pyList))
    {
        printf("Function argument is no list\n");
        return PYINT(1);
    }
    nrThreads = PyList_Size(pyList);
    int * cpulist = malloc(nrThreads * sizeof(int));
    if (!cpulist)
    {
        printf("Cannot allocate space for cpu list\n");
        return PYINT(1);
    }
    for (i=0; i<nrThreads; i++)
    {
        int size = 0;
#if (PY_MAJOR_VERSION == 2)
        size = (int)PyInt_AsSsize_t(PyList_GetItem(pyList, i));
#endif
#if (PY_MAJOR_VERSION == 3)
        size = (int)PyLong_AsSsize_t(PyList_GetItem(pyList, i));
#endif
        cpulist[i] = size;
    }
    if (perfmon_initialized == 0)
    {
        ret = perfmon_init(nrThreads, &(cpulist[0]));
        if (ret != 0)
        {
            free(cpulist);
            printf("Initialization of PerfMon module failed.\n");
            return PYINT(1);
        }
        perfmon_initialized = 1;
        timer_initialized = 1;
    }
    free(cpulist);
    return PYINT(0);
}

static PyObject *
likwid_addEventSet(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        PYINT(-1);
    }
    const char* tmpString;
    int groupId = -1;
    if (PyArg_ParseTuple(args, "s", &tmpString))
    {
        groupId = perfmon_addEventSet((char*)tmpString);
    }
    return PYINT(groupId);
}

static PyObject *
likwid_setupCounters(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        PYINT(-1);
    }
    int groupId, ret = -1;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        ret = perfmon_setupCounters(groupId);
    }
    return PYINT(ret);
}

static PyObject *
likwid_startCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (perfmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = perfmon_startCounters();
    return PYINT(ret);
}

static PyObject *
likwid_stopCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (perfmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = perfmon_stopCounters();
    return PYINT(ret);
}

static PyObject *
likwid_readCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (perfmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = perfmon_readCounters();
    return PYINT(ret);
}

static PyObject *
likwid_readCountersCpu(PyObject *self, PyObject *args)
{
    int ret = -1;
    if (perfmon_initialized > 0 && PyArg_ParseTuple(args, "i", &ret))
    {
        ret = perfmon_readCountersCpu(ret);
    }
    return PYINT(ret);
}

static PyObject *
likwid_readGroupCounters(PyObject *self, PyObject *args)
{
    int ret = -1;
    if (perfmon_initialized > 0 && PyArg_ParseTuple(args, "i", &ret))
    {
        ret = perfmon_readGroupCounters(ret);
    }
    return PYINT(ret);
}

static PyObject *
likwid_readGroupThreadCounters(PyObject *self, PyObject *args)
{
    int ret = -1;
    int thread = 0;
    if (perfmon_initialized > 0 && PyArg_ParseTuple(args, "ii", &ret, &thread))
    {
        ret = perfmon_readGroupThreadCounters(ret, thread);
    }
    return PYINT(ret);
}

static PyObject *
likwid_switchGroup(PyObject *self, PyObject *args)
{
    int ret = -1;
    int newgroup = 0;
    if (perfmon_initialized > 0 && PyArg_ParseTuple(args, "i", &newgroup))
    {
        if (newgroup >= perfmon_getNumberOfGroups())
        {
            newgroup = 0;
        }
        if (newgroup != perfmon_getIdOfActiveGroup())
        {
            ret = perfmon_switchActiveGroup(newgroup);
        }
    }
    return PYINT(ret);
}

static PyObject *
likwid_finalize(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 1)
    {
        perfmon_finalize();
        perfmon_initialized = 0;
    }
    if (affinity_initialized == 1)
    {
        affinity_finalize();
        affinity_initialized = 0;
        affinity = NULL;
    }
    if (numa_initialized == 1)
    {
        numa_finalize();
        numa_initialized = 0;
        numainfo = NULL;
    }
    if (topo_initialized == 1)
    {
        topology_finalize();
        topo_initialized = 0;
        cputopo = NULL;
        cpuinfo = NULL;
    }
    if (config_initialized == 1)
    {
        destroy_configuration();
        config_initialized = 0;
        configfile = NULL;
    }
    return PYINT(0);
}

static PyObject *
likwid_getResult(PyObject *self, PyObject *args)
{
    int g, e, t;
    double result = NAN;
    if (PyArg_ParseTuple(args, "iii", &g, &e, &t))
    {
        result = perfmon_getResult(g, e, t);
    }
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_getLastResult(PyObject *self, PyObject *args)
{
    int g, e, t;
    double result = NAN;
    if (PyArg_ParseTuple(args, "iii", &g, &e, &t))
    {
        result = perfmon_getLastResult(g, e, t);
    }
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_getMetric(PyObject *self, PyObject *args)
{
    int g, m, t;
    double result = NAN;
    if (PyArg_ParseTuple(args, "iii", &g, &m, &t))
    {
        result = perfmon_getMetric(g, m, t);
    }
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_getLastMetric(PyObject *self, PyObject *args)
{
    int g, m, t;
    double result = NAN;
    if (PyArg_ParseTuple(args, "iii", &g, &m, &t))
    {
        result = perfmon_getLastMetric(g, m, t);
    }
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_getNumberOfGroups(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(perfmon_getNumberOfGroups());
}

static PyObject *
likwid_getIdOfActiveGroup(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(perfmon_getIdOfActiveGroup());
}

static PyObject *
likwid_getNumberOfThreads(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(perfmon_getNumberOfThreads());
}

static PyObject *
likwid_getTimeOfGroup(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    double time = 0.0;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        time = perfmon_getTimeOfGroup(groupId);
    }
    return Py_BuildValue("d", time);
}

static PyObject *
likwid_getNumberOfEvents(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    int num_events = 0;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        num_events = perfmon_getNumberOfEvents(groupId);
    }
    return PYINT(num_events);
}

static PyObject *
likwid_getNumberOfMetrics(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    int num_metrics = 0;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        num_metrics = perfmon_getNumberOfMetrics(groupId);
    }
    return PYINT(num_metrics);
}

static PyObject *
likwid_getNameOfEvent(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int g, e;
    char* name = NULL;
    if (PyArg_ParseTuple(args, "ii", &g, &e))
    {
        name = perfmon_getEventName(g,e);
    }
    return PYSTR(name);
}

static PyObject *
likwid_getNameOfCounter(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int g, c;
    char* name = NULL;
    if (PyArg_ParseTuple(args, "ii", &g, &c))
    {
        name = perfmon_getCounterName(g,c);
    }
    return PYSTR(name);
}

static PyObject *
likwid_getNameOfMetric(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int g, m;
    char* name = NULL;
    if (PyArg_ParseTuple(args, "ii", &g, &m))
    {
        name = perfmon_getMetricName(g, m);
    }
    return PYSTR(name);
}

static PyObject *
likwid_getNameOfGroup(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    char* name = NULL;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        name = perfmon_getGroupName(groupId);
    }
    return PYSTR(name);
}

static PyObject *
likwid_getShortInfoOfGroup(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    char* info = NULL;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        info = perfmon_getGroupInfoShort(groupId);
    }
    return PYSTR(info);
}

static PyObject *
likwid_getLongInfoOfGroup(PyObject *self, PyObject *args)
{
    if (perfmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    char* info = NULL;
    if (PyArg_ParseTuple(args, "i", &groupId))
    {
        info = perfmon_getGroupInfoLong(groupId);
    }
    return PYSTR(info);
}

static PyObject *
likwid_getGroups(PyObject *self, PyObject *args)
{
    int i, ret;
    char** tmp, **infos, **longs;
    PyObject *l;
    if (topo_initialized == 0)
    {
        topology_init();
        topo_initialized = 1;
    }
    ret = perfmon_getGroups(&tmp, &infos, &longs);
    if (ret > 0)
    {
        l = PyList_New(ret);
        for(i=0;i<ret;i++)
        {
            PyObject *d = PyDict_New();
            PyDict_SetItem(d, PYSTR("Name"), PYSTR(tmp[i]));
            PyDict_SetItem(d, PYSTR("Info"), PYSTR(infos[i]));
            PyDict_SetItem(d, PYSTR("Long"), PYSTR(longs[i]));
            PyList_SET_ITEM(l, (Py_ssize_t)i, d);
        }
        perfmon_returnGroups(ret, tmp, infos, longs);
    }
    else
    {
        l = PyList_New(0);
    }
    return l;
}

/*
################################################################################
# Perfmon MarkerAPI related functions
################################################################################
*/

static PyObject *
likwid_readMarkerFile(PyObject *self, PyObject *args)
{
    const char* filename;
    if (PyArg_ParseTuple(args, "s", &filename))
    {
        perfmon_readMarkerFile(filename);
    }
    Py_RETURN_NONE;
}

static PyObject *
likwid_markerNumRegions(PyObject *self, PyObject *args)
{
    return PYINT(perfmon_getNumberOfRegions());
}

static PyObject *
likwid_markerRegionGroup(PyObject *self, PyObject *args)
{
    int r = -1;
    if (PyArg_ParseTuple(args, "i", &r))
    {
        r = perfmon_getGroupOfRegion(r);
    }
    return PYINT(r);
}

static PyObject *
likwid_markerRegionTag(PyObject *self, PyObject *args)
{
    int r = -1;
    char* tag = NULL;
    if (PyArg_ParseTuple(args, "i", &r))
    {
        tag = perfmon_getTagOfRegion(r);
    }
    return PYSTR(tag);
}

static PyObject *
likwid_markerRegionEvents(PyObject *self, PyObject *args)
{
    int r = -1;
    if (PyArg_ParseTuple(args, "i", &r))
    {
        r = perfmon_getEventsOfRegion(r);
    }
    return PYINT(r);
}

static PyObject *
likwid_markerRegionThreads(PyObject *self, PyObject *args)
{
    int r = -1;
    if (PyArg_ParseTuple(args, "i", &r))
    {
        r = perfmon_getThreadsOfRegion(r);
    }
    return PYINT(r);
}

static PyObject *
likwid_markerRegionCpulist(PyObject *self, PyObject *args)
{
    int r, ret = 0;
    int* cpulist;
    if (!PyArg_ParseTuple(args, "i", &r))
    {
        return PyList_New(0);
    }
    if (!topo_initialized)
    {
        ret = topology_init();
        if (ret == 0)
        {
            topo_initialized = 1;
        }
        else
        {
            return PyList_New(0);
        }
    }
    if ((topo_initialized) && (cpuinfo == NULL))
    {
        cpuinfo = get_cpuInfo();
    }
    if ((topo_initialized) && (cputopo == NULL))
    {
        cputopo = get_cpuTopology();
    }
    cpulist = (int*)malloc(cputopo->numHWThreads * sizeof(int));
    if (cpulist == NULL)
    {
        return PyList_New(0);
    }
    ret = perfmon_getCpulistOfRegion(r, cputopo->numHWThreads, cpulist);
    PyObject *l = PyList_New(ret);
    for(r=0; r<ret;r++)
    {
        PyList_SET_ITEM(l, (Py_ssize_t)r, PYINT(cpulist[r]));
    }
    free(cpulist);
    return l;
}

static PyObject *
likwid_markerRegionTime(PyObject *self, PyObject *args)
{
    int r, t;
    PyArg_ParseTuple(args, "ii", &r, &t);
    return Py_BuildValue("d", perfmon_getTimeOfRegion(r, t));
}

static PyObject *
likwid_markerRegionCount(PyObject *self, PyObject *args)
{
    int r, t;
    PyArg_ParseTuple(args, "ii", &r, &t);
    return PYINT(perfmon_getCountOfRegion(r, t));
}

static PyObject *
likwid_markerRegionResult(PyObject *self, PyObject *args)
{
    int r, t, e;
    PyArg_ParseTuple(args, "iii", &r, &e, &t);
    return Py_BuildValue("d", perfmon_getResultOfRegionThread(r, e, t));
}

static PyObject *
likwid_markerRegionMetric(PyObject *self, PyObject *args)
{
    int r, t, m;
    PyArg_ParseTuple(args, "iii", &r, &m, &t);
    return Py_BuildValue("d", perfmon_getMetricOfRegionThread(r, m, t));
}

/*
################################################################################
# CPU frequency related functions
################################################################################
*/
#if (LIKWID_MAJOR == 5)
static PyObject *
likwid_freqInit(PyObject *self, PyObject *args)
{
    return PYUINT(freq_init());
}
#endif

static PyObject *
likwid_freqGetCpuClockCurrent(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYUINT(freq_getCpuClockCurrent(c));
}

static PyObject *
likwid_freqGetCpuClockMax(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYUINT(freq_getCpuClockMax(c));
}

#if (LIKWID_MAJOR == 5)
static PyObject *
likwid_freqGetConfCpuClockMax(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYUINT(freq_getConfCpuClockMax(c));
}
#endif

static PyObject *
likwid_freqGetCpuClockMin(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYUINT(freq_getCpuClockMin(c));
}

#if (LIKWID_MAJOR == 5)
static PyObject *
likwid_freqGetConfCpuClockMin(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYUINT(freq_getConfCpuClockMin(c));
}
#endif

static PyObject *
likwid_freqSetCpuClockMax(PyObject *self, PyObject *args)
{
    int c = 0;
    int f = 0;
    PyArg_ParseTuple(args, "ii", &c, &f);
    return PYUINT(freq_setCpuClockMax(c, f));
}

static PyObject *
likwid_freqSetCpuClockMin(PyObject *self, PyObject *args)
{
    int c = 0;
    int f = 0;
    PyArg_ParseTuple(args, "ii", &c, &f);
    return PYUINT(freq_setCpuClockMin(c, f));
}

static PyObject *
likwid_freqGetGovernor(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYSTR(freq_getGovernor(c));
}

static PyObject *
likwid_freqSetGovernor(PyObject *self, PyObject *args)
{
    int c = 0;
    const char* g;
    PyArg_ParseTuple(args, "is", &c, &g);
    return PYINT(freq_setGovernor(c, (char*)g));
}

static PyObject *
likwid_freqGetAvailFreq(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYSTR(freq_getAvailFreq(c));
}

static PyObject *
likwid_freqGetAvailGovs(PyObject *self, PyObject *args)
{
    int c = 0;
    PyArg_ParseTuple(args, "i", &c);
    return PYSTR(freq_getAvailGovs(c));
}

static PyObject *
likwid_freqSetUncoreClockMin(PyObject *self, PyObject *args)
{
    int s = 0;
    int f = 0;
    PyArg_ParseTuple(args, "ii", &s, &f);
    return Py_BuildValue("i", freq_setUncoreFreqMin(s, f));
}

static PyObject *
likwid_freqGetUncoreClockMin(PyObject *self, PyObject *args)
{
    int s = 0;
    PyArg_ParseTuple(args, "i", &s);
    return PYUINT(freq_getUncoreFreqMin(s)*1000000);
}

static PyObject *
likwid_freqSetUncoreClockMax(PyObject *self, PyObject *args)
{
    int s = 0;
    int f = 0;
    PyArg_ParseTuple(args, "ii", &s, &f);
    return Py_BuildValue("i", freq_setUncoreFreqMax(s, f));
}

static PyObject *
likwid_freqGetUncoreClockMax(PyObject *self, PyObject *args)
{
    int s = 0;
    PyArg_ParseTuple(args, "i", &s);
    return PYUINT(freq_getUncoreFreqMax(s)*1000000);
}

#if (LIKWID_MAJOR == 5)
static PyObject *
likwid_freqFinalize(PyObject *self, PyObject *args)
{
    freq_finalize();
    Py_RETURN_NONE;
}
#endif

#if 0
static PyObject *
likwid_freqGetUncoreClockCurrent(PyObject *self, PyObject *args)
{
    int s = 0;
    PyArg_ParseTuple(args, "i", &s);
    return PYUINT(freq_getUncoreFreqCur(s)*1000000);
}
#endif


#if LIKWID_MAJOR == 5 && defined LIKWID_NVMON
static int gpuTopology_initialized = 0;
static GpuTopology_t gputopo = NULL;
static int nvmon_initialized = 0;

static PyObject *
likwid_initgputopology(PyObject *self, PyObject *args)
{
    int ret = 0;
    if (!gpuTopology_initialized)
    {
        ret = topology_gpu_init();
        if (ret == EXIT_SUCCESS)
        {
            gpuTopology_initialized = 1;
        }
    }
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_finalizegputopology(PyObject *self, PyObject *args)
{
    if (gpuTopology_initialized)
    {
        topology_gpu_finalize();
    }
    Py_RETURN_NONE;
}

static PyObject *
likwid_getgputopology(PyObject *self, PyObject *args)
{
    int i, ret;

    if (!gpuTopology_initialized)
    {
        ret = topology_gpu_init();
        if (ret == EXIT_SUCCESS)
        {
            gpuTopology_initialized = 1;
        }
        else
        {
            Py_RETURN_NONE;
        }
    }
    gputopo = get_gpuTopology();
    if (gputopo)
    {
        PyObject *l = PyList_New(gputopo->numDevices);
        for(i = 0; i < gputopo->numDevices; i++)
        {
            GpuDevice* dev = &gputopo->devices[i];
            PyObject *d = PyDict_New();

            PyDict_SetItem(d, PYSTR("devid"), PYUINT(dev->devid));
            PyDict_SetItem(d, PYSTR("numaNode"), PYUINT(dev->numaNode));
            PyDict_SetItem(d, PYSTR("name"), PYSTR(dev->name));
            PyDict_SetItem(d, PYSTR("mem"), PYUINT(dev->mem));
            PyDict_SetItem(d, PYSTR("ccapMajor"), PYUINT(dev->ccapMajor));
            PyDict_SetItem(d, PYSTR("ccapMinor"), PYUINT(dev->ccapMinor));
            PyDict_SetItem(d, PYSTR("maxThreadsPerBlock"), PYUINT(dev->maxThreadsPerBlock));
            PyDict_SetItem(d, PYSTR("sharedMemPerBlock"), PYUINT(dev->sharedMemPerBlock));
            PyDict_SetItem(d, PYSTR("totalConstantMemory"), PYUINT(dev->totalConstantMemory));
            PyDict_SetItem(d, PYSTR("simdWidth"), PYUINT(dev->simdWidth));
            PyDict_SetItem(d, PYSTR("memPitch"), PYUINT(dev->memPitch));
            PyDict_SetItem(d, PYSTR("regsPerBlock"), PYUINT(dev->regsPerBlock));
            PyDict_SetItem(d, PYSTR("clockRatekHz"), PYUINT(dev->clockRatekHz));
            PyDict_SetItem(d, PYSTR("textureAlign"), PYUINT(dev->textureAlign));
            PyDict_SetItem(d, PYSTR("l2Size"), PYUINT(dev->l2Size));
            PyDict_SetItem(d, PYSTR("memClockRatekHz"), PYUINT(dev->memClockRatekHz));
            PyDict_SetItem(d, PYSTR("pciBus"), PYUINT(dev->pciBus));
            PyDict_SetItem(d, PYSTR("pciDev"), PYUINT(dev->pciDev));
            PyDict_SetItem(d, PYSTR("pciDom"), PYUINT(dev->pciDom));
            PyDict_SetItem(d, PYSTR("maxBlockRegs"), PYUINT(dev->maxBlockRegs));
            PyDict_SetItem(d, PYSTR("numMultiProcs"), PYUINT(dev->numMultiProcs));
            PyDict_SetItem(d, PYSTR("maxThreadPerMultiProc"), PYUINT(dev->maxThreadPerMultiProc));
            PyDict_SetItem(d, PYSTR("memBusWidth"), PYUINT(dev->memBusWidth));
            PyDict_SetItem(d, PYSTR("unifiedAddrSpace"), PYUINT(dev->unifiedAddrSpace));
            PyDict_SetItem(d, PYSTR("ecc"), PYUINT(dev->ecc));
            PyDict_SetItem(d, PYSTR("asyncEngines"), PYUINT(dev->asyncEngines));
            PyDict_SetItem(d, PYSTR("mapHostMem"), PYUINT(dev->mapHostMem));
            PyDict_SetItem(d, PYSTR("integrated"), PYUINT(dev->integrated));

            PyObject *maxThreadsDim = PyList_New(3);
            PyList_SET_ITEM(maxThreadsDim, 0, PYUINT(dev->maxThreadsDim[0]));
            PyList_SET_ITEM(maxThreadsDim, 1, PYUINT(dev->maxThreadsDim[1]));
            PyList_SET_ITEM(maxThreadsDim, 2, PYUINT(dev->maxThreadsDim[2]));
            PyDict_SetItem(d, PYSTR("maxThreadsDim"), maxThreadsDim);

            PyObject *maxGridSize = PyList_New(3);
            PyList_SET_ITEM(maxGridSize, 0, PYUINT(dev->maxGridSize[0]));
            PyList_SET_ITEM(maxGridSize, 1, PYUINT(dev->maxGridSize[1]));
            PyList_SET_ITEM(maxGridSize, 2, PYUINT(dev->maxGridSize[2]));
            PyDict_SetItem(d, PYSTR("maxGridSize"), maxGridSize);

            PyList_SET_ITEM(l, (Py_ssize_t)i, d);
        }
        return l;
    }
    Py_RETURN_NONE;
}

/*
################################################################################
# Nvmon Marker API related functions
################################################################################
*/

static PyObject *
likwid_gpumarkerinit(PyObject *self, PyObject *args)
{
    likwid_gpuMarkerInit();
    Py_RETURN_NONE;
}


static PyObject *
likwid_gpumarkerregisterregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_gpuMarkerRegisterRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_gpumarkerstartregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_gpuMarkerStartRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_gpumarkerstopregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_gpuMarkerStopRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_gpumarkergetregion(PyObject *self, PyObject *args)
{
    int i;
    int currentGroup = 0;
    const char *regiontag = NULL;
    int nr_events = 0;
    double* events = NULL;
    double time = 0;
    int count = 0;
    int nr_gpus = 0;
    Py_ssize_t pyLen = 0;
    PyObject *pyList;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;
    currentGroup = nvmon_getIdOfActiveGroup();
    nr_events = nvmon_getNumberOfEvents(currentGroup);
    events = (double*) malloc(nr_events * sizeof(double));
    if (events == NULL)
    {
        return NULL;
    }
    for (i = 0; i < nr_events; i++)
    {
        events[i] = 0.0;
    }
    pyLen = (Py_ssize_t)nr_events;
    pyList = PyList_New(pyLen);
    likwid_gpuMarkerGetRegion(regiontag, &nr_gpus, &nr_events, &events, &time, &count);
    for (i=0; i< nr_events; i++)
    {
        PyList_SET_ITEM(pyList, (Py_ssize_t)i, Py_BuildValue("d", events[i]));
    }
    free(events);
    return Py_BuildValue("iiOdi", nr_gpus, nr_events, pyList, time, count);
}

static PyObject *
likwid_gpumarkernextgroup(PyObject *self, PyObject *args)
{
    likwid_gpuMarkerNextGroup();
    Py_RETURN_NONE;
}

static PyObject *
likwid_gpumarkerresetregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_gpuMarkerResetRegion(regiontag);
    return Py_BuildValue("i", ret);
}


static PyObject *
likwid_gpumarkerclose(PyObject *self, PyObject *args)
{
    likwid_gpuMarkerClose();
    Py_RETURN_NONE;
}


/*
################################################################################
# Nvmon related functions
################################################################################
*/

static PyObject *
likwid_nvmon_init(PyObject *self, PyObject *args)
{
    int ret, i;
    int nrGpus = 0;
    PyObject * pyList;

    if (gpuTopology_initialized == 0)
    {
        topology_gpu_init();
        gpuTopology_initialized = 1;
        gputopo = get_gpuTopology();
    }
    if ((gpuTopology_initialized) && (gputopo == NULL))
    {
        gputopo = get_gpuTopology();
    }

    PyArg_ParseTuple(args, "O!", &PyList_Type, &pyList);
    if (pyList == NULL)
    {
        printf("No function argument\n");
        return PYINT(1);
    }
    if (!PyList_Check(pyList))
    {
        printf("Function argument is no list\n");
        return PYINT(1);
    }
    nrGpus = PyList_Size(pyList);
    int * gpulist = malloc(nrGpus * sizeof(int));
    if (!gpulist)
    {
        printf("Cannot allocate space for gpu list\n");
        return PYINT(1);
    }
    for (i=0; i<nrGpus; i++)
    {
        int size = 0;
#if (PY_MAJOR_VERSION == 2)
        size = (int)PyInt_AsSsize_t(PyList_GetItem(pyList, i));
#endif
#if (PY_MAJOR_VERSION == 3)
        size = (int)PyLong_AsSsize_t(PyList_GetItem(pyList, i));
#endif
        gpulist[i] = size;
    }
    if (nvmon_initialized == 0)
    {
        ret = nvmon_init(nrGpus, &(gpulist[0]));
        if (ret != 0)
        {
            free(gpulist);
            printf("Initialization of PerfMon module failed.\n");
            return PYINT(1);
        }
        nvmon_initialized = 1;
    }
    free(gpulist);
    return PYINT(0);
}

static PyObject *
likwid_nvmon_addEventSet(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    const char* tmpString;
    int groupId;
    PyArg_ParseTuple(args, "s", &tmpString);
    groupId = nvmon_addEventSet((char*)tmpString);
    return PYINT(groupId);
}

static PyObject *
likwid_nvmon_setupCounters(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    int groupId, ret = 0;
    PyArg_ParseTuple(args, "i", &groupId);
    ret = nvmon_setupCounters(groupId);
    return PYINT(ret);
}

static PyObject *
likwid_nvmon_startCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = nvmon_startCounters();
    return PYINT(ret);
}

static PyObject *
likwid_nvmon_stopCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = nvmon_stopCounters();
    return PYINT(ret);
}

static PyObject *
likwid_nvmon_readCounters(PyObject *self, PyObject *args)
{
    int ret;
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    ret = nvmon_readCounters();
    return PYINT(ret);
}

static PyObject *
likwid_nvmon_switchGroup(PyObject *self, PyObject *args)
{
    int ret = 0, newgroup;
    if (nvmon_initialized == 0)
    {
        PYINT(-1);
    }
    PyArg_ParseTuple(args, "i", &newgroup);
    if (newgroup >= nvmon_getNumberOfGroups())
    {
        newgroup = 0;
    }
    if (newgroup == nvmon_getIdOfActiveGroup())
    {
        return PYINT(-1);
    }
    ret = nvmon_switchActiveGroup(newgroup);
    return PYINT(ret);
}

static PyObject *
likwid_nvmon_finalize(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 1)
    {
        nvmon_finalize();
        nvmon_initialized = 0;
    }
    if (gpuTopology_initialized == 1)
    {
        topology_gpu_finalize();
        gpuTopology_initialized = 0;
        gputopo = NULL;
    }
    return PYINT(0);
}

static PyObject *
likwid_nvmon_getResult(PyObject *self, PyObject *args)
{
    int g, e, t;
    double result;
    PyArg_ParseTuple(args, "iii", &g, &e, &t);
    result = nvmon_getResult(g, e, t);
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_nvmon_getLastResult(PyObject *self, PyObject *args)
{
    int g, e, t;
    double result;
    PyArg_ParseTuple(args, "iii", &g, &e, &t);
    result = nvmon_getLastResult(g, e, t);
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_nvmon_getMetric(PyObject *self, PyObject *args)
{
    int g, m, t;
    double result;
    PyArg_ParseTuple(args, "iii", &g, &m, &t);
    result = nvmon_getMetric(g, m, t);
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_nvmon_getLastMetric(PyObject *self, PyObject *args)
{
    int g, m, t;
    double result = 0.0;
    PyArg_ParseTuple(args, "iii", &g, &m, &t);
    result = nvmon_getLastMetric(g, m, t);
    return Py_BuildValue("d", result);
}

static PyObject *
likwid_nvmon_getNumberOfGroups(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(nvmon_getNumberOfGroups());
}

static PyObject *
likwid_nvmon_getIdOfActiveGroup(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(nvmon_getIdOfActiveGroup());
}

static PyObject *
likwid_nvmon_getNumberOfGpus(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    return PYINT(nvmon_getNumberOfGPUs());
}

static PyObject *
likwid_nvmon_getTimeOfGroup(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return Py_BuildValue("d", nvmon_getTimeOfGroup(groupId));
}

static PyObject *
likwid_nvmon_getNumberOfEvents(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return PYINT(nvmon_getNumberOfEvents(groupId));
}

static PyObject *
likwid_nvmon_getNumberOfMetrics(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return PYINT(nvmon_getNumberOfMetrics(groupId));
}

static PyObject *
likwid_nvmon_getNameOfEvent(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int g, e;
    PyArg_ParseTuple(args, "ii", &g, &e);
    return PYSTR(nvmon_getEventName(g,e));
}

static PyObject *
likwid_nvmon_getNameOfCounter(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int g, c;
    PyArg_ParseTuple(args, "ii", &g, &c);
    return PYSTR(nvmon_getCounterName(g,c));
}

static PyObject *
likwid_nvmon_getNameOfMetric(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int g, m;
    PyArg_ParseTuple(args, "ii", &g, &m);
    return PYSTR(nvmon_getMetricName(g,m));
}

static PyObject *
likwid_nvmon_getNameOfGroup(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return PYSTR(nvmon_getGroupName(groupId));
}

static PyObject *
likwid_nvmon_getShortInfoOfGroup(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return PYSTR(nvmon_getGroupInfoShort(groupId));
}

static PyObject *
likwid_nvmon_getLongInfoOfGroup(PyObject *self, PyObject *args)
{
    if (nvmon_initialized == 0)
    {
        return 0;
    }
    int groupId;
    PyArg_ParseTuple(args, "i", &groupId);
    return PYSTR(nvmon_getGroupInfoLong(groupId));
}

static PyObject *
likwid_nvmon_getGroups(PyObject *self, PyObject *args)
{
#if LIKWID_RELEASE >= 1
    // Since 5.1.0, nvmon_getGroups takes the the gpu id as first argument.
    int gpuId, i, ret;
    if (!PyArg_ParseTuple(args, "i", &gpuId))
    {
        Py_RETURN_NONE;
    }
#else
    int i, ret;
#endif
    char** tmp, **infos, **longs;
    PyObject *l;
    if (gpuTopology_initialized == 0)
    {
        topology_gpu_init();
        gpuTopology_initialized = 1;
    }
    if (gpuTopology_initialized && gputopo == NULL)
    {
        gputopo = get_gpuTopology();
    }
#if LIKWID_RELEASE >= 1
    ret = nvmon_getGroups(gpuId, &tmp, &infos, &longs);
#else
    ret = nvmon_getGroups(&tmp, &infos, &longs);
#endif
    if (ret > 0)
    {
        l = PyList_New(ret);
        for(i=0;i<ret;i++)
        {
            PyObject *d = PyDict_New();
            PyDict_SetItem(d, PYSTR("Name"), PYSTR(tmp[i]));
            PyDict_SetItem(d, PYSTR("Info"), PYSTR(infos[i]));
            PyDict_SetItem(d, PYSTR("Long"), PYSTR(longs[i]));
            PyList_SET_ITEM(l, (Py_ssize_t)i, d);
        }
        nvmon_returnGroups(ret, tmp, infos, longs);
    }
    else
    {
        l = PyList_New(0);
    }
    return l;
}

static PyObject *
likwid_nvmon_setverbosity(PyObject *self, PyObject *args)
{
    int verbosity;
    if (!PyArg_ParseTuple(args, "i", &verbosity))
        return NULL;
    if (verbosity >= DEBUGLEV_ONLY_ERROR && verbosity <= DEBUGLEV_DEVELOP)
    {
        nvmon_setVerbosity(verbosity);
        return Py_BuildValue("i", verbosity);
    }
    return Py_BuildValue("i", -1);
}

static PyObject *
likwid_nvmon_getEventsOfGpu(PyObject *self, PyObject *args)
{
    int g;
    if (!PyArg_ParseTuple(args, "i", &g))
        Py_RETURN_NONE;
    if (gpuTopology_initialized == 0)
    {
        topology_gpu_init();
        gpuTopology_initialized = 1;
    }
    if (gpuTopology_initialized && gputopo == NULL)
    {
        gputopo = get_gpuTopology();
    }
    if (g < 0 || g >= gputopo->numDevices)
        Py_RETURN_NONE;
    NvmonEventList_t l = NULL;

    int num_events = nvmon_getEventsOfGpu(g, &l);
    if (num_events > 0)
    {
        PyObject *o = PyList_New(l->numEvents);

        for (int i = 0; i < l->numEvents; i++)
        {
            PyObject *d = PyDict_New();
            PyDict_SetItem(d, PYSTR("name"), PYSTR(l->events[i].name));
            PyDict_SetItem(d, PYSTR("desc"), PYSTR(l->events[i].desc));
            PyDict_SetItem(d, PYSTR("limit"), PYSTR(l->events[i].limit));
            PyList_SET_ITEM(o, (Py_ssize_t)i, d);
        }
        nvmon_returnEventsOfGpu(l);
        return o;
    }
    Py_RETURN_NONE;
}

#endif

static PyMethodDef LikwidMethods[] = {
    {"likwidversion", likwid_lversion, METH_VARARGS, "Get the likwid version numbers."},
    {"markerinit", likwid_markerinit, METH_VARARGS, "Initialize the LIKWID Marker API."},
    {"markerthreadinit", likwid_markerthreadinit, METH_VARARGS, "Initialize threads for the LIKWID Marker API."},
    {"markerregisterregion", likwid_markerregisterregion, METH_VARARGS, "Register a region to the LIKWID Marker API. Optional"},
    {"markerstartregion", likwid_markerstartregion, METH_VARARGS, "Start a code region."},
    {"markerstopregion", likwid_markerstopregion, METH_VARARGS, "Stop a code region."},
    {"markergetregion", likwid_markergetregion, METH_VARARGS, "Get the current results for a code region."},
    {"markernextgroup", likwid_markernextgroup, METH_VARARGS, "Switch to next event set."},
    {"markerclose", likwid_markerclose, METH_VARARGS, "Close the Marker API and write results to file."},
    {"markerreset", likwid_markerresetregion, METH_VARARGS, "Reset the values of the code region to 0"},
    {"getprocessorid", likwid_getprocessorid, METH_VARARGS, "Returns the current CPU ID."},
    {"pinprocess", likwid_pinprocess, METH_VARARGS, "Pins the current process to the given CPU."},
    {"pinthread", likwid_pinthread, METH_VARARGS, "Pins the current thread to the given CPU."},
    /* misc functions */
    {"setverbosity", likwid_setverbosity, METH_VARARGS, "Set the verbosity for the LIKWID library."},
    /* configuration functions */
    {"initconfiguration", likwid_initconfiguration, METH_VARARGS, "Initialize the configuration module."},
    {"destroyconfiguration", likwid_destroyconfiguration, METH_VARARGS, "Finalize the configuration module."},
    {"setgrouppath", likwid_setgrouppath, METH_VARARGS, "Set search path for performance group files"},
    {"getconfiguration", likwid_getconfiguration, METH_VARARGS, "Get the configration information."},
    /* access functions */
    {"hpmmode", likwid_hpmmode, METH_VARARGS, "Set the access mode for the HPM access module."},
    {"hpminit", likwid_hpminit, METH_VARARGS, "Initialize the HPM access module."},
    {"hpmaddthread", likwid_hpmaddthread, METH_VARARGS, "Add the current thread/CPU to the access module."},
    {"hpmfinalize", likwid_hpmfinalize, METH_VARARGS, "Finalize the HPM access module."},
    /* topology functions */
    {"inittopology", likwid_inittopology, METH_VARARGS, "Initialize the topology module."},
    {"finalizetopology", likwid_finalizetopology, METH_VARARGS, "Finalize the topology module."},
    {"getcputopology", likwid_getcputopology, METH_VARARGS, "Get the topology information for the current system."},
    {"getcpuinfo", likwid_getcpuinfo, METH_VARARGS, "Get the system information for the current system."},
    {"printsupportedcpus", likwid_printsupportedcpus, METH_VARARGS, "Print all CPU variants supported by current version of LIKWID."},
    /* numa functions */
    {"initnuma", likwid_initnuma, METH_VARARGS, "Initialize the NUMA module."},
    {"finalizenuma", likwid_finalizenuma, METH_VARARGS, "Finalize the NUMA module."},
    /* affinity functions */
    {"initaffinity", likwid_initaffinity, METH_VARARGS, "Initialize the affinity module."},
    {"finalizeaffinity", likwid_finalizeaffinity, METH_VARARGS, "Finalize the affinity module."},
    {"cpustr_to_cpulist", likwid_cpustr_to_cpulist, METH_VARARGS, "Translate cpu string to list of cpus."},
    /* timing functions */
    {"getcpuclock", likwid_getCpuClock, METH_VARARGS, "Return the clock frequency of the current system."},
    {"startclock", likwid_startClock, METH_VARARGS, "Start a time measurement."},
    {"stopclock", likwid_stopClock, METH_VARARGS, "Stop a time measurement."},
    {"getclockcycles", likwid_getClockCycles, METH_VARARGS, "Return the clock ticks between start and stop."},
    {"getclock", likwid_getClock, METH_VARARGS, "Return the time in seconds between start and stop."},
    /* temperature functions */
    {"inittemp", likwid_initTemp, METH_VARARGS, "Initialize temperature module of LIKWID."},
    {"readtemp", likwid_readTemp, METH_VARARGS, "Read current temperature."},
    /* power functions */
    {"getpowerinfo", likwid_getPowerInfo, METH_VARARGS, "Initialize and get power information."},
    {"putpowerinfo", likwid_putPowerInfo, METH_VARARGS, "Finalize and return power information."},
    {"startpower", likwid_startPower, METH_VARARGS, "Start a power measurement."},
    {"stoppower", likwid_stopPower, METH_VARARGS, "Stop a power measurement."},
    {"getpower", likwid_getPower, METH_VARARGS, "Get the energy information from a power measurement."},
    /* perfmon functions */
    {"init", likwid_init, METH_VARARGS, "Initialize the whole Likwid system including Performance Monitoring module."},
    {"addeventset", likwid_addEventSet, METH_VARARGS, "Add an event set to LIKWID."},
    {"setup", likwid_setupCounters, METH_VARARGS, "Setup measuring an event set with LIKWID."},
    {"start", likwid_startCounters, METH_VARARGS, "Start measuring an event set with LIKWID."},
    {"stop", likwid_stopCounters, METH_VARARGS, "Stop measuring an event set with LIKWID."},
    {"read", likwid_readCounters, METH_VARARGS, "Read the current values of the configured event set with LIKWID."},
    {"readcpu", likwid_readCountersCpu, METH_VARARGS, "Read the current values of the configured event set on a CPU"},
    {"readgroup", likwid_readGroupCounters, METH_VARARGS, "Read the current values of the given group ID on all CPUs"},
    {"readgroupthread", likwid_readGroupThreadCounters, METH_VARARGS, "Read the current values of the given group ID of the given thread"},
    {"switch", likwid_switchGroup, METH_VARARGS, "Switch the currently set up group."},
    {"finalize", likwid_finalize, METH_VARARGS, "Finalize the whole Likwid system including Performance Monitoring module."},
    {"getresult", likwid_getResult, METH_VARARGS, "Get the current result of a measurement."},
    {"getlastresult", likwid_getLastResult, METH_VARARGS, "Get the result of the last measurement cycle."},
    {"getmetric", likwid_getMetric, METH_VARARGS, "Get the current result of a derived metric."},
    {"getlastmetric", likwid_getLastMetric, METH_VARARGS, "Get the current result of a derived metric with values from the last measurement cycle."},
    {"getnumberofgroups", likwid_getNumberOfGroups, METH_VARARGS, "Get the amount of currently configured groups."},
    {"getnumberofevents", likwid_getNumberOfEvents, METH_VARARGS, "Get the amount of events in a groups."},
    {"getnumberofmetrics", likwid_getNumberOfMetrics, METH_VARARGS, "Get the amount of events in a groups."},
    {"getnumberofthreads", likwid_getNumberOfThreads, METH_VARARGS, "Get the amount of configured threads."},
    {"getidofactivegroup", likwid_getIdOfActiveGroup, METH_VARARGS, "Get the ID of currently active group."},
    {"gettimeofgroup", likwid_getTimeOfGroup, METH_VARARGS, "Get the runtime of a group."},
    {"getgroups", likwid_getGroups, METH_VARARGS, "Get a list of all available performance groups."},
    {"getnameofevent", likwid_getNameOfEvent, METH_VARARGS, "Return the name of an event in a group."},
    {"getnameofcounter", likwid_getNameOfCounter, METH_VARARGS, "Return the name of a counter in a group."},
    {"getnameofmetric", likwid_getNameOfMetric, METH_VARARGS, "Return the name of a metric in a group."},
    {"getnameofgroup", likwid_getNameOfGroup, METH_VARARGS, "Return the name of a group."},
    {"getshortinfoofgroup", likwid_getShortInfoOfGroup, METH_VARARGS, "Return the short description of a group."},
    {"getlonginfoofgroup", likwid_getLongInfoOfGroup, METH_VARARGS, "Return the long description of a group."},
    /* perfmon markerAPI functions */
    {"markerreadfile", likwid_readMarkerFile, METH_VARARGS, "Read in the results from a Marker API run."},
    {"markernumregions", likwid_markerNumRegions, METH_VARARGS, "Return the number of regions from a Marker API run."},
    {"markerregiongroup", likwid_markerRegionGroup, METH_VARARGS, "Return the group of a region from a Marker API run."},
    {"markerregionevents", likwid_markerRegionEvents, METH_VARARGS, "Return the number of events of a region from a Marker API run."},
    {"markerregiontag", likwid_markerRegionTag, METH_VARARGS, "Return the tag of a region from a Marker API run."},
    {"markerregionthreads", likwid_markerRegionThreads, METH_VARARGS, "Return the number of threads of a region from a Marker API run."},
    {"markerregioncpulist", likwid_markerRegionCpulist, METH_VARARGS, "Returns the list of CPUs that took part in the region."},
    {"markerregiontime", likwid_markerRegionTime, METH_VARARGS, "Return the runtime of a region for a thread from a Marker API run."},
    {"markerregioncount", likwid_markerRegionCount, METH_VARARGS, "Return the call count of a region for a thread from a Marker API run."},
    {"markerregionresult", likwid_markerRegionResult, METH_VARARGS, "Return the result of a region for a event/thread combination from a Marker API run."},
    {"markerregionmetric", likwid_markerRegionMetric, METH_VARARGS, "Return the metric value of a region for a metric/thread combination from a Marker API run."},
    /* CPU frequency functions */
    {"getcpuclockcurrent", likwid_freqGetCpuClockCurrent, METH_VARARGS, "Returns the current CPU frequency (in Hz) of the given CPU."},
    {"getcpuclockmax", likwid_freqGetCpuClockMax, METH_VARARGS, "Returns the maximal CPU frequency (in Hz) of the given CPU."},
    {"getcpuclockmin", likwid_freqGetCpuClockMin, METH_VARARGS, "Returns the minimal CPU frequency (in Hz) of the given CPU."},

    {"setcpuclockmax", likwid_freqSetCpuClockMax, METH_VARARGS, "Sets the maximal CPU frequency (in Hz) of the given CPU."},
    {"setcpuclockmin", likwid_freqSetCpuClockMin, METH_VARARGS, "Sets the minimal CPU frequency (in Hz) of the given CPU."},
    {"getgovernor", likwid_freqGetGovernor, METH_VARARGS, "Returns the CPU frequency govneror of the given CPU."},
    {"setgovernor", likwid_freqSetGovernor, METH_VARARGS, "Sets the CPU frequency govneror of the given CPU."},
    {"getavailfreqs", likwid_freqGetAvailFreq, METH_VARARGS, "Returns the available CPU frequency steps (in GHz, returns string)."},
    {"getavailgovs", likwid_freqGetAvailGovs, METH_VARARGS, "Returns the available CPU frequency governors (returns string)."},
#if 0
    {"getuncoreclockcurrent", likwid_freqGetUncoreClockCurrent, METH_VARARGS, "Returns the current Uncore frequency of the given CPU socket."},
#endif
    {"getuncoreclockmax", likwid_freqGetUncoreClockMax, METH_VARARGS, "Returns the maximal Uncore frequency (in Hz) of the given CPU socket."},
    {"getuncoreclockmin", likwid_freqGetUncoreClockMin, METH_VARARGS, "Returns the minimal Uncore frequency (in Hz) of the given CPU socket."},
    {"setuncoreclockmax", likwid_freqSetUncoreClockMax, METH_VARARGS, "Sets the maximal Uncore frequency (in Hz) of the given CPU socket."},
    {"setuncoreclockmin", likwid_freqSetUncoreClockMin, METH_VARARGS, "Sets the minimal Uncore frequency (in Hz) of the given CPU socket."},
#if (LIKWID_MAJOR == 5)
    /* CPU frequency functions (additions for LIKWID 5) */
    {"freqinit", likwid_freqInit, METH_VARARGS, "Initializes the frequency module"},
    {"freqfinalize", likwid_freqFinalize, METH_VARARGS, "Finalizes the frequency module"},
    {"getconfcpuclockmax", likwid_freqGetConfCpuClockMax, METH_VARARGS, "Returns the maximal configurable CPU frequency (in Hz) of the given CPU."},
    {"getconfcpuclockmin", likwid_freqGetConfCpuClockMin, METH_VARARGS, "Returns the minimal configurable CPU frequency (in Hz) of the given CPU."},
    /* GPU functions */
#ifdef LIKWID_NVMON
    {"gpustr_to_gpulist", likwid_gpustr_to_gpulist, METH_VARARGS, "Translate gpu string to list of gpus."},
    {"initgputopology", likwid_initgputopology, METH_VARARGS, "Initialize the topology module for NVIDIA GPUs."},
    {"finalizegputopology", likwid_finalizegputopology, METH_VARARGS, "Finalize the topology module for NVIDIA GPUs."},
    {"getgputopology", likwid_getgputopology, METH_VARARGS, "Get the topology information for the current system for NVIDIA GPUs."},
    /* Nvmon Marker API functions */
    {"gpumarkerinit", likwid_gpumarkerinit, METH_VARARGS, "Initialize the LIKWID Nvmon Marker API."},
    {"gpumarkerregisterregion", likwid_gpumarkerregisterregion, METH_VARARGS, "Register a region to the LIKWID Nvmon Marker API. Optional"},
    {"gpumarkerstartregion", likwid_gpumarkerstartregion, METH_VARARGS, "Start a Nvmon code region."},
    {"gpumarkerstopregion", likwid_gpumarkerstopregion, METH_VARARGS, "Stop a Nvmon code region."},
    {"gpumarkergetregion", likwid_gpumarkergetregion, METH_VARARGS, "Get the current results for a Nvmon code region."},
    {"gpumarkernextgroup", likwid_gpumarkernextgroup, METH_VARARGS, "Switch to next Nvmon event set."},
    {"gpumarkerclose", likwid_gpumarkerclose, METH_VARARGS, "Close the Nvmon Marker API and write results to file."},
    {"gpumarkerreset", likwid_gpumarkerresetregion, METH_VARARGS, "Reset the values of the Nvmon code region to 0"},
    /* Nvmon functions */
    {"nvinit", likwid_nvmon_init, METH_VARARGS, "Initialize the whole Likwid Nvmon system including Nvmon Performance Monitoring module."},
    {"nvaddeventset", likwid_nvmon_addEventSet, METH_VARARGS, "Add an Nvmon event set to LIKWID."},
    {"nvsetup", likwid_nvmon_setupCounters, METH_VARARGS, "Setup measuring an Nvmon event set with LIKWID."},
    {"nvstart", likwid_nvmon_startCounters, METH_VARARGS, "Start measuring an Nvmon event set with LIKWID."},
    {"nvstop", likwid_nvmon_stopCounters, METH_VARARGS, "Stop measuring an Nvmon event set with LIKWID."},
    {"nvread", likwid_nvmon_readCounters, METH_VARARGS, "Read the current values of the configured Nvmon event set with LIKWID."},
    {"nvswitch", likwid_nvmon_switchGroup, METH_VARARGS, "Switch the currently set up Nvmon group."},
    {"nvfinalize", likwid_nvmon_finalize, METH_VARARGS, "Finalize the whole Likwid Nvmon system including Nvmon Performance Monitoring module."},
    {"nvgetresult", likwid_nvmon_getResult, METH_VARARGS, "Get the current Nvmon result of a measurement."},
    {"nvgetlastresult", likwid_nvmon_getLastResult, METH_VARARGS, "Get the Nvmon result of the last measurement cycle."},
    {"nvgetmetric", likwid_nvmon_getMetric, METH_VARARGS, "Get the current Nvmon result of a derived metric."},
    {"nvgetlastmetric", likwid_nvmon_getLastMetric, METH_VARARGS, "Get the current Nvmon result of a derived metric with values from the last measurement cycle."},
    {"nvgetnumberofgroups", likwid_nvmon_getNumberOfGroups, METH_VARARGS, "Get the amount of currently configured Nvmon groups."},
    {"nvgetnumberofevents", likwid_nvmon_getNumberOfEvents, METH_VARARGS, "Get the amount of events in a Nvmon groups."},
    {"nvgetnumberofmetrics", likwid_nvmon_getNumberOfMetrics, METH_VARARGS, "Get the amount of events in a Nvmon groups."},
    {"nvgetnumberofgpus", likwid_nvmon_getNumberOfGpus, METH_VARARGS, "Get the amount of configured GPUs."},
    {"nvgetidofactivegroup", likwid_nvmon_getIdOfActiveGroup, METH_VARARGS, "Get the ID of currently active Nvmon group."},
    {"nvgettimeofgroup", likwid_nvmon_getTimeOfGroup, METH_VARARGS, "Get the runtime of a Nvmon group."},
    {"nvgetgroups", likwid_nvmon_getGroups, METH_VARARGS, "Get a list of all available Nvmon performance groups."},
    {"nvgetnameofevent", likwid_nvmon_getNameOfEvent, METH_VARARGS, "Return the name of an event in a Nvmon group."},
    {"nvgetnameofcounter", likwid_nvmon_getNameOfCounter, METH_VARARGS, "Return the name of a counter in a Nvmon group."},
    {"nvgetnameofmetric", likwid_nvmon_getNameOfMetric, METH_VARARGS, "Return the name of a metric in a Nvmon group."},
    {"nvgetnameofgroup", likwid_nvmon_getNameOfGroup, METH_VARARGS, "Return the name of a Nvmon group."},
    {"nvgetshortinfoofgroup", likwid_nvmon_getShortInfoOfGroup, METH_VARARGS, "Return the short description of a Nvmon group."},
    {"nvgetlonginfoofgroup", likwid_nvmon_getLongInfoOfGroup, METH_VARARGS, "Return the long description of a Nvmon group."},
    {"nvgeteventsofgpu", likwid_nvmon_getEventsOfGpu, METH_VARARGS, "Get the events of a gpu."},
    /* Misc function */
    {"nvsetverbosity", likwid_nvmon_setverbosity, METH_VARARGS, "Set the verbosity for the LIKWID Nvmon library."},
#endif
#endif
    {NULL, NULL, 0, NULL}
};

#if (PY_MAJOR_VERSION == 2)
PyMODINIT_FUNC
initpylikwid(void)
{
    (void) Py_InitModule("pylikwid", LikwidMethods);
}
#endif

#if (PY_MAJOR_VERSION == 3)
static struct PyModuleDef pylikwidmodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "pylikwid",   /* name of module */
    .m_doc = NULL, /* module documentation, may be NULL */
    .m_size = -1,       /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    .m_methods = LikwidMethods,
};

PyMODINIT_FUNC
PyInit_pylikwid(void)
{
        return PyModule_Create(&pylikwidmodule);
}
#endif
