/*
* Team Name: Rolling Pandas
* Assignment: Program 1 - Part 1
* Class: CSC453 Fall 2024
* File: lexer.c
*/

#include "global.h"

char lexbuf[BSIZE];
int lineno = 1;
int tokenval = NONE;

int lexan() {
    int t;
    int rc;

    while (1) {
	t = getchar();

	if (t== ' ' || t == '\t') {
	} else if (t == '\n') {
	    lineno = lineno + 1;
	} else if (isdigit(t)) {
	    ungetc(t, stdin);
	    scanf("%d", &tokenval);
	    if (tokenval < 256) {
		rc = INT8;
	    } else if (tokenval < 65536) {
		rc = INT16;
	    } else {
		rc = INT32;
	    }
	    break;
	} else if (isalpha(t) || t == '_') {
	    int p, b = 0;

	    while (isalnum(t) || t == '_') {
		lexbuf[b] = t;
		t = getchar();
		b = b + 1;

		if (b >= BSIZE) {
		    error("compiler error");
		}
	    }

	    lexbuf[b] = EOS;

	    if (t != EOF) {
		ungetc(t, stdin);
	    }
	    p = lookup(lexbuf);

	    if (p == -1) {
		//printf("** calling insert\n");
		p = insert(lexbuf, ID, varIndex);
		varIndex++;
	    }

	    tokenval = p;

	    rc = symtable[p].token;

	    break;
	} else if (t == EOF) {
	    rc = DONE;
	    break;
	} else {
	    tokenval = NONE;
	    rc = t;
	    break;
	}
    }

    return(rc);
}
