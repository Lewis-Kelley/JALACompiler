#include <stdlib.h>
#include <string.h>

/** @struct Stack
 * Holds the current contents of the memory stack.
 */
typedef struct {
	char **names; ///< The list of strings representing the parameters in order.
	int size; ///< The current length of the stack.
} Stack;

void stack_push(Stack *stack, char *name);
char * stack_pop(Stack *stack);
char * stack_shallow_peek(Stack stack);
char * stack_deep_peek(Stack stack);
