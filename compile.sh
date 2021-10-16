#!/bin/bash
g++ -g -O2 -std=c++11 -pthread -march=native test.cpp -o test -lntl -lgmp -lm
./test $1 $2
