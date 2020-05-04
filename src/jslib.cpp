// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#include "HTJ2KDecoder.hpp"
#include "HTJ2KEncoder.hpp"

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

EMSCRIPTEN_BINDINGS(Point) {
  value_object<Point>("Point")
    .field("x", &Point::x)
    .field("y", &Point::y)
       ;
}

EMSCRIPTEN_BINDINGS(Size) {
  value_object<Size>("Size")
    .field("width", &Size::width)
    .field("height", &Size::height)
       ;
}

EMSCRIPTEN_BINDINGS(HTJ2KDecoder) {
  class_<HTJ2KDecoder>("HTJ2KDecoder")
    .constructor<>()
    .function("getEncodedBuffer", &HTJ2KDecoder::getEncodedBuffer)
    .function("getDecodedBuffer", &HTJ2KDecoder::getDecodedBuffer)
    .function("decode", &HTJ2KDecoder::decode)
    .function("getFrameInfo", &HTJ2KDecoder::getFrameInfo)
    .function("getDownSample", &HTJ2KDecoder::getDownSample)
    .function("getNumDecompositions", &HTJ2KDecoder::getNumDecompositions)
    .function("getIsReversible", &HTJ2KDecoder::getIsReversible)
    .function("getProgressionOrder", &HTJ2KDecoder::getProgressionOrder)
    .function("getImageOffset", &HTJ2KDecoder::getImageOffset)
    .function("getTileSize", &HTJ2KDecoder::getTileSize)
    .function("getTileOffset", &HTJ2KDecoder::getTileOffset)
    .function("getBlockDimensions", &HTJ2KDecoder::getBlockDimensions)
    .function("getPrecinct", &HTJ2KDecoder::getPrecinct)
    .function("getNumLayers", &HTJ2KDecoder::getNumLayers)
    .function("getIsUsingColorTransform", &HTJ2KDecoder::getIsUsingColorTransform)
   ;
}

EMSCRIPTEN_BINDINGS(HTJ2KEncoder) {
  class_<HTJ2KEncoder>("HTJ2KEncoder")
    .constructor<>()
    .function("getDecodedBuffer", &HTJ2KEncoder::getDecodedBuffer)
    .function("getEncodedBuffer", &HTJ2KEncoder::getEncodedBuffer)
    .function("encode", &HTJ2KEncoder::encode)
    .function("setDecompositions", &HTJ2KEncoder::setDecompositions)
    .function("setQuality", &HTJ2KEncoder::setQuality)
    .function("setDecompositions", &HTJ2KEncoder::setDecompositions)
    .function("setDecompositions", &HTJ2KEncoder::setDecompositions)
   ;
}