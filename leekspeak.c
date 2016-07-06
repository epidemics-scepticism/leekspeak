/*
    Copyright (C) 2016 cacahuatl < cacahuatl at autistici dot org >

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include "onion.h"
#include "uthash.h" /* https://raw.githubusercontent.com/troydhanson/uthash/master/src/uthash.h */

#define xfree(x) { if (x) { free(x); } x = NULL; }

static u64 word_count = 0;

typedef struct {
	u8 *w;
	u16 v;
	UT_hash_handle hw;
	UT_hash_handle hv;
} WORD_LIST;

static WORD_LIST *words = NULL, *values = NULL;

static u8
populate_word_list(void)
{
	u8 s[512];
	u8 *ptr;
	FILE *wordfile = NULL;
	WORD_LIST *tmp = NULL;

	wordfile = fopen("words", "r");
	if (!wordfile) goto error;
	while (fgets((char *)s, sizeof(s), wordfile) && word_count < 65536) {
		ptr = (u8 *)strchr((const char *)s, '\n');
		if (ptr) *ptr = 0;
		tmp = calloc(1, sizeof(WORD_LIST));
		if (!tmp) goto error;
		tmp->v = (u16)word_count++;
		tmp->w = (u8 *)strdup((const char *)s);
		if (!tmp->w) goto error;
		HASH_ADD_KEYPTR(hw, words, tmp->w, strlen((const char *)tmp->w), tmp);
		HASH_ADD(hv, values, v, sizeof(u16), tmp);
		tmp = NULL;
	}
	fclose(wordfile);
	return 1;
error:
	warn("populate_word_list");
	return 0;
}

static void
depopulate_word_list(void)
{
	WORD_LIST *ptr = NULL, *tmp = NULL;
	HASH_ITER(hw, words, ptr, tmp) {
		HASH_DELETE(hw, words, ptr);
		HASH_DELETE(hv, values, ptr);
		xfree(ptr->w);
		xfree(ptr);
	}
}

static u16
by_word(const u8 *word)
{
	if (!word) {
		warnx("%s: %s", "by_word", "NULL pointer");
		return 0;
	}
	WORD_LIST *tmp = NULL;
	HASH_FIND(hw, words, word, strlen((const char *)word), tmp);
	if (tmp) return tmp->v;
	else {
		warnx("%s: %s", "by_word", "Invalid word");
		return 0;
	}
}

static u8 *
by_value(const u16 value)
{
	WORD_LIST *tmp = NULL;
	HASH_FIND(hv, values, &value, sizeof(u16), tmp);
	if (tmp) return tmp->w;
	else { /* this can only happen is the wordlist is underpopulated... */
		warnx("%s: %s", "by_value", "Invalid value");
		return NULL;
	}
}

static void
onion_to_phrase(const u8 *onion)
{
	u8 raw_onion[10] = { 0 };
	u16 *x = (u16 *)&raw_onion[0];
	onion_decode(raw_onion, onion);
	for (u64 i = 0; i < 5; i++) {
		printf("%s ", by_value(x[i]));
	}
	printf("\n");
}

static void
phrase_to_onion(const u8 **phrase)
{
	u8 raw_onion[10] = { 0 }, onion[17] = { 0 };
	u16 *x = (u16 *)&raw_onion[0];
	for (u64 i = 0; phrase[i] && i < 5; i++) {
		x[i] = by_word(phrase[i]);
	}
	onion_encode(onion, raw_onion);
	printf("%s.onion\n", onion);
}

int
main(int argc, char **argv)
{
	if (argc < 3) goto usage;
	if (!populate_word_list()) goto end;
	if ('e' == argv[1][0]) {
		onion_value_init();
		char *p = strchr(argv[2], '.');
		if (p) *p = 0;
		onion_to_phrase((const u8 *)argv[2]);
	} else if ('d' == argv[1][0]) {
		phrase_to_onion((const u8 **)&argv[2]);
	} else goto usage;
end:
	depopulate_word_list();
	return 0;
usage:
	printf("Usage: %s [(e)ncode/(d)ecode] [phrase/onion]\n", argv[0]);
	printf("%s encode facebookcorewwwi.onion\n", argv[0]);
	printf("%s decode redate sheilas lifesome dowable tontiners\n", argv[0]);
	return -1;
}
