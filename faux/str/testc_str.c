#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "faux/str.h"


int testc_faux_str_nextword(void)
{
	const char* line = "asd\"\\\"\"mmm \"``\" `ll\"l\\p\\\\m```j`j`` ```kk``pp``` ll\\ l jj\\\"kk ll\\\\nn  \"aaa\"bbb`ccc```ddd``eee ``lk\\\"";
	const char* etalon[] = {
		"asd\"mmm",
		"``",
		"ll\"l\\p\\\\mj`j",
		"kk``pp",
		"ll l",
		"jj\"kk",
		"ll\\nn",
		"aaabbbcccdddeee",
		"lk\\\"", // Unclosed quotes
		NULL
		 };
	int retval = 0;
	int i = 0;
	const char *saveptr = line;
	bool_t closed_quotes = BOOL_FALSE;

	printf("Line   : [%s]\n", line);

	for (i = 0; etalon[i]; i++) {
		int r = -1;
		char *res = NULL;
		printf("Etalon %d : [%s]\n", i, etalon[i]);
		res = faux_str_nextword(saveptr, &saveptr, "`", &closed_quotes);
		if (!res) {
			printf("The faux_str_nextword() return value is NULL\n");
			break;
		} else {
			printf("Result %d : [%s]\n", i, res);
		}
		r = strcmp(etalon[i], res);
		if (r < 0) {
			printf("Not equal %d\n", i);
			retval = -1;
		}
		faux_str_free(res);
	}
	// Last quote is unclosed
	if (closed_quotes) {
		printf("Closed quotes flag is wrong\n");
		retval = -1;
	} else {
		printf("Really unclosed quotes\n");
	}

	return retval;
}


int testc_faux_str_getline(void)
{
	const char* line = "arg 0\narg 1\narg 2";
	const char* etalon[] = {
		"arg 0",
		"arg 1",
		"arg 2",
		NULL
		 };
	size_t num_etalon = 3;
	size_t index = 0;
	char *str = NULL;
	const char *saveptr = NULL;

	printf("Line   : [%s]\n", line);

	saveptr = line;
	while ((str = faux_str_getline(saveptr, &saveptr)) && (index < num_etalon)) {
		int r = -1;
		printf("Etalon %ld : [%s]\n", index, etalon[index]);
		r = strcmp(etalon[index], str);
		if (r < 0) {
			printf("Not equal %ld [%s]\n", index, str);
			return -1;
		}
		faux_str_free(str);
		index++;
	}
	if (index != num_etalon) {
		printf("Number of args is not equal real=%ld etalon=%ld\n", index, num_etalon);
		return -1;
	}

	return 0;
}


int testc_faux_str_numcmp(void)
{
	if (faux_str_numcmp("abc2", "abc10") >= 0) {
		printf("'abc2' >= 'abc10'\n");
		return -1;
	}

	if (faux_str_numcmp("abc2ccc", "abc10ccc") >= 0) {
		printf("'abc2ccc' >= 'abc10ccc'\n");
		return -1;
	}

	if (faux_str_numcmp("abc2ccc", "abcaccc") >= 0) {
		printf("'abc2ccc' >= 'abcaccc'\n");
		return -1;
	}

	if (faux_str_numcmp("abc222222222222222ccc", "abc222222222222222cdc") >= 0) {
		printf("'abc222222222222222ccc' >= 'abc222222222222222cdc'\n");
		return -1;
	}

	// Overflow
	if (faux_str_numcmp("abc222222222222222222222222222222ccc", "abc1022222222222222222222222222222ccc") <= 0) {
		printf("'abc222222222222222222222222222222ccc' <= 'abc1022222222222222222222222222222ccc'\n");
		return -1;
	}

	return 0;
}


int testc_faux_str_c_esc_quote(void)
{
	char *src = NULL;
	char *etalon = NULL;
	char *esc = NULL;

	src = "aaa\\bbb\"";
	etalon = "aaa\\\\bbb\\\"";
	esc = faux_str_c_esc_quote(src);
	if (strcmp(esc, etalon) != 0) {
		printf("Problem with string without spaces\n");
		printf("src=[%s], etalon=[%s], esc=[%s]\n",
			src, etalon, esc);
		return -1;
	}
	faux_str_free(esc);

	src = "aaa\\ bbb\"";
	etalon = "\"aaa\\\\ bbb\\\"\"";
	esc = faux_str_c_esc_quote(src);
	if (strcmp(esc, etalon) != 0) {
		printf("Problem with string with spaces\n");
		printf("src=[%s], etalon=[%s], esc=[%s]\n",
			src, etalon, esc);
		return -1;
	}
	faux_str_free(esc);

	return 0;
}


int testc_faux_str_casecmpn(void)
{
	const char *line_long = "abcdefj";
	const char *line_short = "abcde";
	const char *line_diff = "abche";

	if (faux_str_casecmpn(line_long, line_short, strlen(line_short)) != 0) {
		printf("Equal string comparison problem\n");
		return -1;
	}

	if (faux_str_casecmpn(line_long, line_short, strlen(line_long)) <= 0) {
		printf("Sort string comparison problem\n");
		return -1;
	}

	if (faux_str_casecmpn(line_long, line_diff, strlen(line_diff)) >= 0) {
		printf("Different string comparison problem\n");
		return -1;
	}

	if (faux_str_casecmpn(line_long, NULL, strlen(line_long)) <= 0) {
		printf("Broken first string comparison with NULL\n");
		return -1;
	}

	if (faux_str_casecmpn(NULL, line_long, strlen(line_long)) >= 0) {
		printf("Broken second string comparison with NULL\n");
		return -1;
	}

	return 0;
}
