/*
* Team Name: Rolling Pandas
* Assignment: Program 1 - Part 1
* Class: CSC453 Fall 2024
* File: error.c
*/

/**** error.c ****/

#include "global.h"

void error(char *m) {
    fprintf(stderr, "line %d: %s\n", lineno, m);
    exit(1);
}
