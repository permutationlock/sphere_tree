# Specify compiler
COMP = clang++ -std=c++1y -O2

# Specify target
all: sphere_tree

# Build executable
sphere_tree: collision.cpp lib/object_w_tree.h lib/octree.h lib/treesphere.h
	$(COMP) collision.cpp -lGL -lGLU -lglut -lGLEW -o sphere_tree

# Clean build
clean:
	rm sphere_tree
