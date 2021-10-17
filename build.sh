#!/bin/bash
g++ -g -O2 -std=c++11 -pthread -march=native wiener.cpp -o wiener -lntl -lgmp -lm
