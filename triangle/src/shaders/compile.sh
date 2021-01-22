#! /bin/sh
#
# compile.sh
# Copyright (C) 2021 codom <codom@sol>
#
# Distributed under terms of the MIT license.
#

glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv

