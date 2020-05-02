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
class OpenJPHEncoder {
  public: 
  /// <summary>
  /// Constructor for encoding a HTJ2K image from JavaScript.  
  /// </summary>
  OpenJPHEncoder() :
    numDecompositions_(5) {
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
  /// Executes an HTJ2K encode using the data in the source buffer.  The
  /// JavaScript code must copy the source image frame into the source
  /// buffer before calling this method.  See documentation on getSourceBytes()
  /// above
  /// </summary>
  void encode() {
    ojph::mem_outfile mem_file;
    mem_file.open();

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

    ojph::param_cod cod = codestream.access_cod();
    cod.set_num_decomposition(numDecompositions_);
    cod.set_block_dims(64,64);
    //if (num_precints != -1)
    //    cod.set_precinct_size(num_precints, precinct_size);
    cod.set_progression_order("RPCL");
    cod.set_color_transform(false);
    cod.set_reversible(true);

    codestream.write_headers(&mem_file);
    int next_comp;
    ojph::line_buf* cur_line = codestream.exchange(NULL, next_comp);
    siz = codestream.access_siz();
    int height = siz.get_image_extent().y - siz.get_image_offset().y;
    for (int i = 0; i < height; ++i)
    {
        for (int c = 0; c < siz.get_num_components(); ++c)
        {
            int* dp = cur_line->i32;
            for(int x=0; x < frameInfo_.width; x++) {
                *dp++ = ((signed short*)(decoded_.data()))[(i * frameInfo_.width) + x];
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
    size_t numDecompositions_;
};
