
#include <Python.h>

#include <likwid.h>


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
    PyObject *pyList, *pyValue;
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

static PyMethodDef LikwidMethods[] = {
    {"markerinit", likwid_markerinit, METH_VARARGS, "Initialize the LIKWID Marker API."},
    {"markerthreadinit", likwid_markerthreadinit, METH_VARARGS, "Initialize threads for the LIKWID Marker API."},
    {"markerregisterregion", likwid_markerregisterregion, METH_VARARGS, "Register a region to the LIKWID Marker API. Optional"},
    {"markerstartregion", likwid_markerstartregion, METH_VARARGS, "Start a code region."},
    {"markerstopregion", likwid_markerstopregion, METH_VARARGS, "Stop a code region."},
    {"markergetregion", likwid_markergetregion, METH_VARARGS, "Get the current results for a code region."},
    {"markernextgroup", likwid_markernextgroup, METH_VARARGS, "Switch to next event set."},
    {"markerclose", likwid_markerclose, METH_VARARGS, "Close the Marker API and write results to file."},
    {"getprocessorid", likwid_getprocessorid, METH_VARARGS, "Returns the current CPU ID."},
    {"pinprocess", likwid_pinprocess, METH_VARARGS, "Pins the current process to the given CPU."},
    {"pinthread", likwid_pinthread, METH_VARARGS, "Pins the current thread to the given CPU."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initlikwid(void)
{
    (void) Py_InitModule("likwid", LikwidMethods);
}
