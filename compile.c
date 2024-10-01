#include "global.h"
#include "javaclass.h"
#include "bytecode.h"

#define MAX_SIZE 127

struct ClassFile cf;
int index1, index2, index3;
int label1, label2;

int tk; // lookahead token
int stackDepth;

int retLoc[2*MAX_SIZE];
int numRets = 0;

extern int assign_var_index(int tokenval);
extern int match(int token); 
extern void stmt();
extern void opt_stmts();
extern void expr();
extern void term();
extern void moreterms();
extern void factor();
extern void morefactors();

//*******************************************************************************

void
java_preamble() {
    // set up new class file structure
    init_ClassFile(&cf);

    // class has public access
    cf.access = ACC_PUBLIC;

    // class name is "Calc"
    cf.name = "Calc";

    // no fields
    cf.field_count = 0;

    // one method
    cf.method_count = 1;

    // allocate array of methods (just one "main" in our example)
    cf.methods = (struct MethodInfo*)malloc(cf.method_count * sizeof(struct MethodInfo));

    // method has public access and is static
    cf.methods[0].access = ACC_PUBLIC | ACC_STATIC;

    // method name is "main"
    cf.methods[0].name = "main";

    // method descriptor of "void main(String[] arg)"
    cf.methods[0].descriptor = "([Ljava/lang/String;)V";

    // max operand stack size of this method
    cf.methods[0].max_stack = MAX_SIZE;

    // the number of local variables in the local variable array
    //   local variable 0 contains "arg"
    //   local variable 1 contains "val"
    //   local variable 2 contains "i" and "result"
    cf.methods[0].max_locals = 3 + MAX_SIZE;

    // set up new bytecode buffer
    init_code();
    
    // generate code
/*LOC*/
/*000*/	emit(aload_0);
/*001*/	emit(arraylength);		// arg.length
/*002*/	emit2(newarray, T_INT);
/*004*/	emit(astore_1);			// val = new int[arg.length]
/*005*/	emit(iconst_0);
/*006*/	emit(istore_2);			// i = 0
    label1 = pc;			// label1:
/*007*/	emit(iload_2);
/*008*/	emit(aload_0);
/*009*/	emit(arraylength);
    label2 = pc;
/*010*/	emit3(if_icmpge, PAD);		// if i >= arg.length then goto label2
/*013*/	emit(aload_1);
/*014*/	emit(iload_2);
/*015*/	emit(aload_0);
/*016*/	emit(iload_2);
/*017*/	emit(aaload);			// push arg[i] parameter for parseInt
    index1 = constant_pool_add_Methodref(&cf, "java/lang/Integer", "parseInt", "(Ljava/lang/String;)I");
/*018*/	emit3(invokestatic, index1);	// invoke Integer.parseInt(arg[i])
/*021*/	emit(iastore);			// val[i] = Integer.parseInt(arg[i])
/*022*/	emit32(iinc, 2, 1);		// i++
/*025*/	emit3(goto_, label1 - pc);	// goto label1
    backpatch(label2, pc - label2);	// label2:

}

//*******************************************************************************

void
java_postamble() {

    for (int i = 0; i < numRets; i++) {
	backpatch(retLoc[i], pc - retLoc[i]);
    }
    index2 = constant_pool_add_Fieldref(&cf, "java/lang/System", "out", "Ljava/io/PrintStream;");
/*036*/	emit3(getstatic, index2);	// get static field System.out of type PrintStream
/*039*/	emit(iload_2);			// push parameter for println()
    index3 = constant_pool_add_Methodref(&cf, "java/io/PrintStream", "println", "(I)V");
/*040*/	emit3(invokevirtual, index3);	// invoke System.out.println(result)
/*043*/	emit(return_);			// return

    // length of bytecode is in the emmiter's pc variable
    cf.methods[0].code_length = pc;
    
    // must copy code to make it persistent
    cf.methods[0].code = copy_code();

    // save class file to "Calc.class"
    save_classFile(&cf);
}

//*******************************************************************************

int
match(int token) {
    int rc = 0;  // in case we don't have a match, we return "false"

    if (token == tk) {
	rc = 1;
	tk = lexan(); // get next token
    }

    return(rc);
}

//*******************************************************************************

void
stmt() {
    int var_index;
    int loc = -1;
    int if_loc = -1;
    int else_loc = -1;

    switch (tk) {
	case LEFT_CURLY:
	    match(LEFT_CURLY);

		// Optionally get a sequence of statements 
	    opt_stmts(); 

		// Check for closing curly brace
	    if (!match(RIGHT_CURLY)) {
		error("Expected closing curly brace");
	    }

	    break;
	
	case ID:
		printf("ID\n");
		printf("VarIndex: %d\n", symtable[tokenval].var_index);
	    var_index = symtable[tokenval].var_index;

		// If the variable is a keyword, print an error
	    if (var_index < 0) {
		error("Attempting to use keyword as variable");
	    } 
		
		// If it's a new variable, assign it a new index
		else if (var_index == 0) {
		var_index = assign_var_index(tokenval); // initializeing variable
	    }
	    // else case already taken care of, just overwrite existing variable
	    
		// Check the ID of the variable
	    match(ID);
		// If it is not '=' then print an error
	    if (!match(EQ)) {
		error("Expected assignment operator");
	    }

		// Get the expression on the right side of the assignment
	    expr();

	    if (stackDepth >= 1) {
			// Store the value in the variable
			emit2(istore, var_index);
			stackDepth--;
	    } else {
			error("No rvalue for assignment operator");
	    }
	    if (!match(';')) {
			error("Expected ';'");
	    }
	    break;
	
	case IF:
	    match(IF);
	    if (!match('(')) {
			error("Mising parenthesis");
	    }
	    expr();
	    if (!match(')')) {
			error("Mising parenthesis");
	    }

	    emit(iconst_0);
	    if_loc = pc;

		// If the condition is false, jump to the else block
	    emit3(if_icmpeq, 0);

	    stmt();

	    if (!match(ELSE)) {
			// If there is no else block, backpatch the jump to the end of the if block
			backpatch(if_loc, pc - if_loc); 
		} else {
			else_loc = pc;

			// If the if block is true, jump to the end of the else block
			emit3(goto_, 0);

			// If the condition is false, jump to the else block
			backpatch(if_loc, pc - if_loc); 

			stmt(); 

			// Backpatch the jump to the end of the else block
			backpatch(else_loc, pc - else_loc);  
		}

	    break;
	
	case WHILE:
	    match(WHILE);
	    if (!match('(')) {
			error("Mising parenthesis");
	    }

		// Save the start location of the while loop
	    int test_loc = pc;

	    expr();

	    if (!match(')')) {
		error("Mising parenthesis");
	    }

	    emit(iconst_0);
	    loc = pc;
		// If the condition is false, jump to the end of the while loop
	    emit3(if_icmpeq, 0);

	    stmt();

		// Go back to the start of the while loop
	    emit3(goto_, test_loc-pc);
		// Backpatch the jump to the end of the while loop
	    backpatch(loc, pc-loc);
	    break;
	
	case RET:
	    match(RET);
	    expr();
	    if (stackDepth >= 1) {
			emit(istore_2);
			retLoc[numRets] = pc;
			emit3(goto_, retLoc[numRets]);
			numRets++;
	    } else {
			error("No value to return, stack empty");
	    }

	case ARG:
		match(ARG);
		if (!match(ID)) {
			error("Expected identifier");
		} else {
			// Get the variable index of the argument
			var_index = symtable[tokenval].var_index;

			// If the variable is a keyword, print an error
			// If it's a new variable, assign it a new index
			if (var_index < 0) {
				error("Attempting to use keyword as variable");
			} else if (var_index == 0) {
				var_index = assign_var_index(tokenval);  
			}

			// Load the value of the argument
			emit2(iload, var_index);  
		}
		break;
	
	default:
	    error("Unexpected token, expected statement");
	    break;
    }
}

//*******************************************************************************

void
opt_stmts() {
    while (tk != RIGHT_CURLY && tk != DONE) {
        stmt();  
    }
	if (!match(LEFT_CURLY)) {
		error("Expected closing curly brace");
	}
}

//*******************************************************************************

void
expr() {
	// Get the first term
    term();  

    while (tk == PLUS || tk == MINUS) {  
        if (tk == PLUS) {
            match(PLUS);  
            term();  
            emit(iadd);  
        } else if (tk == MINUS) {
            match(MINUS);  
            term();  
            emit(isub);  
        }
    }

}

//*******************************************************************************

void
term() {
    factor();  

    while (tk == MUL || tk == DIV || tk == MOD) {  
        if (tk == MUL) {
            match(MUL);  
            factor();  
            emit(imul);  
        } else if (tk == DIV) {
            match(DIV);  
            factor();  
            emit(idiv);  
        } else if (tk == MOD) {
            match(MOD);  
            factor();  
            emit(irem);  
    	}
	}
}


//*******************************************************************************

void
moreterms() {

    if (!(tk==';' || tk==')')) {
	switch (tk) {
	    case PLUS:
		match(PLUS);
		term();
		if (stackDepth < 2) {
		    error("Not enough operands for '+'");
		}
		emit(iadd);
		stackDepth--;
		moreterms();
		break;
	    
	    case MINUS:
		// to be completed

		break;
	    
	    default:
		error("Expected '+' or '-'");
	}
    }
}

//*******************************************************************************

void
morefactors() {
    if (!(tk=='+' || tk=='-' || tk==';' || tk==')')) {
	switch (tk) {
	    case MUL:
		// to be completed

		morefactors();
		break;
	    
	    // to be completed
	    
	    default:
		error("Expected '*', '/' or '%'");
	}
    }
}

//*******************************************************************************

void
factor() {
    int var_index = -1;

    switch (tk) {
	case LEFT_PAREN:
	    match('(');
	    expr();
	    if (!match(')')) {
		error("Missing close parenthesis");
	    }
	    break;
	
	case MINUS:
	    match('-');
	    if (stackDepth < 1) {
		error("Not enough operands for negation");
	    }
	    factor();

	    emit(ineg);
	    break;
	
	case INT8:
	    emit2(bipush, tokenval);
	    stackDepth++;

	    match(INT8);
	    break;
	
	case INT16:
	    emit3(sipush, tokenval);
	    stackDepth++;

	    match(INT16);
	    break;
	
	case INT32:
	    emit2(ldc, constant_pool_add_Integer(&cf, tokenval));
	    stackDepth++;

	    match(INT32);
	    break;
	
	case ID:
	    var_index = symtable[tokenval].var_index;
	    if (var_index < 0) {
		error("Keywords are not valid lvalues");
	    } else if (var_index == 0) {
		error("Variable does not exist");
	    } else {
			// Load the variable's value onto the stack
			emit2(iload, var_index);
			stackDepth++;
		}

	    match(ID);
	    break;
	
	case ARG:
	    match(ARG);

	    // Assuming ARG is an array argument, we need to load the value of ARG[i] for example
		if (!match('[')) {
			error("Expected '[' after ARG");
		}
		
		// Evaluate the expression inside the brackets (for example ARG[0])
		expr();  // Assuming expr() gives the index

		if (!match(']')) {
			error("Expected ']' after expression");
		}

		// Load ARG[index] onto the stack
		emit2(aload, 0);  // aload 0 loads the array ARG (assumed to be in local variable 0)
		emit(iaload);     // iaload loads the value at the computed index

		stackDepth++;
		break;

	default:
	    error("Expected a factor");
	    break;
    }
}

//*******************************************************************************

int
main() {
	printf("HI\n");
	fflush(stdout);
    init();

    java_preamble();

    tk = lexan();

    if (!(tk == DONE || tk == EOF)) {
		printf("HI\n");
	stmt();
    }

    java_postamble();

    // Useful to do the following when debugging
    //dumpSumbolTable();

    return 0;
}
