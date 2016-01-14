#include <string.h>
#include <stdlib.h>

#define LIST_LEN 100

/** @struct String_list
 * Holds a short list of strings representing variables.
 * Should be used in an array as a string_set.
 */
typedef struct {
	char **data; ///< The list of strings in this list.
	int length; ///< The current length of the list.
} String_list;

int hash_func(char *str);
void new_string_list(String_list *list);

void string_list_append(String_list *list, char *str);
int string_list_remove_str(String_list *list, char *str);
int string_list_contains(String_list list, char *str);
void free_string_list(String_list *list);

void string_set_add(String_list **set, char *str);
int string_set_remove_str(String_list **set, char *str);
int string_set_contains(String_list *set, char *str);
void free_string_set(String_list **set);
