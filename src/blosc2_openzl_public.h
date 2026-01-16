/*********************************************************************
Blosc - Blocked Shuffling and Compression Library

Copyright (c) 2021  The Blosc Development Team <blosc@blosc.org>
https://blosc.org
License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/


#ifndef PUBLIC_WRAPPER_H
#define PUBLIC_WRAPPER_H

#if defined(_MSC_VER)
#define BLOSC2_OPENZl_EXPORT __declspec(dllexport)
#elif (defined(__GNUC__) || defined(__clang__))
#define BLOSC2_OPENZl_EXPORT __attribute__((visibility("default")))
#else
#error "Unknown compiler"
#endif

#include "blosc2.h"
#include "blosc2/codecs-registry.h"


BLOSC2_OPENZl_EXPORT int blosc2_openzl_encoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_cparams *cparams, const void *chunk);

BLOSC2_OPENZl_EXPORT int blosc2_openzl_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk);

// Declare the info struct as extern
BLOSC2_OPENZl_EXPORT extern codec_info info;

#if defined(_MSC_VER)
// Needed to export functions in Windows
#endif

#endif
