#include "Stack.h"
#include "StringList.h"

/**
 * Adds the passed value to the given stack.
 *
 * @param stack Stack to be added to. Note this *will* be altered.
 * @param name Name to be added to stack.
 */
void stack_push(Stack *stack, char *name) {
	if(stack->size == 0)
		stack->names = (char **)malloc(sizeof(char*));
	else
		stack->names = (char **)realloc(stack->names, (stack->size + 1) * sizeof(char *));
	stack->names[stack->size] = (char *)malloc(100);

	strcpy(stack->names[stack->size++], name);
}

/**
 * Pops the next value off the given stack.
 *
 * @param stack Stack to be added to. Note this *will* be altered.
 * @return The value at the top of the stack
 */
char * stack_pop(Stack *stack) {
	char *ret = (char*)malloc(STR_LEN);
	if(stack->size == 0)
		return 0;

	strcpy(ret, stack->names[--stack->size]);

	free(stack->names[stack->size]);

	if(stack->size == 0)
		free(stack->names);
	else
		stack->names = (char **)realloc(stack->names, (stack->size) * sizeof(char *));

	return ret;
}

/**
 * Prints out the data on the stack.
 * Only used for debugging purposes.
 */
void print_stack(Stack stack) {
	int index = stack.size - 1;
	printf("---TOP---\n");
	while(index >= 0) {
		printf("%s\n", stack.names[index--]);
	}
	printf("---BOT---\n");
}
