#!/bin/bash
g++ -std=c++17 -O2 wiener_attack.cpp -o wiener_attack -lntl -lgmp
chmod +x wiener_attack
.venv/bin/python3 test.py -n 10 -s 128 -e 2048 -b ./wiener_attack
