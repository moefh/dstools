
CC = gcc
CFLAGS = -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-cast-function-type -I$(DEVROOT)/include -I.
LDFLAGS = -w -Wl,-subsystem,windows
#LDFLAGS = 

OBJS = main.o debug.o gl_error.o matrix.o shader.o model.o mouse_camera.o key_camera.o glad.o
LIBS = -L$(DEVROOT)/lib -lglfw3 -lgdi32 -lopengl32 -lm
#LIBS = -lglfw -lGL -ldl -lm

all: dsview

clean:
	-rm -f *.o *.exe

dsview: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
