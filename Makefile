CFILES = src/main.cpp src/CPU/i8080.cpp
HFILES = Makefile src/SI.hpp src/CPU/i8080.hpp
FILES = $(CFILES) $(HFILES)
CC = clang++
OUT = bin/main
FLAGS = -lsfml-window -lsfml-system -lsfml-graphics -O3 -o $(OUT)


main: $(FILES)
	$(CC) $(CFILES) $(FLAGS)
