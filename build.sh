#!/bin/bash
echo "Building Calculator..."
gcc calculator.c -lSDL2 -lSDL2_ttf -o calculator
echo "Running Calculator..."
./calculator
