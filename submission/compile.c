#include "global.h"
#include "javaclass.h"
#include "bytecode.h"
#include <stdbool.h>

#define MAX_SIZE 127

struct ClassFile cf;
int index1, index2, index3;
int label1, label2;

int tk; // lookahead token
int stackDepth;

int retLoc[2*MAX_SIZE];
int numRets = 0;

bool condition = false;
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

    // class name is "Code"
    cf.name = "Code";

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

    // save class file to "Code.class"
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
int ifCount = 0;
void stmt() {
    int var_index;
    int loc = -1;
    int if_loc = -1;
    int else_loc = -1;

    switch (tk) {
        case LEFT_CURLY:
			match(LEFT_CURLY);

            opt_stmts();  // Process the block of statements inside curly braces
			
			if (!match(RIGHT_CURLY)) {
				error("Expected '}'");
			}
            break;

        case ID:
            // Process assignment statement
            var_index = symtable[tokenval].var_index;

            if (var_index < 0) {
                error("Attempting to use keyword as variable");
            } else if (var_index == 0) {
                var_index = assign_var_index(tokenval);
            }

            match(ID);

            if (!match(EQ)) {
                error("Expected assignment operator");
            }

            expr();

            if (stackDepth >= 1) {
                emit2(istore, var_index);
                stackDepth--;
            } else {
                error("No rvalue for assignment operator");
            }

            if (!match(SEMI)) {
                error("Expected ';'");
            }
            break;

        case IF:
			ifCount++;
            match(IF);

            if (!match(LEFT_PAREN)) {
                error("Missing parenthesis after 'if'");
            }

            expr();

            if (!match(RIGHT_PAREN)) {
                error("Missing closing parenthesis after condition");
            }

			// Push 0 onto the stack to compare with the condition
            emit(iconst_0);

			// Save the current program counter to set up the jump when the condition is false
            if_loc = pc;

			// if_icmpeq pops two integers from the stack and compares them
			// If they are equal (which means its false), the jump is taken	
			// 0 is the offset to jump to, but we don't know that yet so we set it to 0
			// The offset will be backpatched later
			emit3(if_icmpeq, 0);

			stmt(); 

			if (tk == ELSE) {
				match(ELSE);

				if (ifCount == 1) {
					else_loc = pc;
					emit3(goto_, 0);

					backpatch(if_loc, pc - if_loc);

					stmt();
				} else if (ifCount > 1) {
					backpatch(if_loc, pc - if_loc);
					stmt();
				} else {
					error("Unexpected else");
				}
				
				ifCount--;

				// else_loc = pc;

				// // Jump to the end of else block if condition is true
				// emit3(goto_, 0);
				// backpatch(if_loc, pc - if_loc);

				// match(ELSE);
				// stmt();

				backpatch(else_loc, pc - else_loc);
			} else {
				//If there is no else, backpatch the jump to after if
				backpatch(if_loc, pc - if_loc);
			}

			break;

		case ELSE:
			match(ELSE);

			break;

        case WHILE:
            match(WHILE);

            if (!match('(')) {
                error("Missing parenthesis after 'while'");
            }

            int test_loc = pc;
            expr();

            if (!match(')')) {
                error("Missing closing parenthesis after condition");
            }

            emit(iconst_0);
            loc = pc;

			// Jump out of the loop if condition is false
            emit3(if_icmpeq, 0); 

			// Process the statement inside the while loop
            stmt();  

			// Jump back to the test
            emit3(goto_, test_loc - pc);  
			// Backpatch the exit jump
            backpatch(loc, pc - loc);  
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

            if (!match(SEMI)) {
                error("Expected ';'");
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
    // Continue to process statements until we encounter a right curly brace or the end of the input
    while (tk != RIGHT_CURLY && tk != DONE) {
        stmt();  
    }

}


//*******************************************************************************

void
expr() {
	// Get the first term
    term();  

	// Deal with terms after first term if it exists
    moreterms();

	

}

//*******************************************************************************

void
term() {
    factor();  
    morefactors();
}


//*******************************************************************************

void
moreterms() {

    if (!(tk==SEMI || tk==RIGHT_BRACKET || tk == RIGHT_PAREN)) {
		switch (tk) {
			case PLUS:
				match(PLUS);

				// Get term that will come next to '+'
				term();

				// If stack have less than two numbers, it cannot run addition
				if (stackDepth < 2) {
					error("Not enough operands for '+'");
				}

				// Pop up two numbers from stack, and push the result of addition of them
				emit(iadd);

				// Since we popped up 2 numbers and pushed 1, decrease stack depth
				stackDepth--;

				// Deal with more terms if they exists
				moreterms();

				break;
			
			case MINUS:
				match(MINUS);

				term();

				if (stackDepth < 2) {
					error("Not enough operands for negation");
				}

				emit(isub);
				stackDepth--;
				moreterms();

				break;
		
			default:
				error("Expected '+' or '-'");
		}
	} else {
		return;
	}
}

//*******************************************************************************

void
morefactors() {
    if (!(tk== PLUS || tk== MINUS || tk== SEMI || tk==RIGHT_PAREN || tk== RIGHT_BRACKET)) {
		
		switch (tk) {
			case MUL:
				match(MUL);
				factor();
				if(stackDepth < 2){
					error("Not enough operands for '*'");
				}
				
				emit(imul);
				stackDepth--;
				
				morefactors();
				break;
			
			case DIV:
				match(DIV);
				factor();
				if(stackDepth < 2){
					error("Not enough operands for '/'");
				}
			
				emit(idiv);
				stackDepth--;

				morefactors();
				break;

			case MOD:
				match(MOD);
				factor();
				if(stackDepth < 2){
					error("Not enough operands for '%'");
				}
				
				emit(irem);
				stackDepth--;
				morefactors();
				break;
			
			default:
				error("Expected '*', '/' or '%'");
		}
    } else {
		return;
	}
}

//*******************************************************************************
int array_index = -1;
void
factor() {
    int var_index = -1;
    switch (tk) {
		case LEFT_PAREN:
			match(LEFT_PAREN);
			expr();
			if (!match(')')) {
				error("Missing close parenthesis");
			}
			break;
	
		case MINUS:
			match(MINUS);
			if (stackDepth < 1) {
				error("Not enough operands for negation");
			}
			factor();

			emit(ineg);
			break;
	
		case INT8:
			// Push read integer on the stack
			array_index = tokenval;
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
				// Load the variable's value from local table and push to the stack
				emit2(iload, var_index);
				stackDepth++;
			}

			match(ID);
			break;
	
		case ARG:
			match(ARG);

			if (!match(LEFT_BRACKET)) {
				error("Expected '[' after ARG");
			}

			// Push arg array
			// It has to be run before expr since iaload requires array first
			emit(aload_1);  

			expr(); 


			// As we are pushing array_index in case INT8, we shouldn't push again
			// emit2(bipush, array_index);


			// Get the value from array with index, and push the value on stack
			emit(iaload);  

			if (!match(RIGHT_BRACKET)) {
				error("Expected ']' after expression");
			}

			break;

		default:
			error("Expected a factor");
			break;
	}
}

//*******************************************************************************

int
main() {
    init();

    java_preamble();

    tk = lexan();

    if (!(tk == DONE || tk == EOF)) {
		stmt();
    }
	// while (tk != DONE && tk != EOF) {
	// 	stmt();  
	// 	tk = lexan();
	// }


    java_postamble();

    // Useful to do the following when debugging
    //dumpSumbolTable();

    return 0;
}
