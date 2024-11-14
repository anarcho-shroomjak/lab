compile:
	gcc main.c -I /opt/homebrew/Cellar/libpng/1.6.44/include -L /opt/homebrew/Cellar/libpng/1.6.44/lib -I /opt/homebrew/Cellar/gmp/6.3.0/include -L /opt/homebrew/Cellar/gmp/6.3.0/lib -o a.out -lgmp -lpng
run:
	./a.out
all: compile run

.DEFAULT_GOAL:=all