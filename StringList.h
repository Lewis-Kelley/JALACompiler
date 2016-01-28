#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define LIST_LEN 100
#define STR_LEN 100

/** @struct String_list
 * Holds a short list of strings representing variables.
 * Should be used in an array as a string_set.
 */
typedef struct {
	char keys[LIST_LEN][STR_LEN]; ///< The list of strings in this list.
	int addrs[LIST_LEN]; ///< The memory address reserved for this variable
	int length; ///< The current length of the list.
} String_list;

int hash_func(char *str);
void new_string_list(String_list *list);

void string_list_append(String_list *list, char str[], int addr);
int string_list_remove_str(String_list *list, char str[]);
int string_list_lookup(String_list list, char *str);

void string_set_init(String_list *set[]);
void string_set_clean(String_list *set[]);
void string_set_add(String_list set[], char str[], int addr);
int string_set_remove_str(String_list set[], char str[]);
int string_set_contains(String_list set[], char str[]);
void string_set_cpy(String_list dest[], String_list orig[]);

void print_string_list(String_list list);
void print_string_set(String_list set[]);
