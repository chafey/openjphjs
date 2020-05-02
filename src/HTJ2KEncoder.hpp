// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <memory>

#include "../extern/OpenJPH/src/core/common/ojph_arch.h"
#include "../extern/OpenJPH/src/core/common/ojph_file.h"
#include "../extern/OpenJPH/src/core/common/ojph_mem.h"
#include "../extern/OpenJPH/src/core/common/ojph_params.h"
#include "../extern/OpenJPH/src/core/common/ojph_codestream.h"

#include <emscripten/val.h>

#include "FrameInfo.hpp"

/// <summary>
/// JavaScript API for encoding images to HTJ2K bitstreams with OpenJPH
/// </summary>
class HTJ2KEncoder {
  public: 
  /// <summary>
  /// Constructor for encoding a HTJ2K image from JavaScript.  
  /// </summary>
  HTJ2KEncoder() :
    decompositions_(5),
    lossless_(true),
    quantizationStep_(-1.0),
    progressionOrder_(2) // RPCL
  {
  }

  /// <summary>
  /// Resizes the decoded buffer to accomodate the specified frameInfo.
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// will hold the pixel data to be encoded.  JavaScript code needs
  /// to copy the pixel data into the returned TypedArray.  This copy
  /// operation is needed because WASM runs in a sandbox and cannot access 
  /// data managed by JavaScript
  /// </summary>
  /// <param name="frameInfo">FrameInfo that describes the pixel data to be encoded</param>
  /// <returns>
  /// TypedArray for the buffer allocated in WASM memory space for the 
  /// source pixel data to be encoded.
  /// </returns>
  emscripten::val getDecodedBuffer(const FrameInfo& frameInfo) {
    frameInfo_ = frameInfo;
    const size_t bytesPerPixel = frameInfo_.bitsPerSample / 8;
    const size_t decodedSize = frameInfo_.width * frameInfo_.height * frameInfo_.componentCount * bytesPerPixel;
    decoded_.resize(decodedSize);
    return emscripten::val(emscripten::typed_memory_view(decoded_.size(), decoded_.data()));
  }
  
  /// <summary>
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// holds the encoded pixel data.
  /// </summary>
  /// <returns>
  /// TypedArray for the buffer allocated in WASM memory space for the 
  /// encoded pixel data.
  /// </returns>
  emscripten::val getEncodedBuffer() {
    return emscripten::val(emscripten::typed_memory_view(encoded_.size(), encoded_.data()));
  }

  /// <summary>
  /// Sets the number of wavelet decompositions
  /// </summary>
  void setDecompositions(size_t decompositions) {
    decompositions_ = decompositions;
  }

  /// <summary>
  /// Sets the quality level for the image.  If lossless is false then
  /// quantizationStep controls the lossy quantization applied.  quantizationStep
  /// is ignored if lossless is true
  /// </summary>
  void setQuality(bool lossless, float quantizationStep) {
    lossless_ = lossless;
    quantizationStep_ = quantizationStep;
  }

  /// <summary>
  /// Sets the progression order 
  /// 0 = LRCP
  /// 1 = RLCP
  /// 2 = RPCL
  /// 3 = PCRL
  /// 4 = CPRL 
  /// </summary>
  void setprogressionOrder(size_t progressionOrder) {
    progressionOrder_ = progressionOrder;
  }

  /// <summary>
  /// Executes an HTJ2K encode using the data in the source buffer.  The
  /// JavaScript code must copy the source image frame into the source
  /// buffer before calling this method.  See documentation on getSourceBytes()
  /// above
  /// </summary>
  void encode() {
    ojph::mem_outfile mem_file;
    mem_file.open();

    // Setup image size parameters
    ojph::codestream codestream;
    ojph::param_siz siz = codestream.access_siz();
    siz.set_image_extent(ojph::point(frameInfo_.width, frameInfo_.height));
    int num_comps = frameInfo_.componentCount;
    siz.set_num_components(num_comps);
    for (int c = 0; c < num_comps; ++c)
        siz.set_component(c, ojph::point(1, 1), frameInfo_.bitsPerSample, frameInfo_.isSigned);
    siz.set_image_offset(ojph::point(0, 0));
    siz.set_tile_size(ojph::size(0,0));
    siz.set_tile_offset(ojph::point(0,0));

    // Setup encoding parameters
    ojph::param_cod cod = codestream.access_cod();
    cod.set_num_decomposition(decompositions_);
    cod.set_block_dims(64,64);
    //if (num_precints != -1)
    //    cod.set_precinct_size(num_precints, precinct_size);
    const char* progOrders[] = {"LRCP", "RLCP", "RPCL", "PCRL", "CPRL"};
    cod.set_progression_order(progOrders[progressionOrder_]);
    cod.set_color_transform(false);
    cod.set_reversible(lossless_);
    if(!lossless_) {
      codestream.access_qcd().set_irrev_quant(quantizationStep_);
    }
    codestream.write_headers(&mem_file);

    // Encode the image
    const size_t bytesPerPixel = frameInfo_.bitsPerSample / 8;
    int next_comp;
    ojph::line_buf* cur_line = codestream.exchange(NULL, next_comp);
    siz = codestream.access_siz();
    int height = siz.get_image_extent().y - siz.get_image_offset().y;
    for (size_t y = 0; y < height; y++)
    {
        for (size_t c = 0; c < siz.get_num_components(); c++)
        {
          int* dp = cur_line->i32;
          if(frameInfo_.bitsPerSample <= 8) {
            uint8_t* pIn = (uint8_t*)(decoded_.data() + (y * frameInfo_.width * bytesPerPixel));
            for(size_t x=0; x < frameInfo_.width; x++) {
              *dp++ = *pIn++;
            }
          } else {
            if(frameInfo_.isSigned) {
              int16_t* pIn = (int16_t*)(decoded_.data() + (y * frameInfo_.width * bytesPerPixel));
              for(size_t x=0; x < frameInfo_.width; x++) {
                *dp++ = *pIn++;
              }
            } else {
              uint16_t* pIn = (uint16_t*)(decoded_.data() + (y * frameInfo_.width * bytesPerPixel));
              for(size_t x=0; x < frameInfo_.width; x++) {
                *dp++ = *pIn++;
              }
            }
          }
          cur_line = codestream.exchange(cur_line, next_comp);
        }
    }
    codestream.flush();

    encoded_.resize(mem_file.tell());
    memcpy(encoded_.data(), mem_file.get_data(), (size_t)mem_file.tell());

    codestream.close();
  }

  private:
    std::vector<uint8_t> decoded_;
    std::vector<uint8_t> encoded_;
    FrameInfo frameInfo_;
    size_t decompositions_;
    bool lossless_;
    float quantizationStep_;
    size_t progressionOrder_;
};
