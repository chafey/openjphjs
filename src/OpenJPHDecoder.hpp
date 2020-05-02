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
/// JavaScript API for decoding HTJ2K bistreams with OpenJPH
/// </summary>
class OpenJPHDecoder {
  public: 
  /// <summary>
  /// Constructor for decoding a HTJ2K image from JavaScript.
  /// </summary>
  OpenJPHDecoder() :
    numDecompositions_(0),
    isReversible_(false),
    progressionOrder_(0)
  {
  }

  /// <summary>
  /// Resizes encoded buffer and returns a TypedArray of the buffer allocated
  /// in WASM memory space that will hold the HTJ2K encoded bitstream.
  /// JavaScript code needs to copy the HTJ2K encoded bistream into the
  /// returned TypedArray.  This copy operation is needed because WASM runs
  /// in a sandbox and cannot access memory managed by JavaScript.
  /// </summary>
  emscripten::val getEncodedBuffer(size_t encodedSize) {
    encoded_.resize(encodedSize);
    return emscripten::val(emscripten::typed_memory_view(encoded_.size(), encoded_.data()));
  }
  
  /// <summary>
  /// Returns a TypedArray of the buffer allocated in WASM memory space that
  /// holds the decoded pixel data
  /// </summary>
  emscripten::val getDecodedBuffer() {
    return emscripten::val(emscripten::typed_memory_view(decoded_.size(), decoded_.data()));
  }

  /// <summary>
  /// Decodes the encoded HTJ2K bitstream.  The caller must have copied the
  /// HTJ2K encoded bitstream into the encoded buffer before calling this
  /// method, see getEncodedBuffer() above.
  /// </summary>
  void decode() {
    // Parse the header
    ojph::codestream codestream;
    ojph::mem_infile mem_file;
    mem_file.open(encoded_.data(), encoded_.size());
    codestream.read_headers(&mem_file);
    ojph::param_siz siz = codestream.access_siz();
    frameInfo_.width = siz.get_image_extent().x - siz.get_image_offset().x;
    frameInfo_.height = siz.get_image_extent().y - siz.get_image_offset().y;
    frameInfo_.componentCount = siz.get_num_components();
    frameInfo_.bitsPerSample = siz.get_bit_depth(0);
    frameInfo_.isSigned = siz.is_signed(0);

    ojph::param_cod cod = codestream.access_cod();
    numDecompositions_ = cod.get_num_decompositions();
    isReversible_ = cod.is_reversible();
    progressionOrder_ = cod.get_progression_order();

    // allocate destination buffer
    const size_t bytesPerPixel = frameInfo_.bitsPerSample / 8;
    const size_t destinationSize = frameInfo_.width * frameInfo_.height * frameInfo_.componentCount * bytesPerPixel;
    decoded_.resize(destinationSize);

    // parse it
    codestream.set_planar(false);
    codestream.create();

    // Extract the data line by line...
    int comp_num;
    for (int y = 0; y < frameInfo_.height; y++)
    {
        size_t lineStart = y * frameInfo_.width * frameInfo_.componentCount * bytesPerPixel;
        for (int c = 0; c < frameInfo_.componentCount; c++)
        {
            ojph::line_buf *line = codestream.pull(comp_num);
            if(frameInfo_.bitsPerSample <= 8) {
                uint8_t* pOut = &decoded_[lineStart] + c;
                for (size_t x = 0; x < frameInfo_.width; x++) {
                    int val = line->i32[x];
                    pOut[x * frameInfo_.componentCount] = val;
                }
            } else {
                if(frameInfo_.isSigned) {
                    short* pOut = (short*)&decoded_[lineStart] + c;
                    for (size_t x = 0; x < frameInfo_.width; x++) {
                        int val = line->i32[x];
                        pOut[x * frameInfo_.componentCount] = val;
                    }
                } else {
                    unsigned short* pOut = (unsigned short*)&decoded_[lineStart] + c;
                    for (size_t x = 0; x < frameInfo_.width; x++) {
                        int val = line->i32[x];
                        pOut[x * frameInfo_.componentCount] = val;
                    }
                }
            }
        }
    }
  }

  /// <summary>
  /// returns the FrameInfo object for the decoded image.
  /// </summary>
  const FrameInfo& getFrameInfo() const {
      return frameInfo_;
  }

  /// <summary>
  /// returns the number of wavelet decompositions.
  /// </summary>
  const size_t getNumDecompositions() const {
      return numDecompositions_;
  }

  /// <summary>
  /// returns true if the image is lossless, false if lossy
  /// </summary>
  const bool getIsReversible() const {
      return isReversible_;
  }

  /// <summary>
  /// returns progression order.
  // 0 = LRCP
  // 1 = RLCP
  // 2 = RPCL
  // 3 = PCRL
  // 4 = CPRL
  /// </summary>
  const size_t getProgressionOrder() const {
      return progressionOrder_;
  }

  private:
    std::vector<uint8_t> encoded_;
    std::vector<uint8_t> decoded_;
    FrameInfo frameInfo_;
    size_t numDecompositions_;
    bool isReversible_;
    size_t progressionOrder_;
};

