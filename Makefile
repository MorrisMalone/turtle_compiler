all: clean lexer	

lexer:
	gcc -lSDL2 $(sdl2-config --cflags) -Wall -lm   -o parser turtle-eval.c turtle-main.c turtle-nametab.c sdlinterf.c $(sdl2-config --libs) 

clean:
	rm -f lexer