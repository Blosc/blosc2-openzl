/*********************************************************************
 * blosc2_openzl: OpenZL plugin for Blosc2
 *
 * Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
 * https://blosc.org
 * License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/


#ifndef WRAPPER_H
#define WRAPPER_H

#include "blosc2.h"
#include "b2nd.h"
#include "blosc2/codecs-registry.h"
#include "openzl/openzl.h"
#include "openzl/codecs/zl_concat.h"
#include "openzl/codecs/zl_store.h"
#include "openzl/codecs/zl_transpose.h"
#include <stdio.h>

/* Necessary for caching*/
// typedef struct {
//     int clevel;
//     uint8_t profile;
//     ZL_GraphID openzl_graph;
//     ZL_Compressor* openzl_compressor;
//     ZL_CCtx* openzl_cctx;
// } blosc2_openzl_graphcparams;

#endif
