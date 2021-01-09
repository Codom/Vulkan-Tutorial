#
# Makefile
# codom, 2021-01-09 05:43
#
CXXFLAGS = -std=c++17 -O2 -Wall -g
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

all: Triangle

Triangle: main.cc
	g++ $(CXXFLAGS) -o Triangle main.cc $(LDFLAGS)


.PHONY: test clean

test: Triangle
	./Triangle

clean:
	rm ./Triangle

# vim:ft=make
#
