all: sdlfix.so

sdlfix.so: sdlfix.o
	$(CC) -shared -fpic -o $@ $<

CFLAGS = -fpic -g -O0 -Wall -Wextra -Werror 

.PHONY: clean

clean:
	$(RM) sdlfix.so sdlfix.o
