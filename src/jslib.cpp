// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#include "OpenJPHDecoder.hpp"
#include "OpenJPHEncoder.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(FrameInfo) {
  value_object<FrameInfo>("FrameInfo")
    .field("width", &FrameInfo::width)
    .field("height", &FrameInfo::height)
    .field("bitsPerSample", &FrameInfo::bitsPerSample)
    .field("componentCount", &FrameInfo::componentCount)
    .field("isSigned", &FrameInfo::isSigned)
       ;
}

EMSCRIPTEN_BINDINGS(JpegLSDecoder) {
  class_<OpenJPHDecoder>("OpenJPHDecoder")
    .constructor<>()
    .function("getEncodedBuffer", &OpenJPHDecoder::getEncodedBuffer)
    .function("getDecodedBuffer", &OpenJPHDecoder::getDecodedBuffer)
    .function("decode", &OpenJPHDecoder::decode)
    .function("getFrameInfo", &OpenJPHDecoder::getFrameInfo)
    .function("getNumDecompositions", &OpenJPHDecoder::getNumDecompositions)
    .function("getIsReversible", &OpenJPHDecoder::getIsReversible)
    .function("getProgressionOrder", &OpenJPHDecoder::getProgressionOrder)
   ;
}

EMSCRIPTEN_BINDINGS(OpenJPHEncoder) {
  class_<OpenJPHEncoder>("OpenJPHEncoder")
    .constructor<>()
    .function("getDecodedBuffer", &OpenJPHEncoder::getDecodedBuffer)
    .function("getEncodedBuffer", &OpenJPHEncoder::getEncodedBuffer)
    .function("encode", &OpenJPHEncoder::encode)
   ;
}