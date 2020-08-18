/*-
 * Copyright 2005 Colin Percival
 * Copyright 2013 Christian Mehlis & René Kijewski
 * Copyright 2016 Martin Landsmann <martin.landsmann@haw-hamburg.de>
 * Copyright 2016 OTA keys S.A.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: src/lib/libmd/sha256c.c,v 1.2 2006/01/17 15:35:56 phk Exp $
 */

/**
 * @ingroup     sys_hashes
 * @{
 *
 * @file
 * @brief       SHA256 hash function implementation
 *
 * @author      Lena Boeckmann
 *
 * @}
 */

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "vendor/MKW21D5.h"
#include "hashes/sha256.h"
#include "sha256_hwctx.h"
#include "mmcau.h"
#include "cau_api.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifdef __BIG_ENDIAN__
/* Copy a vector of big-endian uint32_t into a vector of bytes */
#define be32enc_vect memcpy

/* Copy a vector of bytes into a vector of big-endian uint32_t */
#define be32dec_vect memcpy

#else /* !__BIG_ENDIAN__ */

/*
 * Encode a length len/4 vector of (uint32_t) into a length len vector of
 * (unsigned char) in big-endian form.  Assumes len is a multiple of 4.
 */
static void be32enc_vect(void *dst_, const void *src_, size_t len)
{
    if ((uintptr_t)dst_ % sizeof(uint32_t) == 0 &&
        (uintptr_t)src_ % sizeof(uint32_t) == 0) {
        uint32_t *dst = dst_;
        const uint32_t *src = src_;
        for (size_t i = 0; i < len / 4; i++) {
            dst[i] = __builtin_bswap32(src[i]);
        }
    }
    else {
        uint8_t *dst = dst_;
        const uint8_t *src = src_;
        for (size_t i = 0; i < len; i += 4) {
            dst[i] = src[i + 3];
            dst[i + 1] = src[i + 2];
            dst[i + 2] = src[i + 1];
            dst[i + 3] = src[i];
        }
    }
}

/*
 * Decode a big-endian length len vector of (unsigned char) into a length
 * len/4 vector of (uint32_t).  Assumes len is a multiple of 4.
 */
#define be32dec_vect be32enc_vect

#endif /* __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__ */

/* SHA-256 initialization.  Begins a SHA-256 operation. */
void sha256_init(sha256_context_t *ctx)
{
    DEBUG("SHA256 init HW accelerated implementation\n");
    cau_sha256_initialize_output(ctx->sha256_state);
}

/* ATTENTION – The following padding function has been copy-pasted from a Freescale coding example, which can be downloaded here: https://www.nxp.com/docs/en/application-note-software/AN4307SW.zip

The padding function implemented in RIOT can't be separated from the hashing function. To use the mmCAU hashing functions a separate padding function is needed. */
static void *mmcau_sha_pad(const void *data, unsigned int *len, unsigned char endianess)
{
    unsigned char *padding_array;
    signed char padding_length;
    unsigned int temp_length;
    unsigned int bits_length;
    unsigned int final_length;

    temp_length = *len % SHA256_INTERNAL_BLOCK_SIZE;

    /*get padding length: padding special case when 448 mod 512*/
    /*working with bytes rather than bits*/
    if( !((padding_length = 56-(temp_length%SHA256_INTERNAL_BLOCK_SIZE)) > 0) )
        padding_length = SHA256_INTERNAL_BLOCK_SIZE - (temp_length%56);

    padding_length +=  temp_length/SHA256_INTERNAL_BLOCK_SIZE;
    temp_length = *len;

    /*reserve necessary memory*/
    final_length = temp_length + padding_length + 8/*bits length*/;
    if( (padding_array = (unsigned char *)malloc(final_length)) == NULL )
        return (unsigned char *)NULL;/*not enough mem*/

    /*copy original data to new padding array*/
    memcpy((void*)padding_array,data,temp_length);

    /*add padding*/
    padding_array[temp_length++] = 0x80;/*first bit enabled*/
    while((--padding_length != 0))
        padding_array[temp_length++] = 0;/*clear the rest*/

    /*add length depending on endianess: get number of bits*/
    bits_length = *len << 3;
    *len = final_length;

    if( endianess == 0 )
    {
        padding_array[temp_length++] = bits_length     & 0xff;
        padding_array[temp_length++] = bits_length>>8  & 0xff;
        padding_array[temp_length++] = bits_length>>16 & 0xff;
        padding_array[temp_length++] = bits_length>>24 & 0xff;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length  ] = 0;
    }
    else/*CRYPTO_BIG_ENDIAN*/
    {
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = 0;
        padding_array[temp_length++] = bits_length>>24 & 0xff;
        padding_array[temp_length++] = bits_length>>16 & 0xff;
        padding_array[temp_length++] = bits_length>>8  & 0xff;
        padding_array[temp_length  ] = bits_length     & 0xff;
    }

    return padding_array;
}

static void mmcau_sha256(unsigned int *state, const void *data, unsigned int len)
{
    unsigned char *sha256_padded;
    sha256_padded = mmcau_sha_pad(data, &len, 1);
    int blocks = len/SHA256_INTERNAL_BLOCK_SIZE;
    cau_sha256_update(sha256_padded, blocks, state);

    free(sha256_padded);
}

/* Add bytes into the hash */
void sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
   mmcau_sha256(ctx->sha256_state, data, len);
}

/*
 * SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 */
void sha256_final(sha256_context_t *ctx, void *dst)
{
    /* Write the hash */
    be32enc_vect(dst, ctx->sha256_state, SHA256_DIGEST_LENGTH);

    /* Clear the context state */
    memset((void *) ctx, 0, sizeof(*ctx));
}



void *sha256(const void *data, size_t len, void *digest)
{
    sha256_context_t ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);

    return digest;
}

void hmac_sha256_init(hmac_context_t *ctx, const void *key, size_t key_length)
{
    unsigned char k[SHA256_INTERNAL_BLOCK_SIZE];

    memset((void *)k, 0x00, SHA256_INTERNAL_BLOCK_SIZE);

    if (key_length > SHA256_INTERNAL_BLOCK_SIZE) {
        sha256(key, key_length, k);
    }
    else {
        memcpy((void *)k, key, key_length);
    }

    /*
     * create the inner and outer keypads
     * rising hamming distance enforcing i_* and o_* are distinct
     * in at least one bit
     */
    unsigned char o_key_pad[SHA256_INTERNAL_BLOCK_SIZE];
    unsigned char i_key_pad[SHA256_INTERNAL_BLOCK_SIZE];

    for (size_t i = 0; i < SHA256_INTERNAL_BLOCK_SIZE; ++i) {
        o_key_pad[i] = 0x5c ^ k[i];
        i_key_pad[i] = 0x36 ^ k[i];
    }

    /*
     * Initiate calculation of the inner hash
     * tmp = hash(i_key_pad CONCAT message)
     */
    sha256_init(&ctx->c_in);
    sha256_update(&ctx->c_in, i_key_pad, SHA256_INTERNAL_BLOCK_SIZE);

    /*
     * Initiate calculation of the outer hash
     * result = hash(o_key_pad CONCAT tmp)
     */
    sha256_init(&ctx->c_out);
    sha256_update(&ctx->c_out, o_key_pad, SHA256_INTERNAL_BLOCK_SIZE);

}

void hmac_sha256_update(hmac_context_t *ctx, const void *data, size_t len)
{
    sha256_update(&ctx->c_in, data, len);
}

void hmac_sha256_final(hmac_context_t *ctx, void *digest)
{
    unsigned char tmp[SHA256_DIGEST_LENGTH];

    static unsigned char m[SHA256_DIGEST_LENGTH];

    if (digest == NULL) {
        digest = m;
    }

    sha256_final(&ctx->c_in, tmp);
    sha256_update(&ctx->c_out, tmp, SHA256_DIGEST_LENGTH);
    sha256_final(&ctx->c_out, digest);
}

const void *hmac_sha256(const void *key, size_t key_length,
                        const void *data, size_t len, void *digest)
{

    hmac_context_t ctx;

    hmac_sha256_init(&ctx, key, key_length);
    hmac_sha256_update(&ctx,data, len);
    hmac_sha256_final(&ctx, digest);

    return digest;
}

/**
 * @brief helper to compute sha256 inplace for the given buffer
 *
 * @param[in, out] element the buffer to compute a sha256 and store it back to it
 *
 */
static inline void sha256_inplace(unsigned char element[SHA256_DIGEST_LENGTH])
{
    sha256_context_t ctx;

    sha256_init(&ctx);
    sha256_update(&ctx, element, SHA256_DIGEST_LENGTH);
    sha256_final(&ctx, element);
}

void *sha256_chain(const void *seed, size_t seed_length,
                   size_t elements, void *tail_element)
{
    unsigned char tmp_element[SHA256_DIGEST_LENGTH];

    /* assert if no sha256-chain can be created */
    assert(elements >= 2);

    /* 1st iteration */
    sha256(seed, seed_length, tmp_element);

    /* perform consecutive iterations minus the first one */
    for (size_t i = 0; i < (elements - 1); ++i) {
        sha256_inplace(tmp_element);
    }

    /* store the result */
    memcpy(tail_element, tmp_element, SHA256_DIGEST_LENGTH);

    return tail_element;
}

void *sha256_chain_with_waypoints(const void *seed,
                                  size_t seed_length,
                                  size_t elements,
                                  void *tail_element,
                                  sha256_chain_idx_elm_t *waypoints,
                                  size_t *waypoints_length)
{
    /* assert if no sha256-chain can be created */
    assert(elements >= 2);

    /* assert to prevent division by 0 */
    assert(*waypoints_length > 0);

    /* assert if no waypoints can be created */
    assert(*waypoints_length > 1);

    /* if we have enough space we store the whole chain */
    if (*waypoints_length >= elements) {
        /* 1st iteration */
        sha256(seed, seed_length, waypoints[0].element);
        waypoints[0].index = 0;

        /* perform consecutive iterations starting at index 1*/
        for (size_t i = 1; i < elements; ++i) {
            sha256_context_t ctx;
            sha256_init(&ctx);
            sha256_update(&ctx, waypoints[(i - 1)].element, SHA256_DIGEST_LENGTH);
            sha256_final(&ctx, waypoints[i].element);
            waypoints[i].index = i;
        }

        /* store the result */
        memcpy(tail_element, waypoints[(elements - 1)].element, SHA256_DIGEST_LENGTH);
        *waypoints_length = (elements - 1);

        return tail_element;
    }
    else {
        unsigned char tmp_element[SHA256_DIGEST_LENGTH];
        size_t waypoint_streak = (elements / *waypoints_length);

        /* 1st waypoint iteration */
        sha256(seed, seed_length, tmp_element);
        for (size_t i = 1; i < waypoint_streak; ++i) {
            sha256_inplace(tmp_element);
        }
        memcpy(waypoints[0].element, tmp_element, SHA256_DIGEST_LENGTH);
        waypoints[0].index = (waypoint_streak - 1);

        /* index of the current computed element in the chain */
        size_t index = (waypoint_streak - 1);

        /* consecutive waypoint iterations */
        size_t j = 1;
        for (; j < *waypoints_length; ++j) {
            for (size_t i = 0; i < waypoint_streak; ++i) {
                sha256_inplace(tmp_element);
                index++;
            }
            memcpy(waypoints[j].element, tmp_element, SHA256_DIGEST_LENGTH);
            waypoints[j].index = index;
        }

        /* store/pass the last used index in the waypoint array */
        *waypoints_length = (j - 1);

        /* remaining iterations down to elements */
        for (size_t i = index; i < (elements - 1); ++i) {
            sha256_inplace(tmp_element);
        }

        /* store the result */
        memcpy(tail_element, tmp_element, SHA256_DIGEST_LENGTH);

        return tail_element;
    }
}

int sha256_chain_verify_element(void *element,
                                size_t element_index,
                                void *tail_element,
                                size_t chain_length)
{
    unsigned char tmp_element[SHA256_DIGEST_LENGTH];

    int delta_count = (chain_length - element_index);

    /* assert if we have an index mismatch */
    assert(delta_count >= 1);

    memcpy((void *)tmp_element, element, SHA256_DIGEST_LENGTH);

    /* perform all consecutive iterations down to tail_element */
    for (int i = 0; i < (delta_count - 1); ++i) {
        sha256_inplace(tmp_element);
    }

    /* return if the computed element equals the tail_element */
    return (memcmp(tmp_element, tail_element, SHA256_DIGEST_LENGTH) != 0);
}