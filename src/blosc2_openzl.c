/*********************************************************************
 * blosc2_openzl: OpenZL plugin for Blosc2
 *
 * Copyright (c) 2026  The Blosc Development Team <blosc@blosc.org>
 * https://blosc.org
 * License: GNU Affero General Public License v3.0 (see LICENSE.txt)
**********************************************************************/

#include "blosc2_openzl.h"

codec_info info = {
    .encoder=(char *)"blosc2_openzl_encoder",
    .decoder=(char *)"blosc2_openzl_decoder"
};

bool _is_bit_set(uint8_t byte, unsigned bit)
{
    return (byte & (1u << bit)) != 0;
}


static ZL_Report openzl_shuffle_delta_concat_graph(
    ZL_Graph* graph, ZL_Edge** inputs, size_t numInputs) {
  ZL_RESULT_DECLARE_SCOPE_REPORT(graph);

  ZL_ERR_IF_NE(numInputs, 1, parameter_invalid);
  ZL_ERR_IF_NULL(inputs, parameter_invalid);
  ZL_ERR_IF_NULL(graph, parameter_invalid);

  const ZL_NodeIDList nodes = ZL_Graph_getCustomNodes(graph);
  const ZL_GraphIDList graphs = ZL_Graph_getCustomGraphs(graph);
  ZL_ERR_IF_NE(nodes.nbNodeIDs, 3, parameter_invalid);

  // SHUFFLE: just converts to serial if node is not transpose (=shuffle)
  ZL_TRY_LET(ZL_EdgeList, fields, ZL_Edge_runNode(inputs[0], nodes.nodeids[0]));

  if (fields.nbEdges == 0) {
    return ZL_returnSuccess();
  }

  // DELTA: does nothing if node is not delta
  ZL_Edge** delta_edges;
  if (nodes.nodeids[1].nid == ZL_NODE_DELTA_INT.nid){
    delta_edges = (ZL_Edge**)ZL_Graph_getScratchSpace(graph, fields.nbEdges * sizeof(ZL_Edge*));
    ZL_ERR_IF_NULL(delta_edges, allocation);
    for (size_t i = 0; i < fields.nbEdges; ++i) {
      ZL_TRY_LET(ZL_EdgeList,
                as_num,
                ZL_Edge_runNode(fields.edges[i], ZL_NODE_CONVERT_SERIAL_TO_NUM_LE8));
      ZL_ERR_IF_NE(as_num.nbEdges, 1, graph_invalid);

      ZL_TRY_LET(ZL_EdgeList, delta, ZL_Edge_runNode(as_num.edges[0], nodes.nodeids[1]));
      ZL_ERR_IF_NE(delta.nbEdges, 1, graph_invalid);

      ZL_TRY_LET(ZL_EdgeList,
                as_serial,
                ZL_Edge_runNode(delta.edges[0], ZL_NODE_CONVERT_NUM_TO_SERIAL_LE));
      ZL_ERR_IF_NE(as_serial.nbEdges, 1, graph_invalid);

      delta_edges[i] = as_serial.edges[0];
    }
  }
  else{
    delta_edges = fields.edges;
  }

  if (fields.nbEdges == 1) {
    ZL_RET_R_IF_ERR(ZL_Edge_setDestination(delta_edges[0], graphs.graphids[1]));
    return ZL_returnSuccess();
  }
  
  // CONCAT: just directs different streams to codec if node is not concat
  if (nodes.nodeids[2].nid == ZL_NODE_CONCAT_SERIAL.nid){
    ZL_TRY_LET(ZL_EdgeList,
             concat_out,
             ZL_Edge_runMultiInputNode(
                 delta_edges, fields.nbEdges, nodes.nodeids[2])); 
    if (concat_out.nbEdges == 2) {
      ZL_RET_R_IF_ERR(
          ZL_Edge_setDestination(concat_out.edges[0], graphs.graphids[0]));
      ZL_RET_R_IF_ERR(
          ZL_Edge_setDestination(concat_out.edges[1], graphs.graphids[1]));
      return ZL_returnSuccess();
    }
    if (concat_out.nbEdges == 1) {
      ZL_RET_R_IF_ERR(
          ZL_Edge_setDestination(concat_out.edges[0], graphs.graphids[1]));
      return ZL_returnSuccess();
    }
    ZL_RET_R_ERR(graph_invalid, "concat output has unexpected number of edges");
  }
  else{
    for (size_t i = 0; i < fields.nbEdges; ++i) {
      ZL_RET_R_IF_ERR(
          ZL_Edge_setDestination(delta_edges[i], graphs.graphids[i]));
    }
    return ZL_returnSuccess();
  }
}

static ZL_GraphID openzl_register_blosc2_graph(
    ZL_Compressor* compressor, ZL_GraphID* successor_graphs, size_t nbCustomGraphs, bool shuffle, bool split, bool delta) {
  if (split && !shuffle){
     BLOSC_TRACE_ERROR("When split is true, must also shuffle!");
  } 
  ZL_NodeID nodes[3] = {shuffle ? ZL_NODE_TRANSPOSE_SPLIT : ZL_NODE_CONVERT_STRUCT_TO_SERIAL, 
                        delta ? ZL_NODE_DELTA_INT : ZL_NODE_ILLEGAL, 
                        split ? ZL_NODE_ILLEGAL : ZL_NODE_CONCAT_SERIAL};

  ZL_Type input_types[1] = {ZL_Type_struct};
  ZL_FunctionGraphDesc desc = {
      .name = "blosc2.shuffle_delta_concat",
      .graph_f = openzl_shuffle_delta_concat_graph,
      .validate_f = NULL,
      .inputTypeMasks = input_types,
      .nbInputs = 1,
      .lastInputIsVariable = 0,
      .customGraphs = successor_graphs,
      .nbCustomGraphs = nbCustomGraphs,
      .customNodes = nodes,
      .nbCustomNodes = 3,
      .localParams = {0},
      .opaque = {0},
  };
  return ZL_Compressor_registerFunctionGraph(compressor, &desc);
}

int blosc2_openzl_encoder(
    const uint8_t *input,
    int32_t input_len,
    uint8_t *output,
    int32_t output_len,
    uint8_t meta,
    blosc2_cparams* cparams,
    const void* chunk) {
  
  // initialize compress parameters
  /* This would allow caching of generated graph between blocks, but for benchmarks there was
  no slowdown when just generating the graph anew for every compression
  auto *codec_params = (blosc2_openzl_graphcparams *)cparams->codec_params;
  ZL_CCtx* openzl_cctx;
  if (codec_params != NULL && codec_params->openzl_cctx != NULL){
    openzl_cctx = codec_params->openzl_cctx;
  }
  else{
    openzl_cctx = ZL_CCtx_create();
  }
  */
  ZL_CCtx* openzl_cctx = ZL_CCtx_create();
  bool shuffle_ = _is_bit_set(meta, 1);
  bool delta_ = _is_bit_set(meta, 2);
  bool split_ = _is_bit_set(meta, 3);
  bool checksum_ = _is_bit_set(meta, 4);

  const uint32_t typesize = cparams->typesize;
  int clevel = cparams->clevel;

  ZL_GraphID graphId;
  ZL_TypedRef* input_ = ZL_TypedRef_createStruct(input, typesize, input_len / typesize);

  ZL_Compressor* compressor = NULL;
  ZL_GraphID graph = (ZL_GraphID){0};
  /* if caching
  // Try to reuse already-defined graph
  if (codec_params != NULL && codec_params->profile==meta && codec_params->clevel==clevel) {
    compressor = codec_params->openzl_compressor;
    graph = codec_params->openzl_graph;
  }
  else {
    */
    compressor = ZL_Compressor_create();
    if (compressor == NULL) {
      BLOSC_TRACE_ERROR("Error when creating OpenZL compressor.  Giving up.");
      ZL_TypedRef_free(input_);
      return 0;
    }
    ZL_GraphID zstd_graph_id = ZL_GRAPH_ZSTD;
    ZL_GraphID lz4_graph_id = ZL_GRAPH_LZ4;
    /*
    int zstdclevel = (clevel < 9) ? clevel * 2 - 1 : ZSTD_maxCLevel();
    // Make the level 8 close enough to maxCLevel
    if (zstdclevel == 8) zstdclevel = ZSTD_maxCLevel() - 2;
    zstd_graph_id = ZL_Compressor_registerZstdGraph_withLevel(compressor, zstdclevel);
    */
    zstd_graph_id = ZL_Compressor_registerZstdGraph_withLevel(compressor, clevel);
    ZL_RESULT_OF(ZL_GraphID) lz4_result = ZL_Compressor_buildLZ4Graph(compressor, clevel);
    if (ZL_RES_isError(lz4_result)) {
      BLOSC_TRACE_ERROR("Error when building LZ4 graph for OpenZL: '%s'.  Giving up.",
                        ZL_ErrorCode_toString(ZL_RES_code(lz4_result)));
      ZL_Compressor_free(compressor);
      ZL_TypedRef_free(input_);
      return 0;
    }
    lz4_graph_id = ZL_RES_value(lz4_result);

    ZL_GraphID codec_graph_id = _is_bit_set(meta, 0) ? lz4_graph_id : zstd_graph_id;

    size_t nbCustomGraphs = split_ ? typesize : 2;
    ZL_GraphID successor_graphs[nbCustomGraphs];
    successor_graphs[0] = split_ ? codec_graph_id : ZL_GRAPH_STORE; // store concat metadata when not splitting
    for (size_t i = 1; i < nbCustomGraphs; i++) {
        successor_graphs[i] = codec_graph_id;   // EVERY lane -> codec -> STORE
      }
    graphId = openzl_register_blosc2_graph(compressor, successor_graphs, nbCustomGraphs, shuffle_, split_, delta_);
    graph = ZL_Compressor_buildACEGraphWithDefault(compressor, graphId);
/* if caching
    // Cache generated graph
    codec_params->openzl_compressor = compressor;
    codec_params->openzl_graph = graph;
    codec_params->openzl_cctx = openzl_cctx;
  }
*/

  ZL_CCtx_setParameter(openzl_cctx,  ZL_CParam_formatVersion, ZL_MAX_FORMAT_VERSION);
  if (checksum_){
    ZL_CCtx_setParameter(openzl_cctx, ZL_CParam_compressedChecksum, ZL_TernaryParam_enable);
    ZL_CCtx_setParameter(openzl_cctx, ZL_CParam_contentChecksum, ZL_TernaryParam_enable);
  } else {
    ZL_CCtx_setParameter(openzl_cctx, ZL_CParam_compressedChecksum, ZL_TernaryParam_disable);
    ZL_CCtx_setParameter(openzl_cctx, ZL_CParam_contentChecksum, ZL_TernaryParam_disable);
  }

  ZL_Report code = ZL_Compressor_selectStartingGraphID(compressor, graph);
  if (ZL_isError(code)) {
    BLOSC_TRACE_ERROR("Error when setting graph for OpenZL compressor: '%s'.  Giving up.",
                      ZL_ErrorCode_toString(ZL_errorCode(code)));
    ZL_TypedRef_free(input_);
    ZL_Compressor_free(compressor);
    return 0;
  }
  ZL_CCtx_refCompressor(openzl_cctx, compressor);

  code = ZL_CCtx_compressTypedRef(openzl_cctx, output, output_len, input_);

  // Cleanup
  ZL_TypedRef_free(input_);
  ZL_CCtx_free(openzl_cctx); // REMOVE if caching graph between runs

  if (ZL_isError(code)) {
    return 0;
  }
  size_t cbytes = ZL_validResult(code);
  return (int)cbytes;
}

// Decompress a block
int blosc2_openzl_decoder(const uint8_t *input, int32_t input_len, uint8_t *output, int32_t output_len,
                        uint8_t meta, blosc2_dparams *dparams, const void *chunk) {
  bool checksum_ = _is_bit_set(meta, 4);
  // May want to cache decomp params between blocks
  // auto *cache = (blosc2_openzl_graphdparams *)dparams->decomp_cache;
  // ZL_CCtx* openzl_dctx;
  // ZL_DCtx_setParameter(openzl_dctx,
  //                     ZL_DParam_stickyParameters, 1);
  // if (cache->openzl_dctx == NULL) {
  ZL_DCtx* openzl_dctx = ZL_DCtx_create();

  if (checksum_) {
    ZL_DCtx_setParameter(openzl_dctx,
                          ZL_DParam_checkCompressedChecksum,
                          ZL_TernaryParam_enable);
    ZL_DCtx_setParameter(openzl_dctx,
                          ZL_DParam_checkContentChecksum,
                          ZL_TernaryParam_enable);
  } else {
    ZL_DCtx_setParameter(openzl_dctx,
                          ZL_DParam_checkCompressedChecksum,
                          ZL_TernaryParam_disable);
    ZL_DCtx_setParameter(openzl_dctx,
                          ZL_DParam_checkContentChecksum,
                          ZL_TernaryParam_disable);
  }
 
  int32_t typesize = dparams->typesize;
  ZL_Report code;
  if ((typesize > 0) && (output_len % (size_t)typesize == 0)) {
    size_t num_structs = output_len / (size_t)typesize;
    ZL_TypedBuffer* tbuf = ZL_TypedBuffer_createWrapStruct(
        output, (size_t)typesize, num_structs);
    if (tbuf == NULL) {
      BLOSC_TRACE_ERROR("Error in OpenZL decompression: failed to create TypedBuffer.");
      return 0;
    }
    code = ZL_DCtx_decompressTBuffer(openzl_dctx,
                                     tbuf, (void*)input, input_len);
    ZL_TypedBuffer_free(tbuf);
  }
  else {
    code = ZL_DCtx_decompress(openzl_dctx,
                              (void*)output, output_len,
                              (void*)input, input_len);
  }

  ZL_DCtx_free(openzl_dctx); // REMOVE if caching DCtx between runs

  if (ZL_isError(code)) {
    BLOSC_TRACE_ERROR("Error in OpenZL decompression: '%s'.  Giving up.",
                      ZL_ErrorCode_toString(ZL_errorCode(code)));
    return 0;
  }
  return (int)output_len;
}
