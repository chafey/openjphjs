// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <memory>

#include <ojph_arch.h>
#include <ojph_file.h>
#include <ojph_mem.h>
#include <ojph_params.h>
#include <ojph_codestream.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/val.h>
#endif

#include "FrameInfo.hpp"
#include "Point.hpp"
#include "Size.hpp"

/// <summary>
/// JavaScript API for decoding HTJ2K bistreams with OpenJPH
/// </summary>
class HTJ2KDecoder {
  public: 
  /// <summary>
  /// Constructor for decoding a HTJ2K image from JavaScript.
  /// </summary>
  HTJ2KDecoder() :
    numDecompositions_(0),
    isReversible_(false),
    progressionOrder_(0)
  {
  }

#ifdef __EMSCRIPTEN__
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
#else
  /// <summary>
  /// Returns the buffer to store the encoded bytes.  This method is not exported
  /// to JavaScript, it is intended to be called by C++ code
  /// </summary>
  std::vector<uint8_t>& getEncodedBytes() {
      return encoded_;
  }
  /// <summary>
  /// Returns the buffer to store the decoded bytes.  This method is not exported
  /// to JavaScript, it is intended to be called by C++ code
  /// </summary>
  const std::vector<uint8_t>& getDecodedBytes() const {
      return decoded_;
  }
#endif

  /// <summary>
  /// Decodes the encoded HTJ2K bitstream.  The caller must have copied the
  /// HTJ2K encoded bitstream into the encoded buffer before calling this
  /// method, see getEncodedBuffer() and getEncodedBytes() above.
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
    downSamples_.resize(frameInfo_.componentCount);
    for(size_t i=0; i < frameInfo_.componentCount; i++) {
      downSamples_[i].x = siz.get_downsampling(i).x;
      downSamples_[i].y = siz.get_downsampling(i).y;
    }

    imageOffset_.x = siz.get_image_offset().x;
    imageOffset_.y = siz.get_image_offset().y;
    tileSize_.width = siz.get_tile_size().w;
    tileSize_.height = siz.get_tile_size().h;
    
    tileOffset_.x = siz.get_tile_offset().x;
    tileOffset_.y = siz.get_tile_offset().y;

    ojph::param_cod cod = codestream.access_cod();
    numDecompositions_ = cod.get_num_decompositions();
    isReversible_ = cod.is_reversible();
    progressionOrder_ = cod.get_progression_order();
    blockDimensions_.width = cod.get_block_dims().w;
    blockDimensions_.height = cod.get_block_dims().h;
    precincts_.resize(numDecompositions_);
    for(size_t i=0; i < numDecompositions_; i++) {
      precincts_[i].width = cod.get_precinct_size(i).w;
      precincts_[i].height = cod.get_precinct_size(i).h;
    }
    numLayers_ = cod.get_num_layers();
    isUsingColorTransform_ = cod.is_using_color_transform();

    // allocate destination buffer
    const size_t bytesPerPixel = frameInfo_.bitsPerSample / 8;
    const size_t destinationSize = frameInfo_.width * frameInfo_.height * frameInfo_.componentCount * bytesPerPixel;
    decoded_.resize(destinationSize);

    // parse it
    if(frameInfo_.componentCount == 1) {
      codestream.set_planar(true);
    } else {
      codestream.set_planar(false);
    }
    codestream.create();

    // Extract the data line by line...
    int comp_num;
    for (int y = 0; y < frameInfo_.height; y++)
    {
      size_t lineStart = y * frameInfo_.width * frameInfo_.componentCount * bytesPerPixel;
      if(frameInfo_.componentCount == 1) {
        ojph::line_buf *line = codestream.pull(comp_num);
        if(frameInfo_.bitsPerSample <= 8) {
          std::copy(line->i32, line->i32 + frameInfo_.width, &decoded_[lineStart]);
        } else {
          if(frameInfo_.isSigned) {
            std::copy(line->i32, line->i32 + frameInfo_.width, (signed short*)&decoded_[lineStart]);
          } else {
            std::copy(line->i32, line->i32 + frameInfo_.width, (unsigned short*)&decoded_[lineStart]);
          }
        }
      } else {
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
            // This should work but has not been tested yet
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

  Point getDownSample(size_t component) const {
    return downSamples_[component];
  }

  Point getImageOffset() const {
    return imageOffset_;
  }
  Size getTileSize() const {
    return tileSize_;
  }
  Point getTileOffset() const {
    return tileOffset_;
  }
  Size getBlockDimensions() const {
    return blockDimensions_;
  }
  Size getPrecinct(size_t level) const {
    return precincts_[level];
  }
  int32_t getNumLayers() const {
    return numLayers_;
  }
  bool getIsUsingColorTransform() const {
    return isUsingColorTransform_;
  }

  private:
    std::vector<uint8_t> encoded_;
    std::vector<uint8_t> decoded_;
    FrameInfo frameInfo_;

    std::vector<Point> downSamples_;
    size_t numDecompositions_;
    bool isReversible_;
    size_t progressionOrder_;
    Point imageOffset_;
    Size tileSize_;
    Point tileOffset_;
    Size blockDimensions_;
    std::vector<Size> precincts_;
    int32_t numLayers_;
    bool isUsingColorTransform_;
};

