#include "Stack.h"

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

	strcpy(stack->names[stack->size++], name);
}

/**
 * Pops the next value off the given stack.
 *
 * @param stack Stack to be added to. Note this *will* be altered.
 * @return The value at the top of the stack
 */
char * stack_pop(Stack *stack) {
	char * ret;
	if(stack->size == 0)
		return 0;

	ret = stack->names[--stack->size];

	if(stack->size == 1)
		stack->names = NULL;
	else
		stack->names = (char **)realloc(stack->names, (stack->size) * sizeof(char *));

	return ret;
}

/**
 * Returns the next value off the given stack without editing the stack.
 *
 * @param stack Stack to be added to.
 * @return The value at the top of the stack
 */
char * stack_shallow_peek(Stack stack) {
	if(stack.size == 0)
		return NULL;

	return stack.names[stack.size - 1];
}

/**
 * Returns the second deepest value off the given stack without editing the stack.
 *
 * @param stack Stack to be added to.
 * @return The value at below the top of the stack
 */
char * stack_deep_peek(Stack stack) {
	if(stack.size <= 1)
		return NULL;

	return stack.names[stack.size - 2];
}
