/** Unit tests for array-based list
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2016, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of VILLASnode. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited.
 *********************************************************************************/

#include <stdint.h>
#include <string.h>
#include <criterion/criterion.h>

#include "utils.h"
#include "list.h"

static char *nouns[] = { "time", "person", "year", "way", "day", "thing", "man", "world", "life", "hand", "part", "child", "eye", "woman", "place", "work", "week", "case", "point", "government", "company", "number", "group", "problem", "fact" };

struct data {
	char *tag;
	int data;
};

Test(list, list_lookup)
{
	struct list l;
	
	list_init(&l);
	
	for (int i = 0; i < ARRAY_LEN(nouns); i++) {
		struct data *d = malloc(sizeof(struct data));
		
		d->tag = nouns[i];
		d->data = i;
		
		list_push(&l, d);
	}
	
	struct data *found = list_lookup(&l, "woman");
	
	cr_assert_eq(found->data, 13);
	
	list_destroy(&l, NULL, true);
}

Test(list, list_search)
{
	struct list l;
	
	list_init(&l);
	
	/* Fill list */
	for (int i = 0; i < ARRAY_LEN(nouns); i++)
		list_push(&l, nouns[i]);

	/* Declare on stack! */
	char positive[] = "woman";
	char negative[] = "dinosaurrier";
	
	char *found = (char *) list_search(&l, (cmp_cb_t) strcmp, positive);
	
	cr_assert_not_null(found);
	cr_assert_eq(found, nouns[13]);
	cr_assert_str_eq(found, positive);
	
	char *not_found = (char *) list_search(&l, (cmp_cb_t) strcmp, negative);

	cr_assert_null(not_found);

	list_destroy(&l, NULL, false);
}

struct content {
	int destroyed;
};

static void dtor(void *ptr)
{
	struct content *elm = (struct content *) ptr;
	
	elm->destroyed = 1;
}

Test(list, destructor)
{
	struct list l;
	struct content elm = { .destroyed = 0 };
	
	list_init(&l);
	list_push(&l, &elm);
	
	cr_assert_eq(list_length(&l), 1);
	
	list_destroy(&l, dtor, false);

	cr_assert_eq(elm.destroyed, 1);
}

static int compare(const void *a, const void *b) {
	return b - a;
}

Test(list, basics)
{
	intptr_t i;
	struct list l;
	
	list_init(&l);
	
	for (i = 0; i < 100; i++) {
		cr_assert_eq(list_length(&l), i);
		
		list_push(&l, (void *) i);
	}

	cr_assert_eq(list_at(&l, 555), NULL);
	cr_assert_eq(list_last(&l), (void *) 99);
	cr_assert_eq(list_first(&l), NULL);

	i = 0;
	list_foreach (void *j, &l)
		cr_assert_eq(j, (void *) i++);
	
	list_sort(&l, compare); /* Reverse list */
	
	i = 99;
	list_foreach (void *j, &l) {
		cr_assert_eq(j, (void *) i, "Is %p, expected %p", i, j);
		i--;
	}

	cr_assert(list_contains(&l, (void *) 55));
	
	list_remove(&l, (void *) 55);
	
	cr_assert(!list_contains(&l, (void *) 55));

	list_destroy(&l, NULL, false);

	cr_assert_eq(list_length(&l), -1, "List not properly destroyed: l.length = %zd", l.length);
}