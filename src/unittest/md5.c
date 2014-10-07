#if 0
/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * (This is a heavily cut-down "BSD license".)
 *
 * This differs from Colin Plumb's older public domain implementation in that
 * no exactly 32-bit integer data type is required (any 32-bit or wider
 * unsigned integer data type will do), there's no compile-time endianness
 * configuration, and the function prototypes match OpenSSL's.  No code from
 * Colin Plumb's implementation has been reused; this comment merely compares
 * the properties of the two independent implementations.
 *
 * The primary goals of this implementation are portability and ease of use.
 * It is meant to be fast, but not as fast as possible.  Some known
 * optimizations are not included to reduce source code size and avoid
 * compile-time configuration.
 */

#ifndef HAVE_OPENSSL

#include <string.h>

#include "md5.h"

/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z)			((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)			((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)			(((x) ^ (y)) ^ (z))
#define H2(x, y, z)			((x) ^ ((y) ^ (z)))
#define I(x, y, z)			((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s) \
	(a) += f((b), (c), (d)) + (x) + (t); \
	(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
	(a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 *
 * The check for little-endian architectures that tolerate unaligned
 * memory accesses is just an optimization.  Nothing will break if it
 * doesn't work.
 */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define SET(n) \
	(*(MD5_u32plus *)&ptr[(n) * 4])
#define GET(n) \
	SET(n)
#else
#define SET(n) \
	(ctx->block[(n)] = \
	(MD5_u32plus)ptr[(n) * 4] | \
	((MD5_u32plus)ptr[(n) * 4 + 1] << 8) | \
	((MD5_u32plus)ptr[(n) * 4 + 2] << 16) | \
	((MD5_u32plus)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
	(ctx->block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There are no alignment requirements.
 */
static const void *body(MD5_CTX *ctx, const void *data, unsigned long size)
{
	const unsigned char *ptr;
	MD5_u32plus a, b, c, d;
	MD5_u32plus saved_a, saved_b, saved_c, saved_d;

	ptr = (const unsigned char *)data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

/* Round 1 */
		STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
		STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
		STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
		STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
		STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
		STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
		STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
		STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
		STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
		STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
		STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
		STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
		STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
		STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
		STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
		STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

/* Round 2 */
		STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
		STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
		STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
		STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
		STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
		STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
		STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
		STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
		STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
		STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
		STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
		STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
		STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
		STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
		STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
		STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

/* Round 3 */
		STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
		STEP(H2, d, a, b, c, GET(8), 0x8771f681, 11)
		STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
		STEP(H2, b, c, d, a, GET(14), 0xfde5380c, 23)
		STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
		STEP(H2, d, a, b, c, GET(4), 0x4bdecfa9, 11)
		STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
		STEP(H2, b, c, d, a, GET(10), 0xbebfbc70, 23)
		STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
		STEP(H2, d, a, b, c, GET(0), 0xeaa127fa, 11)
		STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
		STEP(H2, b, c, d, a, GET(6), 0x04881d05, 23)
		STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
		STEP(H2, d, a, b, c, GET(12), 0xe6db99e5, 11)
		STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
		STEP(H2, b, c, d, a, GET(2), 0xc4ac5665, 23)

/* Round 4 */
		STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
		STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
		STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
		STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
		STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
		STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
		STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
		STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
		STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
		STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
		STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
		STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
		STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
		STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
		STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
		STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

void MD5_Init(MD5_CTX *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size)
{
	MD5_u32plus saved_lo;
	unsigned long used, available;

	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
		ctx->hi++;
	ctx->hi += size >> 29;

	used = saved_lo & 0x3f;

	if (used) {
		available = 64 - used;

		if (size < available) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}

		memcpy(&ctx->buffer[used], data, available);
		data = (const unsigned char *)data + available;
		size -= available;
		body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = body(ctx, data, size & ~(unsigned long)0x3f);
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

void MD5_Final(unsigned char *result, MD5_CTX *ctx)
{
	unsigned long used, available;

	used = ctx->lo & 0x3f;

	ctx->buffer[used++] = 0x80;

	available = 64 - used;

	if (available < 8) {
		memset(&ctx->buffer[used], 0, available);
		body(ctx, ctx->buffer, 64);
		used = 0;
		available = 64;
	}

	memset(&ctx->buffer[used], 0, available - 8);

	ctx->lo <<= 3;
	ctx->buffer[56] = ctx->lo;
	ctx->buffer[57] = ctx->lo >> 8;
	ctx->buffer[58] = ctx->lo >> 16;
	ctx->buffer[59] = ctx->lo >> 24;
	ctx->buffer[60] = ctx->hi;
	ctx->buffer[61] = ctx->hi >> 8;
	ctx->buffer[62] = ctx->hi >> 16;
	ctx->buffer[63] = ctx->hi >> 24;

	body(ctx, ctx->buffer, 64);

	result[0] = ctx->a;
	result[1] = ctx->a >> 8;
	result[2] = ctx->a >> 16;
	result[3] = ctx->a >> 24;
	result[4] = ctx->b;
	result[5] = ctx->b >> 8;
	result[6] = ctx->b >> 16;
	result[7] = ctx->b >> 24;
	result[8] = ctx->c;
	result[9] = ctx->c >> 8;
	result[10] = ctx->c >> 16;
	result[11] = ctx->c >> 24;
	result[12] = ctx->d;
	result[13] = ctx->d >> 8;
	result[14] = ctx->d >> 16;
	result[15] = ctx->d >> 24;

	memset(ctx, 0, sizeof(*ctx));
}

#endif
#else
/*********************************************************************
* Filename: md5.c
* Author: Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details: Implementation of the MD5 hashing algorithm.
Algorithm specification can be found here:
* http://tools.ietf.org/html/rfc1321
This implementation uses little endian byte order.
*********************************************************************/
/*************************** HEADER FILES ***************************/
#include <stdlib.h>
#include <memory.h>
#include "md5.h"
/****************************** MACROS ******************************/
#define ROTLEFT(a,b) ((a << b) | (a >> (32-b)))
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))
#define FF(a,b,c,d,m,s,t) { a += F(b,c,d) + m + t; \
a = b + ROTLEFT(a,s); }
#define GG(a,b,c,d,m,s,t) { a += G(b,c,d) + m + t; \
a = b + ROTLEFT(a,s); }
#define HH(a,b,c,d,m,s,t) { a += H(b,c,d) + m + t; \
a = b + ROTLEFT(a,s); }
#define II(a,b,c,d,m,s,t) { a += I(b,c,d) + m + t; \
a = b + ROTLEFT(a,s); }
/*********************** FUNCTION DEFINITIONS ***********************/
void md5_transform(MD5_CTX *ctx, const uint8_t data[])
{
	uint32_t a, b, c, d, m[16], i, j;
	// MD5 specifies big endian byte order, but this implementation assumes a little
	// endian byte order CPU. Reverse all the bytes upon input, and re-reverse them
	// on output (in md5_final()).
	for (i = 0, j = 0; i < 16; ++i, j += 4)
		m[i] = (data[j]) + (data[j + 1] << 8) + (data[j + 2] << 16) + (data[j + 3] << 24);
	a = ctx->state[0];
	b = ctx->state[1];
	c = ctx->state[2];
	d = ctx->state[3];
	FF(a, b, c, d, m[0], 7, 0xd76aa478);
	FF(d, a, b, c, m[1], 12, 0xe8c7b756);
	FF(c, d, a, b, m[2], 17, 0x242070db);
	FF(b, c, d, a, m[3], 22, 0xc1bdceee);
	FF(a, b, c, d, m[4], 7, 0xf57c0faf);
	FF(d, a, b, c, m[5], 12, 0x4787c62a);
	FF(c, d, a, b, m[6], 17, 0xa8304613);
	FF(b, c, d, a, m[7], 22, 0xfd469501);
	FF(a, b, c, d, m[8], 7, 0x698098d8);
	FF(d, a, b, c, m[9], 12, 0x8b44f7af);
	FF(c, d, a, b, m[10], 17, 0xffff5bb1);
	FF(b, c, d, a, m[11], 22, 0x895cd7be);
	FF(a, b, c, d, m[12], 7, 0x6b901122);
	FF(d, a, b, c, m[13], 12, 0xfd987193);
	FF(c, d, a, b, m[14], 17, 0xa679438e);
	FF(b, c, d, a, m[15], 22, 0x49b40821);
	GG(a, b, c, d, m[1], 5, 0xf61e2562);
	GG(d, a, b, c, m[6], 9, 0xc040b340);
	GG(c, d, a, b, m[11], 14, 0x265e5a51);
	GG(b, c, d, a, m[0], 20, 0xe9b6c7aa);
	GG(a, b, c, d, m[5], 5, 0xd62f105d);
	GG(d, a, b, c, m[10], 9, 0x02441453);
	GG(c, d, a, b, m[15], 14, 0xd8a1e681);
	GG(b, c, d, a, m[4], 20, 0xe7d3fbc8);
	GG(a, b, c, d, m[9], 5, 0x21e1cde6);
	GG(d, a, b, c, m[14], 9, 0xc33707d6);
	GG(c, d, a, b, m[3], 14, 0xf4d50d87);
	GG(b, c, d, a, m[8], 20, 0x455a14ed);
	GG(a, b, c, d, m[13], 5, 0xa9e3e905);
	GG(d, a, b, c, m[2], 9, 0xfcefa3f8);
	GG(c, d, a, b, m[7], 14, 0x676f02d9);
	GG(b, c, d, a, m[12], 20, 0x8d2a4c8a);
	HH(a, b, c, d, m[5], 4, 0xfffa3942);
	HH(d, a, b, c, m[8], 11, 0x8771f681);
	HH(c, d, a, b, m[11], 16, 0x6d9d6122);
	HH(b, c, d, a, m[14], 23, 0xfde5380c);
	HH(a, b, c, d, m[1], 4, 0xa4beea44);
	HH(d, a, b, c, m[4], 11, 0x4bdecfa9);
	HH(c, d, a, b, m[7], 16, 0xf6bb4b60);
	HH(b, c, d, a, m[10], 23, 0xbebfbc70);
	HH(a, b, c, d, m[13], 4, 0x289b7ec6);
	HH(d, a, b, c, m[0], 11, 0xeaa127fa);
	HH(c, d, a, b, m[3], 16, 0xd4ef3085);
	HH(b, c, d, a, m[6], 23, 0x04881d05);
	HH(a, b, c, d, m[9], 4, 0xd9d4d039);
	HH(d, a, b, c, m[12], 11, 0xe6db99e5);
	HH(c, d, a, b, m[15], 16, 0x1fa27cf8);
	HH(b, c, d, a, m[2], 23, 0xc4ac5665);
	II(a, b, c, d, m[0], 6, 0xf4292244);
	II(d, a, b, c, m[7], 10, 0x432aff97);
	II(c, d, a, b, m[14], 15, 0xab9423a7);
	II(b, c, d, a, m[5], 21, 0xfc93a039);
	II(a, b, c, d, m[12], 6, 0x655b59c3);
	II(d, a, b, c, m[3], 10, 0x8f0ccc92);
	II(c, d, a, b, m[10], 15, 0xffeff47d);
	II(b, c, d, a, m[1], 21, 0x85845dd1);
	II(a, b, c, d, m[8], 6, 0x6fa87e4f);
	II(d, a, b, c, m[15], 10, 0xfe2ce6e0);
	II(c, d, a, b, m[6], 15, 0xa3014314);
	II(b, c, d, a, m[13], 21, 0x4e0811a1);
	II(a, b, c, d, m[4], 6, 0xf7537e82);
	II(d, a, b, c, m[11], 10, 0xbd3af235);
	II(c, d, a, b, m[2], 15, 0x2ad7d2bb);
	II(b, c, d, a, m[9], 21, 0xeb86d391);
	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
}
void md5_init(MD5_CTX *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
}
void md5_update(MD5_CTX *ctx, const uint8_t data[], size_t len)
{
	size_t i;
	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen] = data[i];
		ctx->datalen++;
		if (ctx->datalen == 64) {
			md5_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}
void md5_final(MD5_CTX *ctx, uint8_t hash[])
{
	size_t i;
	i = ctx->datalen;
	// Pad whatever data is left in the buffer.
	if (ctx->datalen < 56) {
		ctx->data[i++] = 0x80;
		while (i < 56)
			ctx->data[i++] = 0x00;
	}
	else if (ctx->datalen >= 56) {
		ctx->data[i++] = 0x80;
		while (i < 64)
			ctx->data[i++] = 0x00;
		md5_transform(ctx, ctx->data);
		memset(ctx->data, 0, 56);
	}
	// Append to the padding the total message's length in bits and transform.
	ctx->bitlen += ctx->datalen * 8;
	ctx->data[56] = ctx->bitlen;
	ctx->data[57] = ctx->bitlen >> 8;
	ctx->data[58] = ctx->bitlen >> 16;
	ctx->data[59] = ctx->bitlen >> 24;
	ctx->data[60] = ctx->bitlen >> 32;
	ctx->data[61] = ctx->bitlen >> 40;
	ctx->data[62] = ctx->bitlen >> 48;
	ctx->data[63] = ctx->bitlen >> 56;
	md5_transform(ctx, ctx->data);
	// Since this implementation uses little endian byte ordering and MD uses big endian,
	// reverse all the bytes when copying the final state to the output hash.
	for (i = 0; i < 4; ++i) {
		hash[i] = (ctx->state[0] >> (i * 8)) & 0x000000ff;
		hash[i + 4] = (ctx->state[1] >> (i * 8)) & 0x000000ff;
		hash[i + 8] = (ctx->state[2] >> (i * 8)) & 0x000000ff;
		hash[i + 12] = (ctx->state[3] >> (i * 8)) & 0x000000ff;
	}
}
#endif