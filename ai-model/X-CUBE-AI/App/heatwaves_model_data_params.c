/**
  ******************************************************************************
  * @file    heatwaves_model_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-03-10T14:51:04-0400
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#include "heatwaves_model_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_heatwaves_model_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_heatwaves_model_weights_array_u64[36] = {
  0x3f3962463db2d679U, 0x3c3fb56b3bca96a6U, 0x3de5570c3ef71ec8U, 0x3cb2da9d3f406876U,
  0x3f0330ea3ca04a5dU, 0xbe8f4052390f9a71U, 0x3bedb22abd2d487aU, 0xba4db030be3105eeU,
  0xbd073395be87fcd6U, 0xbe308d5dbc3d9ec3U, 0x3f3955923dc1b552U, 0x3be926c4bbdab353U,
  0xbb9c813b3ef80064U, 0xbcc54bcabe859b0eU, 0xbe200331bb9d5fe8U, 0xbe891e62bbf3010cU,
  0x3a878b80bcb02093U, 0xbbb928f4be245845U, 0xbce3ff87be8c5562U, 0xbe290abe3ad8cbf6U,
  0xbe8b4b3e3bdbaad4U, 0x3b4863e1bd072c50U, 0xbb7753c1be2e2ddeU, 0xbcea41edbe918bf0U,
  0xbe3974773c87d603U, 0xc02e431ac02c0c73U, 0xbfab95aabfa8131aU, 0xbfae928bc02d5473U,
  0xbfaa8118bfa78b28U, 0xbfa0cfa9bfa67ebeU, 0x40937b07408a4590U, 0xbfb1d3f8bfc2af22U,
  0xbfbc50b840898971U, 0xbfb648cabfb40dfbU, 0xbfbe08f5bfb6ed25U, 0xbf2e3df3U,
};


ai_handle g_heatwaves_model_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_heatwaves_model_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

