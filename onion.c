#include "onion.h"

static const u8 onion_chars[32] = {
	'a','b','c','d','e','f','g','h',
	'i','j','k','l','m','n','o','p',
	'q','r','s','t','u','v','w','x',
	'y','z','2','3','4','5','6','7'
};

static u8 onion_values[256] = { 0 };

static u8
onion_char(u8 c)
{
	return onion_chars[c & 0x1f];
}

static u8
onion_value(u8 c)
{
	return onion_values[c];
}

// 0        1        2        3        4
// 12345678 12345678 12345678 12345678 12345678
// 11010000 01111010 01000100 11110010 11000000 1001001110000001011010110110110101011110
// 11010000 01111010 00110100 11110010 11000000 1001001110000001011010110110110101011110
// 12345123 45123451 23451234 51234512 34512345 
// 0    1     2    3     4     5    6     7

// 0        1        2        3        4
// 12345678 12345678 12345678 12345678 12345678
// 10101010 10101010 10101010 10101010 10101010
// 12345123 45123451 23451234 51234512 34512345
// 0    1     2    3     4     5    6     7

static void
onion_decode_block(u8 *dst, const u8 *src)
{
	dst[0]  = onion_value(src[0]) << 3 & 0xf8;
	dst[0] |= onion_value(src[1]) >> 2 & 0x07;
	dst[1]  = onion_value(src[1]) << 6 & 0xc0;
	dst[1] |= onion_value(src[2]) << 1 & 0x3e;
	dst[1] |= onion_value(src[3]) >> 4 & 0x01;
	dst[2]  = onion_value(src[3]) << 4 & 0xf0;
	dst[2] |= onion_value(src[4]) >> 1 & 0x0f;
	dst[3]  = onion_value(src[4]) << 7 & 0x80;
	dst[3] |= onion_value(src[5]) << 2 & 0x7c;
	dst[3] |= onion_value(src[6]) >> 3 & 0x3;
	dst[4]  = onion_value(src[6]) << 5 & 0xe0;
	dst[4] |= onion_value(src[7]) & 0x1f;
}

static void
onion_encode_block(u8 *dst, const u8 *src)
{
	dst[0] = onion_char(src[0] >> 3);
	dst[1] = onion_char(src[0] << 2 | src[1] >> 6);
	dst[2] = onion_char(src[1] >> 1);
	dst[3] = onion_char(src[1] << 4 | src[2] >> 4);
	dst[4] = onion_char(src[2] << 1 | src[3] >> 7);
	dst[5] = onion_char(src[3] >> 2);
	dst[6] = onion_char(src[3] << 3 | src[4] >> 5);
	dst[7] = onion_char(src[4]);
}

void
onion_value_init(void)
{
	for (u64 i = 0; i < 32; i++)
		onion_values[onion_chars[i]] = i;
}

void
onion_decode(u8 *dst, const u8 *src)
{
	onion_decode_block(&dst[0], &src[0]);
	onion_decode_block(&dst[5], &src[8]);
}

void
onion_encode(u8 *dst, const u8 *src)
{
	onion_encode_block(&dst[0], &src[0]);
	onion_encode_block(&dst[8], &src[5]);
}
