
ifeq ($(OS),Windows_NT)
OS_CFLAGS = -I$(DEVROOT)/include
OS_LDFLAGS = -w -Wl,-subsystem,windows
OS_LIBS = -L$(DEVROOT)/lib -lglfw3 -lgdi32 -lopengl32
else
OS_CFLAGS =
OS_LDFLAGS =
OS_LIBS = -lglfw -lGL -ldl
endif

CC = gcc
CFLAGS = $(OS_CFLAGS) -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-cast-function-type -I.
LDFLAGS = $(OS_LDFLAGS)

OBJS = main.o debug.o gl_error.o matrix.o dir.o shader.o model.o mouse_camera.o key_camera.o glad.o
LIBS = $(OS_LIBS) -lm

all: dsview

clean:
	-rm -f *.o
	-rm -f dsview

dsview: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
