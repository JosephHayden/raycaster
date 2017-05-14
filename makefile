all:
	gcc `pkg-config --cflags glfw3` -g -o raycaster raycaster.c `pkg-config --static --libs glfw3` `sdl2-config --cflags --libs` -lSDL2_image

alt:
	gcc `pkg-config --cflags glfw3` -g -o raycaster altraycaster.c `pkg-config --static --libs glfw3` `sdl2-config --cflags --libs` -lSDL2_image

clean:
	rm *.o raycaster
