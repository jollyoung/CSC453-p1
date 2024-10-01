/*
* Team Name: Rolling Pandas
* Assignment: Program 1 - Part 1
* Class: CSC453 Fall 2024
* File: lextest.c
*/

#include <stdio.h>
#include "global.h"

int main(int argc, char* argv[]) {
    init();

    fprintf(stderr, "argc = %d\nargv[0] = %s\n", argc, argv[0]);

    int t = 0;
    do {
        t = lexan();
        if (t != NONE && t != DONE) {
            putchar('<');
            if (t < DONE) {
                // Handle keywords
                if (t >= IF && t <= ARG) {
                    switch(t) {
                        case IF:
                            printf("IF,%d> ", symtable[tokenval].var_index);
                            break;
                        case ELSE:
                            printf("ELSE,%d> ", symtable[tokenval].var_index);
                            break;
                        case WHILE:
                            printf("WHILE,%d> ", symtable[tokenval].var_index);
                            break;
                        case RET:
                            printf("RET,%d> ", symtable[tokenval].var_index);
                            break;
                        case ARG:
                            printf("ARG,%d> ", symtable[tokenval].var_index);
                            break;
                    }
                } else {
                    // Handle general ascii characters
                    printf("%c,%d> ", (char)t, -1);
                }
            } else {
                // Handle t >= DOne (ID, INT8, INT16, INT32, DONE)
                switch(t) {
                    case DONE:
                        // printf("DONE");
                        // printf(",%d> ", symtable[tokenval].var_index);
                        break;
                    
                    case ID:
                        printf("ID,%d> ", symtable[tokenval].var_index);
                        break;
                    
                    case INT8:
                        printf("INT8,%d> ", tokenval);
                        break;
                    
                    case INT16:
                        printf("INT16,%d> ", tokenval);
                        break;
                    
                    case INT32:
                        printf("INT32,%d> ", tokenval);
                        break;
                }
            }
        }
    } while (!(t == DONE || t == EOF));
    putchar('\n');
    return 0;
}