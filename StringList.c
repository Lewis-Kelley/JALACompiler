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

	if(hash < 0)
		hash *= -1;

	return hash % LIST_LEN;
}

/**
 * Adds a copy of the passed string into the list.
 *
 * @param list The list to be edited.
 * @param str The string to be added.
 * @param addr The address where the variable named by str is stored.
 */
void string_list_append(String_list *list, char str[], int addr) {
	printf("Starting string_list_append.\n");
	printf("list->length: %d\n", list->length);

	list->keys[list->length][0] = '\0';

	strcpy(list->keys[list->length], str);
	list->addrs[list->length] = addr;
	list->length++;
	printf("Finishing string_list_append.\n");
}

/**
 * Removes the passed string from the list and returns a value if it was successful.
 *
 * @param list The list to be edited.
 * @param str The string to be removed.
 * @return The address for the variable if successful, 0 otherwise.
 */
int string_list_remove_str(String_list *list, char str[]) {
	for(int i = 0; i < list->length; i++) {
		if(strcmp(list->keys[i], str) == 0) {
			int ret_addr = list->addrs[i];

			for(; i < list->length - 1; i++) {
				strcpy(list->keys[i], list->keys[i + 1]);
				list->addrs[i] = list->addrs[i + 1];
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
int string_list_lookup(String_list list, char str[]) {
	for(int i = 0; i < list.length; i++)
		if(strcmp(list.keys[i], str))
			return list.addrs[i];

	return 0;
}

/**
 * Adds a copy of the passed string into the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be added to the set.
 * @param addr The address where the variable named by str is stored.
 */
void string_set_add(String_list set[], char str[], int addr) {
	int hash = hash_func(str);
	printf("got hash: %d\n", hash);
	string_list_append(&(set[hash_func(str)]), str, addr);
	printf("list at index %d has length %d\n", hash, set[hash].length);
}

/**
 * Removes the passed string from the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be removed from the set.
 * @return The memory address of str if the removal is successful, 0 otherwise.
 */
int string_set_remove_str(String_list set[], char str[]) {
	return string_list_remove_str(&(set[hash_func(str)]), str);
}

/**
 * Checks if the string set contains a string matching the passed string.
 *
 * @param set The string set to be searched through.
 * @param str The string to be searched for.
 * @return The memory address of str if the string is contained in the set.
 */
int string_set_contains(String_list *set, char str[]) {
	return string_list_lookup(*(set + hash_func(str) * sizeof(String_list)), str);
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
void print_string_set(String_list set[]) {
	printf("---------------------------\n");
	for(int i = 0; i < LIST_LEN; i++) {
		printf("---%d\n", i);
		printf("\tL: %d\n", set[i].length);
		print_string_list(set[i]);
	}
	printf("---------------------------\n");
}
