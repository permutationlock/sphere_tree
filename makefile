all: sphere_tree

sphere_tree: collision.cpp
	g++ collision.cpp -lGL -lGLU -lglut -lGLEW -std=c++1y -O2 -o sphere_tree

clean:
	rm sphere_tree

