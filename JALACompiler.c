#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "Stack.h"
#include "StringOps.h"
#include "StringList.h"

#define MEM_STRT 0x1000 //Where the memory block starts

/** @struct Block_ct
 * Holds the current count of each type of block that needs a jump tag
 * so that each can be unique.
 * Also contains a pointer to the last used cell in memory, so that anything above it will be free.
 */
typedef struct {
	int mem_addr; ///< Topmost accessed memory address.
	int if_ct; ///< Number of tags used in if statements.
	int for_ct; ///< Number of tags used in for loops.
	int while_ct; ///< Number of tags used in while loops.
} Block_ct;

/** @enum Comparison
 * Holds the different types of binary comparisons.
 */
typedef enum {
	EQUAL, NOT_EQUAL, LESSER, GREATER, LESSER_EQ, GREATER_EQ
} Comparison;

/** @enum Operation
 * Distinguishes between the 4 basic operations.
 * NO_OP is to be used as kind of a "No operation need be executed at this time" marker.
 */
typedef enum {
	NO_OP, ADD, SUB
} Operation;

/** @enum Type
 * Holds the different types that are recognized.
 * MAIN is special, since it shouldn't actually return anything, nor write a jr at the end of the function.
 */
typedef enum {
	VOID, INT, MAIN
} Type;

char * read_block(FILE *input_file, FILE *output_file, Stack *stack, String_list string_set[], Block_ct *block_ct, int *line_ct, int top_addr);
char * parse_exp(FILE *output_file, char *line, Stack *stack, String_list string_set[]);
char * read_if_block(FILE *input_file, FILE *output_file, Block_ct *block_ct, String_list string_set[], char *headline, Stack *stack, int *line_ct, int top_addr);
void read_while_block(FILE *input_file, FILE *output_file, Block_ct *block_ct, String_list string_set[], char *headline, Stack *stack, int *line_ct, int top_addr);

/**
 * Returns the first word in the line as a string.
 * Note that the return string must be freed
 *
 * @param line The line to read from.
 * @return A string of the first word on the line.
 */
char * read_word(char *line) {
	char *cpy = (char *)malloc(STR_LEN);
	cpy[0] = '\0';

	char *word = NULL;
	int base = 0;
	int cpy_len = strlen(line);
	int paren_ct = 0;

	strcpy(cpy, line);

	while(cpy_len - base > 0 && !((cpy[base] >= 48 && cpy[base] <= 57) || (cpy[base] >= 65 && cpy[base] <= 90) || (cpy[base] >= 97 && cpy[base] <= 122))) { //Continue while the base char is a numeral or a letter
		if(cpy[base] == '+' || cpy[base] == '-' || cpy[base] == '*' || cpy[base] == '/') { //Special case to check for an operation
			word = (char *)malloc(2);
			word[0] = cpy[base];
			word[1] = '\0';

			free(cpy);

			return word;
		}
		base++;
	}

	if(cpy_len - base == 0) {
#ifdef DEBUG
		printf("Found NULL word.\n");
#endif

		return NULL;
	}

	int index = 0;

	word = (char *)malloc(cpy_len - strlen(strchr(cpy + base, '\0')) + 1);

	while((cpy[base] >= 48 && cpy[base] <= 57) || (cpy[base] >= 65 && cpy[base] <= 90) || (cpy[base] >= 97 && cpy[base] <= 122)) {
		word[index++] = cpy[base++];
	}

	word[index] = '\0';

	if(cpy[base] == '(' && strstr(word, "if") <= 0 && strstr(word, "while") <= 0 && strstr(word, "for") <= 0) {
		paren_ct = 0;
		do {
			word[index++] = cpy[base];

			if(cpy[base] == ')')
				paren_ct--;
			else if(cpy[base] == '(')
				paren_ct++;

			base++;
		} while(paren_ct > 0);
	}

	word[index] = '\0';
	free(cpy);

	return word;
}

/**
 * Returns the next line in the file as a cleaned string.
 * Leading and trailing whitespace will be removed, as will comments.
 *
 * @param input_file The file to be read from.
 * @param output_file The assembly file to be written to.
 * @param line_ct A count of the lines read.
 * @return A string representation of the next line in the file.
 */
char * read_next_line(FILE *input_file, FILE *output_file, int *line_ct) {
#ifdef DEBUG
	printf("-----Parsing line %3d-----\n", ++(*line_ct));
#endif

	fprintf(output_file, "#Line: %d\n", *line_ct);

	if(input_file != NULL) {
		char *line = (char *)malloc(STR_LEN);

		if(fgets(line, STR_LEN, input_file) != NULL)
			return clean_str(line);
	}

	return NULL;
}

/**
 * Handles calling a function. Pushed parameters onto the stack, then handles jumping to the function.
 * This won't handle assigning a variable to the output, if there is one. That will be handled elsewhere.
 *
 * @param output_file The assembly file that is being written to.
 * @param line The line where the function is being called.
 * @param name The name of the function.
 * @param stack The current state of the stack.
 * @param string_set The hashmap of all the variables and their memory locations.
 */
void func_call(FILE *output_file, char *line, char *name, Stack *stack, String_list string_set[]) {
	fprintf(output_file, "\t#Calling function %s\n", name);
	char *call_line = strstr(line, name) + strlen(name) + 1; //The line starting after the name. Plus 1 for the parenthesis
	int var_ct = 0;

	for(int i = 0; i < LIST_LEN; i++) {
		for(int j = 0; j < string_set[i].length; j++) {
			stack_push(stack, string_set[i].keys[j]);
			fprintf(output_file, "\tpushi %#x\n\tpush\n", string_set[i].addrs[j]);
			var_ct++;
		}
	}
	fprintf(output_file, "\n");

	while(strchr(call_line, ',') != 0) {
		call_line = parse_exp(output_file, call_line, stack, string_set) + 1;
	}
	parse_exp(output_file, call_line, stack, string_set); //Once more for last parameter.

	fprintf(output_file, "\tpushi %s\n\tjpush\n\n", name);
	fprintf(output_file, "\tpushi %#x\n\tpop\n", var_ct * 4 + MEM_STRT);

	for(int i = 0; i < var_ct; i++) {
		fprintf(output_file, "\tpushi %#x\n\tpop\n", string_set_contains(string_set, stack_pop(stack)));
	}
	fprintf(output_file, "\tpushi %#x\n\tpush\n\n", var_ct * 4 + MEM_STRT);
}

/**
 * Parses a mathematical expression recursively and writes it to the output.
 *
 * @param output_file The assembly output file.
 * @param line The line starting at the beginning of the expression.
 * @param stack The current state of the stack in memory.
 * @param string_set The hashmap of where all variables are stored in memory.
 * @return The line starting at the end of the evaluated expression. Mostly used for recursion.
 */
char * parse_exp(FILE *output_file, char *line, Stack *stack, String_list string_set[]) {
	Operation next_op = NO_OP;
	int base = 0;
	char *word;

	while(strlen(line + base) > 0 && line[base] != ';' && line[base] != ',' && line[base] != '=' && line[base] != '>' && line[base] != '<' && line[base] != '!') {
		while(line[base] == ' ')
			base++;

		if(line[base] == '(') { //Opens a new parse_exp instance to handle the inside of the expression.
			base++;
			line = parse_exp(output_file, line + base, stack, string_set);

			switch(next_op) {
			case NO_OP:
				break;
			case ADD:
				fprintf(output_file, "\tadd\n");
				next_op = NO_OP;
				break;
			case SUB:
				fprintf(output_file, "\tsub\n");
				next_op = NO_OP;
				break;
			}
		} else if(line[base] == ')' || line[base] == '<' ||  line[base] == '>' ||  line[base] == '=' ||  line[base] == '!') { //Reached the end of this expression return.
			if(line[base + 1] == '=')
				return line + base + 2;

			return line + base + 1;
		} else if(line[base] == '+') {
			base++;
			next_op = ADD;
		} else if(line[base] == '-') {
			base++;
			next_op = SUB;
		} else {
			word = read_word(line + base);
			base = (strstr(line, word) - line) + strlen(word); //Update the base

			if(strchr(word, '(') != 0) { //This is a function call.
				word[strlen(word) - strlen(strchr(word, '('))] = '\0';
				func_call(output_file, line, word, stack, string_set);
			} else {
				if(word[0] >= 48 && word[0] <= 57) { //Is a constant
					fprintf(output_file, "\tpushi %s\n", word);
				} else {
					fprintf(output_file, "\tpushi %#x\n\tpush\n", string_set_contains(string_set, word));
				}

				switch(next_op) {
				case NO_OP:
					break;
				case ADD:
					fprintf(output_file, "\tadd\n");
					next_op = NO_OP;
					break;
				case SUB:
					fprintf(output_file, "\tsub\n");
					next_op = NO_OP;
					break;
				}
			}

			free(word);
		}
	}

	return line + base;
}

/**
 * The main workhorse function that goes through a block, line by line, and returns when it hits
 * a closing }. This essentially writes straigh-line assembly code.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param stack The current status of the stack at this point in the code.
 * @param string_set Hashmap linking variables to memory addresses.
 * @param line_ct The line number that is currently being parsed.
 * @param top_addr The greatest address already used.
 * @return The last line read which includes the closing }.
 */
char * read_block(FILE *input_file, FILE *output_file, Stack *stack, String_list string_set[], Block_ct *block_ct, int *line_ct, int top_addr) {
	char *line = read_next_line(input_file, output_file, line_ct);
	char *first_word = NULL;

	while(strchr(line, '}') == 0 && strstr(line, "return") == 0) {
		if(strlen(line) == 0) { //Check if line is empty
			free(line);
			line = read_next_line(input_file, output_file, line_ct);
			continue;
		}

		first_word = read_word(line);

		if(strstr(first_word, "if") == first_word) { //Check for if statement
#ifdef DEBUG
			printf("Reading if statement. With word %s.\n", first_word);
#endif
			line = read_if_block(input_file, output_file, block_ct, string_set, line, stack, line_ct, top_addr);
		} else if(strstr(first_word, "while") == first_word) { //Check for while loop
			printf("Reading while statement.\n", line);

			read_while_block(input_file, output_file, block_ct, string_set, line, stack, line_ct, top_addr);
			printf("Finished reading while statement.\n");
		} else {
			if(strstr(first_word, "int") > 0) { // Is the line a variable definition?
				first_word = read_word(line + 3);

#ifdef DEBUG
				printf("Found declaration of variable %s.\n", first_word);
#endif

				top_addr += 4;
				string_set_add(string_set, first_word, top_addr);
			}

			if(strchr(line, '=') > 0) { //There is a variable assignment.
				parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);
				fprintf(output_file, "\tpushi %#x\n\tpop\n", string_set_contains(string_set, first_word));
			}
		}

		free(line);
		free(first_word);
		line = read_next_line(input_file, output_file, line_ct);
	}

	return line;
}

/**
 * Takes the header line of a function and returns an ordered list of the names of the parameters.
 *
 * @param stack The current status of the memory stack.
 * @param headline The header line for the function.
 */
void read_func_header(Stack *stack, char *headline) {
	char *parameter_line = strchr(headline, '(') + 1; // The header line starting with the (

	while((parameter_line = strstr(parameter_line, "int "))) {
		parameter_line += 4;

		stack_push(stack, read_word(parameter_line));
	}

#ifdef DEBUG
	print_stack(*stack);
#endif
}

/**
 * Reads a while loop block and writes the needed assembly code to the file.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param string_set The hashmap containing the addresses of each variable in memory.
 * @param headline String representation of the header line of this if statement.
 * @param stack The current status of the stack at this point in the code.
 * @param line_ct The line number that is currently being parsed.
 * @param top_addr The greatest address already used.
 */
void read_while_block(FILE *input_file, FILE *output_file, Block_ct *block_ct, String_list string_set[], char *headline, Stack *stack, int *line_ct, int top_addr) {
	String_list local_set[LIST_LEN];
	string_set_cpy(local_set, string_set);

	char *line = (char *)malloc(STR_LEN);
	line[0] = '\0';
	strcpy(line, headline);

	fprintf(output_file, "start_while_%d:\n", block_ct->while_ct);
	//Find which comparison is being used
	//Key: if(A [comp] B)
	fprintf(output_file, "\t#%s\n", line);
	if(strstr(line, "==") > 0) { //A, B, bne
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 2, stack, string_set);

		fprintf(output_file, "\tbne end_while_%d\n", block_ct->while_ct);
	} else if(strstr(line, "!=") > 0) { //A, B, beq
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);

		fprintf(output_file, "\tbeq end_while_%d\n", block_ct->while_ct);
	} else if(strstr(line, ">=") > 0) { //A, B, slt, 1, beq
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbeq end_while_%d\n", block_ct->while_ct);
	} else if(strstr(line, "<=") > 0) { //B, A, slt, 1, beq
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbeq end_while_%d\n", block_ct->while_ct);
	} else if(strchr(line, '>') > 0) { //B, A, slt, 1, bne
		parse_exp(output_file, strchr(line, '>') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '('), stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbne end_while_%d\n", block_ct->while_ct);
	} else if(strchr(line, '<') > 0) { //A, B, slt, 1, bne
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '<') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbne end_while_%d\n", block_ct->while_ct);
	} else {
		printf("ERROR: Unrecognized comparison in line %s\n", headline);
		parse_exp(output_file, strchr(headline, '('), stack, string_set);
	}

	fprintf(output_file, "\n");
	free(line);

	free(read_block(input_file, output_file, stack, local_set, block_ct, line_ct, top_addr));
	fprintf(output_file, "\tpushi start_while_%d\n\tjpop\n", block_ct->while_ct);
}

/**
 * Reads an if block and writes the needed assembly code to the file.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param string_set The hashmap containing the addresses of each variable in memory.
 * @param headline String representation of the header line of this if statement.
 * @param stack The current status of the stack at this point in the code.
 * @param line_ct The line number that is currently being parsed.
 * @param top_addr The greatest address already used.
 * @return The last line read, that being the first line outside of the if statement.
 * This is necessary as it must check the next line for an else.
 */
char * read_if_block(FILE *input_file, FILE *output_file, Block_ct *block_ct, String_list string_set[], char *headline, Stack *stack, int *line_ct, int top_addr) {
	String_list local_set[LIST_LEN]; //Holds any variable declarations inside the if block
	string_set_cpy(local_set, string_set);

	char *line = (char *)malloc(STR_LEN);
	line[0] = '\0';
	strcpy(line, headline);

	fprintf(output_file, "\t#%s\n", headline);

	//Find which comparison is being used
	//Key: if(A [comp] B)
	if(strstr(line, "==") > 0) { //A, B, bne
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 2, stack, string_set);

		fprintf(output_file, "\tbne end_if_%d\n", block_ct->if_ct);
	} else if(strstr(line, "!=") > 0) { //A, B, beq
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);

		fprintf(output_file, "\tbeq end_if_%d\n", block_ct->if_ct);
	} else if(strstr(line, ">=") > 0) { //A, B, slt, 1, beq
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbeq end_if_%d\n", block_ct->if_ct);
	} else if(strstr(line, "<=") > 0) { //B, A, slt, 1, beq
		parse_exp(output_file, strchr(line, '=') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbeq end_if_%d\n", block_ct->if_ct);
	} else if(strchr(line, '>') > 0) { //B, A, slt, 1, bne
		parse_exp(output_file, strchr(line, '>') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbne end_if_%d\n", block_ct->if_ct);
	} else if(strchr(line, '<') > 0) { //A, B, slt, 1, bne
		parse_exp(output_file, strchr(line, '(') + 1, stack, string_set);
		parse_exp(output_file, strchr(line, '<') + 1, stack, string_set);

		fprintf(output_file, "\tslt\n\tpushi 1\n");
		fprintf(output_file, "\tbne end_if_%d\n", block_ct->if_ct);
	} else {
		printf("ERROR: Unrecognized comparison in line %s\n", headline);
		parse_exp(output_file, strchr(headline, '(') + 1, stack, string_set);
	}

	fprintf(output_file, "\n");
	free(line);

	line = read_block(input_file, output_file, stack, local_set, block_ct, line_ct, top_addr);

	if(strstr(line, "else") == 0) { //No else on this line, check next line.
		free(line);
		line = read_next_line(input_file, output_file, line_ct);
		if(strstr(line, "else") == 0) { //No paired else statement
			fprintf(output_file, "end_if_%d:\n", block_ct->if_ct++);
		} else { //There is an else statement
			fprintf(output_file, "\tpushi end_else_%d\n\tjpop\nend_if_%d:\n", block_ct->if_ct, block_ct->if_ct);
			free(line);
			line = read_block(input_file, output_file, stack, local_set, block_ct, line_ct, top_addr);
			fprintf(output_file, "end_else_%d:\n", block_ct->if_ct++);
		}
	} else { //There is an else statement
		fprintf(output_file, "\tpushi end_else_%d\n\tjpop\nend_if_%d:\n", block_ct->if_ct, block_ct->if_ct);
		free(line);
		line = read_block(input_file, output_file, stack, local_set, block_ct, line_ct, top_addr);
		fprintf(output_file, "end_else_%d:\n", block_ct->if_ct++);
	}

	return line;
}

/**
 * Generates the assembly code for the function starting at the current point in the file.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param headline String representation of the header line of this function.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param line_ct The line number that is currently being parsed.
 * @param ret_type The type of function this is.
 */
void read_func(FILE *input_file, FILE *output_file, char *headline, Block_ct *block_ct, Type ret_type, int *line_ct) {
	Stack stack = {NULL, 0};
	String_list string_set[LIST_LEN];
	char *last_line;
	char *popped_var;
	int num_pars = 0;

	for(int i = 0; i < LIST_LEN; i++) { //Initialize each individual list in the string set
		string_set[i].length = 0;
	}

	read_func_header(&stack, headline);
	if(strchr(headline, '{') <= 0) //Check if there is no opening curly brace on the headline
		read_next_line(input_file, output_file, line_ct);

	// Make memory locations for the parameters
	for(int i = stack.size - 1; i >= 0; i--) {
		num_pars++;
		fprintf(output_file, "\tpushi %#x\n\tpop\n", MEM_STRT + 4 * i);
		string_set_add(string_set, stack.names[i], MEM_STRT + 4 * i);
		stack_pop(&stack);
	}

	last_line = read_block(input_file, output_file, &stack, &string_set[0], block_ct, line_ct, MEM_STRT + (num_pars - 1) * 4);

	//Empty stack
	while(stack.size > 0) {
		popped_var = stack_pop(&stack);
		fprintf(output_file, "\tpushi %#x\n\tpop\n", string_set_remove_str(string_set, popped_var));
	}

	switch(ret_type) {
	case VOID:
	case MAIN:
		if(strstr(last_line, "return")) {
			free(last_line);
			last_line = read_next_line(input_file, output_file, line_ct);
			while(!strchr(last_line, '}')) {
				free(last_line);
				last_line = read_next_line(input_file, output_file, line_ct);
			}
		}
		break;
	case INT:
		parse_exp(output_file, last_line + 6, &stack, string_set);
		free(last_line);
		last_line = read_next_line(input_file, output_file, line_ct);
		while(!strchr(last_line, '}')) {
			free(last_line);
			last_line = read_next_line(input_file, output_file, line_ct);
		}
		break;
	}

	if(ret_type != MAIN) //If main function, don't put the jr at the end
		fprintf(output_file, "\tjr\n");

#ifdef DEBUG
	printf("Finished parsing function.\n");
#endif
}

/**
 * Starting point for program. Reads off the input file name from the command line.
 */
int main(int argc, char *argv[]) {
	if(argc < 2) {
		printf("Enter a filename.");
		return 0;
	}

	FILE *input_file;
	FILE *output_file;
	Block_ct block_ct = {MEM_STRT, 0, 0, 0};
	char *filename = argv[1];
	char *line;
	char *first_word = NULL;
	int line_ct = 0;

	input_file = fopen(filename, "r");
	output_file = fopen(strcat(filename, ".asm"), "w");

	fprintf(output_file, "\tpushi main\n\tjpop\n");

	while((line = read_next_line(input_file, output_file, &line_ct))) {
		if(strlen(line) > 1) {
			first_word = read_word(line);

			if(first_word == NULL) { //Line is empty
#ifdef DEBUG
				printf("Read empty line.\n");
#endif

				continue;
			}

			if(strstr(first_word, "int") == first_word) { //Found an int function
				char *name = read_word(line + 4);
				name[strlen(name) - strlen(strchr(name, '('))] = '\0';

#ifdef DEBUG
				printf("Found integer function %s.\n", name);
#endif

				fprintf(output_file, "\n#################\n%s:\n#################\n", name);
				if(strcmp(name, "main") == 0)
					read_func(input_file, output_file, line, &block_ct, MAIN, &line_ct);
				else
					read_func(input_file, output_file, line, &block_ct, INT, &line_ct);
			} else if(strstr(first_word, "void") == first_word) { //Found a void function
#ifdef DEBUG
				printf("Found void function %s.\n", read_word(line + 5));
#endif

				if(strlen(line) > 5) {
					char *name = read_word(line + 5);
					fprintf(output_file, "\n#################\n%s:\n#################\n", name);
					if(strcmp(name, "main") == 0)
						read_func(input_file, output_file, line, &block_ct, MAIN, &line_ct);
					else
						read_func(input_file, output_file, line, &block_ct, VOID, &line_ct);
				}
			}
		} else {
#ifdef DEBUG
			printf("Read empty line.\n");
#endif
		}
	}

	printf("Finished!\n");
    return 0;
}
