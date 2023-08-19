CFILES = main.cpp CPU/i8080.cpp
HFILES = Makefile SI.hpp CPU/i8080.hpp
FILES = $(CFILES) $(HFILES)
CC = clang++
OUT = bin/main
FLAGS = -lsfml-window -lsfml-system -lsfml-graphics -ggdb3 -O0 -o $(OUT)


main: $(FILES)
	$(CC) $(CFILES) $(FLAGS)
