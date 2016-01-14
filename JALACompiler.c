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

/**
 * Returns the first word in the line as a string.
 * Note that the return string must be freed
 *
 * @param line The line to read from.
 * @return A string of the first word on the line.
 */
char * read_word(char *line) {
	char *cpy = (char *)malloc(100);
	cpy[0] = '\0';

	char *word = NULL;
	int base = 0;
	int cpy_len = strlen(line);

	strcpy(cpy, line);

	while(cpy_len - base > 0 && !((cpy[base] >= 48 && cpy[base] <= 57) || (cpy[base] >= 65 && cpy[base] <= 90) || (cpy[base] >= 97 && cpy[base] <= 122))) //Continue while the base char is a numeral or a letter
		base++;

	if(cpy_len - base == 0) {
		#ifdef DEBUG
		printf("Found NULL word.\n");
		#endif

		return NULL;
	}

	int index = 0;

	word = (char *)malloc(cpy_len - strlen(strchr(cpy + base, '\0')) - index + 1);

	while((cpy[base] >= 48 && cpy[base] <= 57) || (cpy[base] >= 65 && cpy[base] <= 90) || (cpy[base] >= 97 && cpy[base] <= 122)) {
		word[index++] = cpy[base++];
	}

	word[index + 1] = '\0';
	free(cpy);

	return word;
}

/**
 * Returns the next line in the file as a cleaned string.
 * Leading and trailing whitespace will be removed, as will comments.
 *
 * @param file The file to be read from.
 * @return A string representation of the next line in the file.
 */
char * read_next_line(FILE *file) {
	if(file != NULL) {
		char *line = (char *)malloc(100);

		if(fgets(line, 100, file) != NULL)
			return clean_str(line);
	}

	return NULL;
}

/**
 * The main workhorse function that goes through a block, line by line, and returns when it hits
 * a closing }. This essentially writes straigh-line assembly code.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param stack The current status of the stack at this point in the code.
 * @return The last line read which includes the closing }.
 */
char * read_block(FILE *input_file, FILE *output_file, Stack *stack, Block_ct *block_ct) {
	char *line = read_next_line(input_file);
	char *first_var = NULL;

	String_list *local_vars = (String_list *)malloc(LIST_LEN * sizeof(String_list));

	while(strchr(line, '}') < 0 && strstr(line, "return") < 0) {
		first_var = read_word(line);

		if(strstr(first_var, "int") >= 0) { // Is the line a variable definition?
			line += strlen(first_var) + 1;

			first_var = read_word(line);
			string_set_add(&local_vars, first_var);

			line += strlen(first_var) + 1;

			while(line[0] == ' ')
				line++;

			if(line[0] == ';') //If at end of statement, go to next line. Nothing to be done.
				continue;
		}
	}

	return line;
}

/**
 * Takes the header line of a function and returns an ordered list of the names of the parameters.
 *
 * @param stack The current status of the memory stack.
 * @param headline The header line for the function.
 * @return Ordered list of the parameters from bottom of the stack to the top.
 */
char **read_func_header(Stack *stack, char *headline) {
	char *parameter_line = strchr(headline, '(') + 1; // The header line starting with the (
	char *parameter = NULL; //Holds the next parameter's name
	char **parameter_list; //List of all the parameters
	int par_ct = str_inst_ct(parameter_line, "int "); //Parameter count

	parameter_list = (char **)malloc(par_ct * sizeof(char *));
	for(int i = 0; i < par_ct; i++)
		parameter_list[i] = (char *)malloc(100);

	par_ct = 0;

	while((parameter_line = strstr(parameter_line, "int "))) {
		parameter_line += 4;

		parameter = read_word(parameter_line);
		strcpy(parameter_list[par_ct++], parameter);
		free(parameter);
	}

	return parameter_list;
}

/**
 * Reads an if block and writes the needed assembly code to the file.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 * @param headline String representation of the header line of this if statement.
 * @param stack The current status of the stack at this point in the code.
 * @return The last line read, that being the first line outside of the if statement.
 * This is necessary as it must check the next line for an else.
 */
char * read_if_block(FILE *input_file, FILE *output_file, Block_ct *block_ct, char *headline, Stack *stack) {
	char *line = strchr(headline, '(') + 1;
	char *first = read_word(headline);
	int first_is_var = 0;
	char *second = NULL;
	int second_is_var = 0;
	char op[3];
	Comparison comp;

	headline += strlen(first);

	while(headline[0] == ' ')
		headline++;

	op[0] = headline[0];
	op[1] = headline[1];
	op[2] = '\0';

	headline += 2;

	if(strcmp(op, "==") == 0)
		comp = EQUAL;
	else if(strcmp(op, "!=") == 0)
		comp = NOT_EQUAL;
	else if(strcmp(op, "<=") == 0)
		comp = LESSER_EQ;
	else if(strcmp(op, ">=") == 0)
		comp = GREATER_EQ;
	else if(op[0] == '>')
		comp = GREATER;
	else
		comp = LESSER;

	while(headline[0] == ' ')
		headline++;

	second = read_word(headline);
	headline += strlen(second);

	first_is_var = char_is_letter(first[0]);
	second_is_var = char_is_letter(second[0]);

	if(!first_is_var && !second_is_var) { //Constant expression
		switch(comp) {
		case EQUAL:
			if(str_to_int(first) != str_to_int(second)) { //Check if the execution will ever be run
				int open_ct = 1;
				if(strchr(line, '{') < 0)
					line = read_next_line(input_file);

				while(open_ct >= 0) { //Loop until finding the close of the if statement
					if(strchr(line, '}') >= 0) {
						open_ct--;
						if(open_ct < 0)
							break;
					}

					if(strchr(line, '{') >= 0) {
						open_ct++;
					}
				}
			}
			break;
		case NOT_EQUAL:
			break;
		case LESSER:
			break;
		case GREATER:
			break;
		case LESSER_EQ:
			break;
		case GREATER_EQ:
			break;
		}
	}

	return NULL;
}

/**
 * Generates the assembly code for the function starting at the current point in the file.
 *
 * @param input_file File that is being compiled.
 * @param output_file Assembly file that is being written.
 * @param headline String representation of the header line of this function.
 * @param block_ct Keeps track of the number of each type of block so as to give each unique names.
 */
void read_func(FILE *input_file, FILE *output_file, char *headline, Block_ct *block_ct) {
	Stack stack = {NULL, 0};
	char **parameter_list = read_func_header(&stack, headline);
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
	char c;

	input_file = fopen(filename, "r");
	output_file = fopen(strcat(filename, ".asm"), "w");

	fprintf(output_file, "\tpushi main\n\tjpop\n");

	int ct = 1;
	while((line = read_next_line(input_file))) {
		#ifdef DEBUG
		printf("-----Parsing line %d-----\n", ct++);
		#endif

		if(strlen(line) > 1) {
			first_word = read_word(line);
			printf("Read word %s.\n", first_word);

			if(first_word == NULL) { //Line is empty
				#ifdef DEBUG
				printf("Read empty line.\n");
				#endif

				continue;
			}

			if(strstr(first_word, "int") == first_word) { //Found an int function
				#ifdef DEBUG
				printf("Found integer function %s.\n", read_word(line + 4));
				#endif

				if(strlen(line) > 4) {
					fprintf(output_file, "%s:\n", read_word(line + 4));
					read_func(input_file, output_file, line, &block_ct);
				}
			} else if(strstr(first_word, "void") == first_word) { //Found a void function
				#ifdef DEBUG
				printf("Found void function %s.\n", read_word(line + 5));
				#endif

				if(strlen(line) > 5) {
					fprintf(output_file, "%s:\n", read_word(line + 5));
					read_func(input_file, output_file, line, &block_ct);
				}
			}
		} else {
            #ifdef DEBUG
			printf("Read empty line.\n");
            #endif
		}
	}

    return 0;
}
