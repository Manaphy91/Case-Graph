CXXFLAGS = -Wall -Wextra -pedantic

test: stack.o case_graph.o
	c99 $(CXXFLAGS) stack.o case_graph.o `pkg-config libgvc --cflags` `pkg-config libgvc --libs` -lm -o test 

case_graph.o: case_graph.h case_graph.c
	c99 $(CXXFLAGS) -c case_graph.c `pkg-config libgvc --cflags`

stack.o: stack.h stack.c
	c99 $(CXXFLAGS) -c stack.c

clean:
	rm -f *.o
	rm -f prova
