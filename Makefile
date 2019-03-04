all:
	clang++ `sdl2-config --static-libs --cflags` -std=c++11 main.cpp -o lineart
