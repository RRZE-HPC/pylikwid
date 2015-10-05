
INCLUDE_LIKWID=-I/usr/local/include
LIBS_LIKWID=-L/usr/local/lib

INCLUDE_PYTHON=-I/usr/include/python2.7
LIBS_PYTHON=

CFLAGS ?= -fPIC -shared

TARGET=likwid.so

INCLUDES= $(INCLUDE_LIKWID) $(INCLUDE_PYTHON)
LIBPATHS= $(LIBS_LIKWID) $(LIBS_PYTHON)
LIBS = -llikwid -lpython2.7


$(TARGET): likwidmodule.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBPATHS) $< -o $@ $(LIBS)

clean:
	rm -rf $(TARGET)
