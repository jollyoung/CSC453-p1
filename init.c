/*
* Team Name: Rolling Pandas
* Assignment: Program 1 - Part 1
* Class: CSC453 Fall 2024
* File: init.c
*/

#include "global.h"

struct entry keywords[] =
{
    {"if", IF, -1},
    // TO BE COMPLETED
    {0, 0, 0}
};

void init() {
    struct entry *p;

    for (p = keywords; p->token; p++) {
	insert(p->lexptr, p->token, p->var_index);
    }
}
