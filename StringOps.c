#include "StringOps.h"

/**
 * Returns 1 if the passed character is a letter. Returns 0 otherwise.
 */
int char_is_letter(char input) {
	if(input > 64 && input < 91)
		return 1;
	if(input > 96 && input < 123)
		return 1;

	return 0;
}

/**
 * Returns the number of instances of a given string in another.
 *
 * @param base String to be searched through.
 * @param search String to be searched for.
 * @return Number of times the search string appears in the base string.
 */
int str_inst_ct(char *base, char *search) {
	if(base == NULL)
		return 0;

	int ct = 0;
	char *cpy = (char *)malloc(100);

	strcpy(cpy, base);

	while(strstr(cpy + 4 * ct, search)) {
		ct++;
	}

	free(cpy);
	return ct;
}

/**
 * Returns the value of the string as an integer.
 * e.g. "123" -> 123
 *
 * @return Integer value of the given string.
 */
int str_to_int(char *str) {
	int pow10 = 1;
	int len = strlen(str);
	int res = 0;

	for(int i = 0; i < len; i++) {
		pow10 *= 10;
	}

	for(int i = 0; i < len; i++) {
		res += pow10 * (str[i] - 48); //-48 converts to number
		pow10 /= 10;
	}

	return res;
}

/**
 * Removes the leading and trailing whitespace from the passed string.
 * Also removes text past a //.
 *
 * @param str String to be cleaned up.
 * @return Returns the passed string for reference, but the passed string itself is altered too.
 */
char * clean_str(char *str) {
	int base = 0;
	int len = strlen(str);
	int top;

	if(strstr(str, "//")) {
		len -= strlen(strstr(str, "//")) + 1;
		str = (char *)realloc(str, len + 2);
		str[len + 1] = '\n';
		str[len + 2] = '\0';
		len += 2;
	}

	top = len - 1;

	while(base < len - 1 && (str[base] == ' ' || str[base] == '\t' || str[base] == '\n'))
		base++;

	while(top > base && (str[top - 1] == ' ' || str[top - 1] == '\t' || str[top - 1] == '\n'))
		top--;

	for(int i = 0; i < len - base - 1; i++)
		str[i] = str[i + base];

	len = top - base;
	str = (char *)realloc(str, len + 1);
	str[len] = '\0';

	return str;
}
