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
 * See md5.c for more information.
 */

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#elif !defined(_MD5_H)
#define _MD5_H

#define MD5_DIGEST_LENGTH    16

/* Any 32-bit or wider unsigned integer data type will do */
typedef unsigned int MD5_u32plus;

typedef struct {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
} MD5_CTX;

#if defined(__cplusplus)
#define PROPER_EXPORT extern "C"
#else
#define PROPER_EXPORT extern 
#endif

PROPER_EXPORT void MD5_Init(MD5_CTX *ctx);
PROPER_EXPORT void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size);
PROPER_EXPORT void MD5_Final(unsigned char *result, MD5_CTX *ctx);

#endif

#else
/*********************************************************************
* Filename: md5.h
* Author: Brad Conte (brad AT bradconte.com)
* Copyright:
* Disclaimer: This code is presented "as is" without any guarantees.
* Details: Defines the API for the corresponding MD5 implementation.
*********************************************************************/
#ifndef MD5_H
#define MD5_H
/*************************** HEADER FILES ***************************/
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
#define PROPER_EXPORT extern "C"
#else
#define PROPER_EXPORT extern 
#endif

/****************************** MACROS ******************************/
#define MD5_BLOCK_SIZE 16 // MD5 outputs a 16 byte digest
#define MD5_DIGEST_LENGTH    MD5_BLOCK_SIZE
/**************************** DATA TYPES ****************************/
//typedef unsigned char BYTE; // 8-bit byte
//typedef unsigned int WORD; // 32-bit word, change to "long" for 16-bit machines

typedef struct {
	uint8_t data[64];
	uint32_t datalen;
	unsigned long long bitlen;
	uint32_t state[4];
} MD5_CTX;
/*********************** FUNCTION DECLARATIONS **********************/
PROPER_EXPORT void md5_init(MD5_CTX *ctx);
PROPER_EXPORT void md5_update(MD5_CTX *ctx, const uint8_t data[], size_t len);
PROPER_EXPORT void md5_final(MD5_CTX *ctx, uint8_t hash[]);

#define MD5_Init md5_init
#define MD5_Update md5_update
#define MD5_Final md5_final

#endif // MD5_H
#endif