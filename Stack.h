#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/** @struct Stack
 * Holds the current contents of the memory stack.
 */
typedef struct {
	char **names; ///< The list of strings representing the parameters in order.
	int size; ///< The current length of the stack.
} Stack;

void stack_push(Stack *stack, char *name);
char * stack_pop(Stack *stack);
void print_stack(Stack stack);
