/*
* Team Name: Rolling Pandas
* Assignment: Program 1 - Part 1
* Class: CSC453 Fall 2024
* File: lextest.c
*/

#include <stdio.h>
#include "global.h"

int
main(int argc, char* argv[])
{
    init();

    fprintf(stderr, "argc = %d\nargv[0] = %s\n", argc, argv[0]);

    int t = 0;
    do {
	t=lexan();
	if (t != NONE) {
	    putchar('<');
	    if (t < DONE) {
		printf("%c,%d> ", (char)t, -1);
	    } else {
		switch(t) {
		    case DONE:
			printf("DONE");
			printf(",%d> ", symtable[tokenval].var_index);
			break;
		    // to be completed
		   
		    case INT8:
			printf("INT8");
			printf(",%d> ", tokenval);
			break;
		    // to be completed
		}
	    }
	}
    } while (!(t == DONE || t == EOF));
    putchar('\n');
    return 0;
}
