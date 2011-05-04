#!/bin/sh
#gcc -g -ansi -pedantic -Wall -Wtraditional -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Waggregate-return -Wredundant-decls -Wnested-externs -Werror -o tactihack tactihack.c -lncurses -lm

#removed -Wtraditional because it failed the build with CygWin's gcc! (on WinXP) but suceeded on Linux
gcc -g -ansi -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Waggregate-return -Wredundant-decls -Wnested-externs -Werror -o dawn dawn.c
