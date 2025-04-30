/** @file argv.c
 * @brief Functions to parse string to arguments.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "private.h"
#include "faux/faux.h"
#include "faux/str.h"
#include "faux/list.h"
#include "faux/argv.h"


/** @brief Allocates new argv object.
 *
 * Before working with argument list it must be allocated and initialized.
 *
 * @return Allocated and initialized argument list or NULL on error.
 */
faux_argv_t *faux_argv_new(void)
{
	faux_argv_t *fargv = NULL;

	fargv = faux_zmalloc(sizeof(*fargv));
	assert(fargv);
	if (!fargv)
		return NULL;

	// Init
	fargv->list = faux_list_new(FAUX_LIST_UNSORTED, FAUX_LIST_NONUNIQUE,
		NULL, NULL, (void (*)(void *))faux_str_free);
	fargv->quotes = NULL;
	fargv->continuable = BOOL_FALSE;

	return fargv;
}


/** @brief Duplicate existing argv object.
 *
 * @param [in] fargv Allocated and initialized argv object.
 * @return Allocated and initialized duplicate or NULL on error.
 */
faux_argv_t *faux_argv_dup(const faux_argv_t *origin)
{
	faux_argv_t *fargv = NULL;
	faux_list_t *list = NULL;
	faux_argv_node_t *iter = NULL;
	const char *arg = NULL;

	assert(origin);
	if (!origin)
		return NULL;

	fargv = faux_argv_new();
	assert(fargv);
	if (!fargv)
		return NULL;

	// Copy all fields but list must be recreated
	list = fargv->list;
	*fargv = *origin;
	fargv->list = list;

	// Copy list
	iter = faux_argv_iter(origin);
	while ((arg = faux_argv_each(&iter)))
		faux_argv_add(fargv, arg);

	return fargv;
}


/** @brief Frees the argv object object.
 *
 * After using the argv object must be freed. Function frees argv object.
 */
void faux_argv_free(faux_argv_t *fargv)
{
	if (!fargv)
		return;

	faux_list_free(fargv->list);
	faux_str_free(fargv->quotes);
	faux_free(fargv);
}


/** @brief Initializes iterator to iterate through the entire argv object.
 *
 * Before iterating with the faux_argv_each() function the iterator must be
 * initialized. This function do it.
 *
 * @param [in] fargv Allocated and initialized argv object.
 * @return Initialized iterator.
 * @sa faux_argv_each()
 */
faux_argv_node_t *faux_argv_iter(const faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return NULL;

	return (faux_argv_node_t *)faux_list_head(fargv->list);
}


/** @brief Initializes revert iterator to iterate through the entire argv object.
 *
 * Before iterating with the faux_argv_eachr() function the iterator must be
 * initialized. This function do it.
 *
 * @param [in] fargv Allocated and initialized argv object.
 * @return Initialized iterator.
 * @sa faux_argv_eachr()
 */
faux_argv_node_t *faux_argv_iterr(const faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return NULL;

	return (faux_argv_node_t *)faux_list_tail(fargv->list);
}


/** @brief Iterate entire argv object for arguments.
 *
 * Before iteration the iterator must be initialized by faux_argv_iter()
 * function. Doesn't use faux_argv_each() with uninitialized iterator.
 *
 * On each call function returns string (argument) and modifies iterator.
 * Stop iteration when function returns NULL.
 *
 * @param [in,out] iter Iterator.
 * @return String.
 * @sa faux_argv_iter()
 */
const char *faux_argv_each(faux_argv_node_t **iter)
{
	return (const char *)faux_list_each((faux_list_node_t **)iter);
}


/** @brief Reverse iterate entire argv object for arguments.
 *
 * Before iteration the iterator must be initialized by faux_argv_iterr()
 * function. Doesn't use faux_argv_eachr() with uninitialized iterator.
 *
 * On each call function returns string (argument) and modifies iterator.
 * Stop iteration when function returns NULL.
 *
 * @param [in,out] iter Iterator.
 * @return String.
 * @sa faux_argv_iterr()
 */
const char *faux_argv_eachr(faux_argv_node_t **iter)
{
	return (const char *)faux_list_eachr((faux_list_node_t **)iter);
}


/** @brief Get current argument by iterator.
 *
 * Before iteration the iterator must be initialized by faux_argv_iter()
 * function. Doesn't use faux_argv_current() with uninitialized iterator.
 *
 * Function doesn't modify iterator.
 *
 * @param [in] iter Iterator.
 * @return String.
 * @sa faux_argv_iter()
 */
const char *faux_argv_current(faux_argv_node_t *iter)
{
	return (const char *)faux_list_data((faux_list_node_t *)iter);
}


/** @brief Sets alternative quotes list.
 *
 * Any character from specified string becomes alternative quote.
 *
 * @param [in] fargv Allocated fargv object.
 * @param [in] quotes String with symbols to consider as a quote.
 */
void faux_argv_set_quotes(faux_argv_t *fargv, const char *quotes)
{
	assert(fargv);
	if (!fargv)
		return;

	faux_str_free(fargv->quotes);
	if (!quotes) {
		fargv->quotes = NULL; // No additional quotes
		return;
	}
	fargv->quotes = faux_str_dup(quotes);
}


/** @brief Parse string to words and quoted substrings.
 *
 * Parse string to words and quoted substrings. Additionally function sets
 * continuable flag. It shows if last word is reliable ended i.e. it can't be
 * continued.
 *
 * @param [in] fargv Allocated fargv object.
 * @param [in] str String to parse.
 * @return Number of resulting words and substrings or < 0 on error.
 */
ssize_t faux_argv_parse(faux_argv_t *fargv, const char *str)
{
	const char *saveptr = str;
	char *word = NULL;
	bool_t closed_quotes = BOOL_FALSE;

	assert(fargv);
	if (!fargv)
		return -1;
	if (!str)
		return -1;

	while ((word = faux_str_nextword(saveptr, &saveptr, fargv->quotes, &closed_quotes)))
		faux_list_add(fargv->list, word);

	// Check if last argument can be continued
	// It's true if last argument has unclosed quotes.
	// It's true if last argument doesn't terminated by space.
	fargv->continuable = !closed_quotes || ((saveptr != str) && (!isspace(*(saveptr - 1))));

	return faux_list_len(fargv->list);
}


/** @brief Get number of arguments.
 *
 * @param [in] fargv Allocated fargv object.
 * @return Number of words and substrings or < 0 on error.
 */
ssize_t faux_argv_len(const faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return -1;

	return faux_list_len(fargv->list);
}


/** @brief Returns continuable flag.
 *
 * Can be used after faux_argv_parse() only.
 *
 * @sa faux_argv_parse()
 * @param [in] fargv Allocated fargv object.
 * @return Boolean continuable flag.
 */
bool_t faux_argv_is_continuable(const faux_argv_t *fargv)
{
	assert(fargv);
	if (!fargv)
		return BOOL_FALSE;

	return fargv->continuable;
}


/** @brief Sets continuable flag.
 *
 * @param [in] fargv Allocated fargv object.
 * @param [in] continuable Continuable flag to set.
 */
void faux_argv_set_continuable(faux_argv_t *fargv, bool_t continuable)
{
	assert(fargv);
	if (!fargv)
		return;

	fargv->continuable = continuable;
}


/** @brief Remove last uncompleted entry.
 *
 * If argv is continuable consider last entry as uncompleted and remove it.
 *
 * @param [in] fargv Allocated fargv object.
 */
void faux_argv_del_continuable(faux_argv_t *fargv)
{
	faux_list_node_t *tail = NULL;

	assert(fargv);
	if (!fargv)
		return;

	if (!fargv->continuable)
		return;
	tail = faux_list_tail(fargv->list);
	if (!tail)
		return;
	faux_list_del(fargv->list, tail);
}


/** @brief If given node is last one.
 *
 * @param [in] iter Iterator/Node.
 * @return BOOL_TRUE if last, BOOL_FALSE - not last or error.
 */
bool_t faux_argv_is_last(faux_argv_node_t *iter)
{
	faux_list_node_t *node = (faux_list_node_t *)iter;

	if (!node)
		return BOOL_FALSE;
	if (faux_list_next_node(node) == NULL)
		return BOOL_TRUE;

	return BOOL_FALSE;
}


/** @brief Adds argument to fargv object.
 *
 * @param [in] fargv Allocated argv object.
 * @param [in] arg Argument to add.
 * @return BOOL_TRUE - success, BOOL_FALSE - error.
 */
bool_t faux_argv_add(faux_argv_t *fargv, const char *arg)
{
	assert(fargv);
	if (!fargv)
		return BOOL_FALSE;
	assert(arg);
	if (!arg)
		return BOOL_FALSE;

	faux_list_add(fargv->list, faux_str_dup(arg));

	return BOOL_TRUE;
}


/** @brief Deletes argument from fargv object.
 *
 * @param [in] fargv Allocated argv object.
 * @param [in] node Node to delete.
 * @return BOOL_TRUE - success, BOOL_FALSE - error.
 */
bool_t faux_argv_del(faux_argv_t *fargv, faux_argv_node_t *node)
{
	assert(fargv);
	if (!fargv)
		return BOOL_FALSE;
	assert(node);
	if (!node)
		return BOOL_FALSE;

	faux_list_del(fargv->list, (faux_list_node_t *)node);

	return BOOL_TRUE;
}


/** @brief Gets argument by index.
 *
 * @param [in] fargv Allocated argv object.
 * @return String or NULL on error.
 */
const char *faux_argv_index(const faux_argv_t *fargv, size_t index)
{
	const char *res = NULL;

	assert(fargv);
	if (!fargv)
		return NULL;

	res = (const char *)faux_list_index(fargv->list, index);

	return res;
}


/** @brief Gets whole text line (concatinated arguments).
 *
 * TODO: Now args with spaces is printed simply with quotes. It must be fixed
 * later because arg can contain quotes itself.
 *
 * @param [in] fargv Allocated argv object.
 * @return String or NULL on error.
 */
char *faux_argv_line(const faux_argv_t *fargv)
{
	bool_t is_first_arg = BOOL_TRUE;
	char *line = NULL;
	faux_argv_node_t *iter = NULL;
	const char *arg = NULL;

	iter = faux_argv_iter(fargv);
	while ((arg = faux_argv_each(&iter))) {
		bool_t space_found = BOOL_FALSE;
		char *str = NULL;

		if (is_first_arg)
			is_first_arg = BOOL_FALSE;
		else
			faux_str_cat(&line, " ");
		str = faux_str_c_esc_quote(arg);
		faux_str_cat(&line, str);
		faux_str_free(str);
	}

	return line;
}
