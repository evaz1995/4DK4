@echo off
echo Compiling...

gcc -c -g -std=c99 -o  simlib.o simlib.c
g++ -c -g -std=c++17 -o  main.o main.cpp
g++ -g -o main simlib.o main.o