#!/bin/bash

gcc -o swarming main2.cpp -I/usr/include/SDL2 -D_REENTRANT -lSDL2 -lm
