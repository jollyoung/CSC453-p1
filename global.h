#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define BSIZE 128
#define NONE -1
#define EOS '\0'

// Single character tokens, follow ASCII values
#define MOD              37          // '%'
#define LEFT_PAREN       40          // '('
#define RIGHT_PAREN      41          // ')'
#define MUL              42          // '*'
#define PLUS             43          // '+'
#define MINUS            45          // '-'
#define DIV              47          // '/'
#define SEMI             59          // ';'
#define EQ               61          // '='
#define LEFT_BRACKET     91          // '[' 
#define RIGHT_BRACKET    93          // ']'
#define LEFT_CURLY       123         // '{'
#define RIGHT_CURLY      125         // '}'
#define IF               256         // if keyword
#define ELSE             IF+1        // else keyword
#define WHILE            ELSE+1      // while keyword
#define RET              WHILE+1     // return keyword
#define ARG              RET+1       // argument keyword

// Operator tokens
#define DONE	        300         // EOF
#define ID	            DONE+1      // 301, Identifier
#define INT8	        ID+1        // 302, 8-bit integer
#define INT16	        INT8+1      // 303, 16-bit integer
#define INT32	        INT16+1     // 304, 32-bit integer

extern void error(char *m);
extern void init();
extern int lexan();
extern int lookup(char s[]);
extern int insert(char s[], int tok, int var_index);
extern int assign_var_index(int tokenval);
extern int assign_var_index_ifZero(int index);
extern void dumpSumbolTable();

extern int varIndex;
extern int tokenval;
extern int lineno;

struct entry {
    char *lexptr;
    int token;
    // index into java frame vars for this variable, or -1 for keyword, or 0 for undeclared symbol
    int var_index;

};

extern struct entry symtable[];

#endif
