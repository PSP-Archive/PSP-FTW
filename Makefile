CFLAGS=-g `sdl-config --cflags`
LDFLAGS=-g -lSDL_ttf `sdl-config --libs`
pspftw2d: main.o
	gcc -o pspftw2d main.o $(LDFLAGS)

clean:
	-rm main.o
