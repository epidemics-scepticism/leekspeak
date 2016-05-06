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
#include "uthash.h" /* https://raw.githubusercontent.com/troydhanson/uthash/master/src/uthash.h */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint64_t u64;

#define xfree(x) { if (x) { free(x); } x = NULL; }

static const u8 onion_digits[33] = "abcdefghijklmnopqrstuvwxyz234567";

static u8 onion_char(u8 c)
{
	return onion_digits[c & 0x1f];
}

static u8 onion_value(u8 c)
{
	u8 *p = strchr(onion_digits, c);
	if (p) return (u8)(p - onion_digits);
	else return 0;
}

static u8 *onion_decode(u8 *in)
{
	u8 *out = NULL;
	u64 i = 0, j, x, out_sz = 0;

	if (!in || 16 != strlen(in)) goto end;
	out = calloc(10, sizeof(u8));
	if (!out) goto end;
	while (i < 16) {
		x = 0;
		while (i < 16) {
			x |= (u64)onion_value(tolower(in[i])) << (7 - i % 8) * 5;
			if (!(++i % 8)) break;
		}
		for (j = 0; j < 5; j++) {
			out[out_sz++] = x >> ((4 - j) * 8);
		}
	}
end:
	return out;
}

static u8 *onion_encode(u8 *in)
{
	u8 *out = NULL;
	u64 i = 0, j, x, out_sz = 0;

	if (!in) goto end;
	out = calloc(17, sizeof(u8));
	if (!out) goto end;
	while (i < 10) {
		x = 0;
		while (i < 10) {
			x |= (u64)in[i] << ((4 - i % 5) * 8);
			if (!(++i % 5)) break;
		}
		for (j = 0; j < 8; j++) {
			out[out_sz++] = onion_char(x >> (7 - j) * 5);
		}
	}
end:
	return out;
}

static u64 word_count = 0;

typedef struct {
	u8 *w;
	u16 v;
	UT_hash_handle hw;
	UT_hash_handle hv;
} WORD_LIST;

static WORD_LIST *words = NULL, *values = NULL;

static u8 populate_word_list(void)
{
	u8 s[512];
	u8 *ptr;
	FILE *wordfile = NULL;
	WORD_LIST *tmp = NULL, *tmp_holder = NULL;
	u64 i;

	wordfile = fopen("words", "r");
	if (!wordfile) goto error;
	while (fgets(s, sizeof(s), wordfile) && word_count < 65536) {
		ptr = strchr(s, '\n');
		if (ptr) *ptr = 0;
		tmp = calloc(1, sizeof(WORD_LIST));
		if (!tmp) goto error;
		tmp->v = (u16)word_count++;
		tmp->w = strdup(s);
		if (!tmp->w) goto error;
		HASH_ADD_KEYPTR(hw, words, tmp->w, strlen(tmp->w), tmp);
		HASH_ADD(hv, values, v, sizeof(u16), tmp);
		tmp = NULL;
	}
	fclose(wordfile);
	return 1;
error:
	warn("populate_word_list");
	return 0;
}

static void depopulate_word_list(void)
{
	WORD_LIST *ptr = NULL, *tmp = NULL;
	HASH_ITER(hw, words, ptr, tmp) {
		HASH_DELETE(hw, words, ptr);
		HASH_DELETE(hv, values, ptr);
		xfree(ptr->w);
		xfree(ptr);
	}
}

static u16 by_word(const u8 *word)
{
	if (!word) return 0;
	WORD_LIST *tmp = NULL;
	HASH_FIND(hw, words, word, strlen(word), tmp);
	if (tmp) return tmp->v;
	else return 0;
}

static u8 *by_value(const u16 value)
{
	WORD_LIST *tmp = NULL;
	HASH_FIND(hv, values, &value, sizeof(u16), tmp);
	if (tmp) return tmp->w;
	else return NULL;
}

static void onion_to_phrase(u8 *onion)
{
	if (!onion) goto end;
	u8 *raw_onion;
	u64 i;
	u16 x;

	raw_onion = onion_decode(onion);
	if (!raw_onion) goto end;
	for (i = 0; i < 5; i++) {
		x = *(u16 *)&raw_onion[i * 2];
		printf("%s ", by_value(x));
	}
	puts("");
	xfree(raw_onion);
end:
	return;
}

static void phrase_to_onion(u8 *phrase)
{
	if (!phrase) goto end;
	u8 raw_onion[10] = { 0 }, *onion = NULL;
	u16 x;
	u64 i = 0;
	u8 *word = strtok(phrase, " \n");

	do {
		x = by_word(word);
		*(u16 *)&raw_onion[i++ * 2] = x;
	} while((word = strtok(NULL, " \n")));
	onion = onion_encode(raw_onion);
	printf("%s.onion\n", onion);
	xfree(onion);
end:
	return;
}

int main(int argc, char **argv)
{
	if (argc < 3) goto end;
	if (!populate_word_list()) goto end;
	if ('e' == argv[1][0]) {
		onion_to_phrase(strtok(argv[2], "."));
	} else if ('d' == argv[1][0]) {
		phrase_to_onion(argv[2]);
	}
end:
	depopulate_word_list();
	return 0;
}
