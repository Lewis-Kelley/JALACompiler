#include "StringList.h"

/**
 * Returns a has of the passed string.
 * Thank you CSSE230 for bringing this up.
 *
 * @param str The string to be hashed. Will not be modified.
 * @return Hash code representing str.
 */
int hash_func(char *str) {
	int hash = 2739;
	int c;

	while ((c = *str++))
		hash = hash * 33 + c;

	return hash % LIST_LEN;
}

/**
 * Adds a copy of the passed string into the list.
 *
 * @param list The list to be edited.
 * @param str The string to be added.
 * @param addr The address where the variable named by str is stored.
 */
void string_list_append(String_list *list, char *str, int addr) {
	if(list->length == 0) {
		list->keys = (char **)malloc(sizeof(char *));
		list->addrs = (int *)malloc(sizeof(int));
	} else {
		list->keys = (char **)realloc(list->keys, (list->length + 1) * sizeof(char *));
		list->addrs = (int *)realloc(list->addrs, (list->length + 1) * sizeof(char *));
	}

	list->keys[list->length] = (char *)malloc(100);
	list->keys[list->length][0] = '\0';

	strcpy(list->keys[list->length], str);
	list->addrs[list->length] = addr;
	list->length++;
}

/**
 * Removes the passed string from the list and returns a value if it was successful.
 *
 * @param list The list to be edited.
 * @param str The string to be removed.
 * @return The address for the variable if successful, 0 otherwise.
 */
int string_list_remove_str(String_list *list, char *str) {
	for(int i = 0; i < list->length; i++) {
		if(strcmp(list->keys[i], str) == 0) {
			int ret_addr = list->addrs[i];
			free(list->keys[i]);

			if(list->length == 1) {
				free(list->keys);
				free(list->addrs);
			} else {
				for(; i < list->length - 1; i++) {
					list->keys[i] = list->keys[i + 1];
					list->addrs[i] = list->addrs[i + 1];
				}

				list->keys = (char **)realloc(list->keys, (list->length - 1) * sizeof(char *));
				list->addrs = (int *)realloc(list->addrs, (list->length - 1) * sizeof(int));
			}

			list->length--;

			return ret_addr;
		}
	}

	return 0;
}

/**
 * Checks if the given string is in the string list.
 *
 * @param list The string list to be searched through.
 * @param str The string to be searched for.
 * @return The address associated with str if str is in list, 0 otherwise.
 */
int string_list_lookup(String_list list, char *str) {
	for(int i = 0; i < list.length; i++)
		if(strcmp(list.keys[i], str))
			return list.addrs[i];

	return 0;
}

/**
 * Frees all the memory used by this list.
 *
 * @param list String list to be emptied.
 */
void free_string_list(String_list *list) {
	if(list == NULL)
		return;

	for(int i = 0; i < list->length; i++) {
		free(list->keys[i]);
	}

	if(list->keys != NULL)
		free(list->keys);
	if(list->addrs != NULL)
		free(list->addrs);
}

/**
 * Adds a copy of the passed string into the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be added to the set.
 * @param addr The address where the variable named by str is stored.
 */
void string_set_add(String_list **set, char *str, int addr) {
	string_list_append(*set + hash_func(str) * sizeof(String_list), str, addr);
}

/**
 * Removes the passed string from the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be removed from the set.
 * @return The memory address of str if the removal is successful, 0 otherwise.
 */
int string_set_remove_str(String_list **set, char *str) {
	return string_list_remove_str(set[hash_func(str)], str);
}

/**
 * Checks if the string set contains a string matching the passed string.
 *
 * @param set The string set to be searched through.
 * @param str The string to be searched for.
 * @return The memory address of str if the string is contained in the set.
 */
int string_set_contains(String_list *set, char *str) {
	return string_list_lookup(set[hash_func(str)], str);
}

/**
 * Frees all the memory used by this set.
 *
 * @param set String set to be emptied.
 */
void free_string_set(String_list *set) {
	if(set == NULL)
		return;

	for(int i = 0; i < LIST_LEN; i++) {
		free_string_list(set + i * sizeof(String_list));
		free(set + i * sizeof(String_list));
	}
}

/**
 * Prints the passed string list. Only used for debugging.
 *
 * @param list The string list to be printed.
 */
void print_string_list(String_list list) {
	for(int i = 0; i < list.length; i++) {
		printf("\t---%d\n", i);
		printf("\t%s:\n\t%#x\n", list.keys[i], list.addrs[i]);
	}
}

/**
 * Prints the passed string set. Only used for debugging.
 *
 * @param set The string set to be printed.
 */
void print_string_set(String_list *set) {
	printf("---------------------------\n");
	for(int i = 0; i < LIST_LEN; i++) {
		printf("---%d\n", i);
		printf("\tL: %d\n", (set + i * sizeof(String_list))->length);
		print_string_list(*(set + i * sizeof(String_list)));
	}
	printf("---------------------------\n");
}
