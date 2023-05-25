CC=gcc
CFLAGS= -Wall -o -I
LDFLAGS=
SOURCES=main.c bmain.c bitio.h
OBJECTS=$(SOURCES:.c=.c)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

