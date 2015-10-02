
INCLUDE_LIKWID=-I/usr/local/include
LIBS_LIKWID=-L/usr/local/lib -llikwid

INCLUDE_PYTHON=-I/usr/include/python2.7
LIBS_PYTHON=-lpython2.7

CFLAGS ?= -fPIC -shared

TARGET=likwid.so

INCLUDES= $(INCLUDE_LIKWID) $(INCLUDE_PYTHON)
LIBS= $(LIBS_LIKWID) $(LIBS_PYTHON)


$(TARGET): likwidmodule.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $< -o $@

clean:
	rm -rf $(TARGET)
