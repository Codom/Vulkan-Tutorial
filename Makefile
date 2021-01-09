#
# Makefile
# codom, 2021-01-09 05:43
#
CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

all: VulkanTest

VulkanTest: main.cc
	g++ $(CXXFLAGS) -o VulkanTest main.cc $(LDFLAGS)


.PHONY: test clean

test: VulkanTest
	./VulkanTest

clean:
	rm ./VulkanTest

# vim:ft=make
#
