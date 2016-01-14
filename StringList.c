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
 */
void string_list_append(String_list *list, char *str) {
	if(list->length == 0)
		list->data = (char **)malloc(sizeof(char *));
	else
		list->data = (char **)realloc(list->data, (list->length + 1) * sizeof(char *));

	strcpy(list->data[list->length], str);
	list->length++;
}

/**
 * Removes the passed string from the list and returns a value if it was successful.
 *
 * @param list The list to be edited.
 * @param str The string to be removed.
 * @return 1 if successful, 0 otherwise.
 */
int string_list_remove_str(String_list *list, char *str) {
	for(int i = 0; i < list->length; i++) {
		if(strcmp(list->data[i], str) == 0) {
			free(list->data[i]);

			if(list->length == 1)
				free(list->data);
			else {
				for(; i < list->length - 1; i++) {
					list->data[i] = list->data[i + 1];
				}

				list->data = (char **)realloc(list->data, (list->length - 1) * sizeof(char *));
			}

			list->length--;

			return 1;
		}
	}

	return 0;
}

/**
 * Checks if the given string is in the string list.
 *
 * @param list The string list to be searched through.
 * @param str The string to be searched for.
 * @return 1 if str is in list, 0 otherwise.
 */
int string_list_contains(String_list list, char *str) {
	for(int i = 0; i < list.length; i++)
		if(strcmp(list.data[i], str))
			return 1;

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
		free(list->data[i]);
	}

	free(list);
}

/**
 * Adds a copy of the passed string into the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be added to the set.
 */
void string_set_add(String_list **set, char *str) {
	string_list_append(set[hash_func(str)], str);
}

/**
 * Removes the passed string from the set.
 *
 * @param set The string set to be edited.
 * @param str The string to be removed from the set.
 * @return 1 if the removal is successful, 0 otherwise.
 */
int string_set_remove_str(String_list **set, char *str) {
	return string_list_remove_str(set[hash_func(str)], str);
}

/**
 * Checks if the string set contains a string matching the passed string.
 *
 * @param set The string set to be searched through.
 * @param str The string to be searched for.
 * @return 1 if the string is contained in the set.
 */
int string_set_contains(String_list *set, char *str) {
	return string_list_contains(set[hash_func(str)], str);
}

/**
 * Frees all the memory used by this set.
 *
 * @param set String set to be emptied.
 */
void free_string_set(String_list **set) {
	if(set == NULL)
		return;

	for(int i = 0; i < LIST_LEN; i++) {
		free_string_list(set[i]);
	}

	free(set);
}
