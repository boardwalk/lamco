#!/bin/sh
set -e
g++ -std=c++11 -Wall -Wextra -Werror -o lamco \
   main.cpp \
   game.cpp \
   map.cpp \
   player.cpp \
   ghost.cpp
