// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

let openjphjs = require('../../dist/openjphjs.js');
const fs = require('fs')

function decode(encodedImagePath, iterations = 1) {
  const encodedBitStream = fs.readFileSync(encodedImagePath);
  const decoder = new openjphjs.HTJ2KDecoder();
  const encodedBuffer = decoder.getEncodedBuffer(encodedBitStream.length);
  encodedBuffer.set(encodedBitStream);

  // do the actual benchmark
  const beginDecode = process.hrtime();
  for(var i=0; i < iterations; i++) {
    decoder.decode();
  }
  const decodeDuration = process.hrtime(beginDecode); // hrtime returns seconds/nanoseconds tuple
  const decodeDurationInSeconds = (decodeDuration[0] + (decodeDuration[1] / 1000000000));
  
  // Print out information about the decode
  console.log("Decode of " + encodedImagePath + " took " + ((decodeDurationInSeconds / iterations * 1000)) + " ms");
  const frameInfo = decoder.getFrameInfo();
  console.log('  frameInfo = ', frameInfo);
  console.log(' imageOffset = ', decoder.getImageOffset());
  var decoded = decoder.getDecodedBuffer();
  console.log('  decoded length = ', decoded.length);

  decoder.delete();
}

function encode(pathToUncompressedImageFrame, imageFrame, pathToJ2CFile, iterations = 1) {
    const uncompressedImageFrame = fs.readFileSync(pathToUncompressedImageFrame);
    console.log('uncompressedImageFrame.length:', uncompressedImageFrame.length)
    const encoder = new openjphjs.HTJ2KEncoder();
    const decodedBytes = encoder.getDecodedBuffer(imageFrame);
    decodedBytes.set(uncompressedImageFrame);
    //encoder.setQuality(false, 0.001);
  
    const encodeBegin = process.hrtime();
    for(var i=0; i < iterations;i++) {
      encoder.encode();
    }
    const encodeDuration = process.hrtime(encodeBegin);
    const encodeDurationInSeconds = (encodeDuration[0] + (encodeDuration[1] / 1000000000));
    
    // print out information about the encode
    console.log("Encode of " + pathToUncompressedImageFrame + " took " + ((encodeDurationInSeconds / iterations * 1000)) + " ms");
    const encodedBytes = encoder.getEncodedBuffer();
    console.log('  encoded length=', encodedBytes.length)
  
    if(pathToJ2CFile) {
        //fs.writeFileSync(pathToJ2CFile, encodedBytes);
    }
    // cleanup allocated memory
    encoder.delete();
  }

openjphjs.onRuntimeInitialized = async _ => {
  decode('../fixtures/j2c/CT2.j2c');
  decode('../../extern/OpenJPH/subprojects/js/html/test.j2c');

  encode('../fixtures/raw/CT1.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: true}, '../fixtures/j2c/CT1.j2c');
  //encode('../fixtures/raw/CT2.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: true}, '../fixtures/j2c/CT2.j2c');
  //encode('../fixtures/raw/MG1.RAW', {width: 3064, height: 4774, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/MG1.j2c');
  //encode('../fixtures/raw/MR1.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: true}, '../fixtures/j2c/MR1.j2c');
  //encode('../fixtures/raw/MR2.RAW', {width: 1024, height: 1024, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/MR2.j2c');
  //encode('../fixtures/raw/MR3.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: true}, '../fixtures/j2c/MR3.j2c');
  //encode('../fixtures/raw/MR4.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/MR4.j2c');
  //encode('../fixtures/raw/NM1.RAW', {width: 256, height: 1024, bitsPerSample: 16, componentCount: 1, isSigned: true}, '../fixtures/j2c/NM1.j2c');
  //encode('../fixtures/raw/RG1.RAW', {width: 1841, height: 1955, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/RG1.j2c');
  //encode('../fixtures/raw/RG1.RAW', {width: 1841, height: 1955, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/RG1.j2c');
  //encode('../fixtures/raw/RG2.RAW', {width: 1760, height: 2140, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/RG2.j2c');
  //encode('../fixtures/raw/RG3.RAW', {width: 1760, height: 1760, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/RG3.j2c');
  //encode('../fixtures/raw/SC1.RAW', {width: 2048, height: 2487, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/SC1.j2c');
  //encode('../fixtures/raw/XA1.RAW', {width: 1024, height: 1024, bitsPerSample: 16, componentCount: 1, isSigned: false}, '../fixtures/j2c/XA1.j2c');

}