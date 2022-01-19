CC = gcc
CFLAGS = -O3 -I.
LIBS = -lm
SOURCES = co_matrix-sample.c kumi3.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = a

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm *.o a
