
#include <Python.h>

#include <likwid.h>


static PyObject *
likwid_init(PyObject *self, PyObject *args)
{
    likwid_markerInit();
    Py_RETURN_NONE;
}

static PyObject *
likwid_threadinit(PyObject *self, PyObject *args)
{
    likwid_markerThreadInit();
    Py_RETURN_NONE;
}

static PyObject *
likwid_registerregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerRegisterRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_startregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerStartRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_stopregion(PyObject *self, PyObject *args)
{
    const char *regiontag;
    int ret;
    if (!PyArg_ParseTuple(args, "s", &regiontag))
        return NULL;

    ret = likwid_markerStopRegion(regiontag);
    return Py_BuildValue("i", ret);
}

static PyObject *
likwid_getregion(PyObject *self, PyObject *args)
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
likwid_nextgroup(PyObject *self, PyObject *args)
{
    likwid_markerNextGroup();
    Py_RETURN_NONE;
}

static PyObject *
likwid_close(PyObject *self, PyObject *args)
{
    likwid_markerClose();
    Py_RETURN_NONE;
}

static PyMethodDef LikwidMethods[] = {
    {"init", likwid_init, METH_VARARGS, "Initialize the LIKWID Marker API."},
    {"threadinit", likwid_threadinit, METH_VARARGS, "Initialize threads for the LIKWID Marker API."},
    {"registerregion", likwid_registerregion, METH_VARARGS, "Register a region to the LIKWID Marker API. Optional"},
    {"startregion", likwid_startregion, METH_VARARGS, "Start a code region."},
    {"stopregion", likwid_stopregion, METH_VARARGS, "Stop a code region."},
    {"getregion", likwid_getregion, METH_VARARGS, "Get the current results for a code region."},
    {"nextgroup", likwid_nextgroup, METH_VARARGS, "Switch to next event set."},
    {"close", likwid_close, METH_VARARGS, "Close the Marker API and write results to file."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initlikwid(void)
{
    (void) Py_InitModule("likwid", LikwidMethods);
}
