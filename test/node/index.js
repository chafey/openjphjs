let openjph = require('../../dist/openjphjs.js');
const fs = require('fs')

function decode(encodedImagePath, iterations = 1) {
  const encodedBitStream = fs.readFileSync(encodedImagePath);
  const decoder = new openjph.OpenJPHDecoder();
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
  var decoded = decoder.getDecodedBuffer();
  console.log('  decoded length = ', decoded.length);

  decoder.delete();
}


function encode(pathToUncompressedImageFrame, imageFrame, iterations = 1) {
    const uncompressedImageFrame = fs.readFileSync(pathToUncompressedImageFrame);
    console.log('uncompressedImageFrame.length:', uncompressedImageFrame.length)
    const encoder = new openjph.OpenJPHEncoder();
    const decodedBytes = encoder.getDecodedBuffer(imageFrame);
    decodedBytes.set(uncompressedImageFrame);
    //encoder.setNearLossless(0);
  
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
  
    fs.writeFileSync('encoded.j2c', encodedBytes);

    // cleanup allocated memory
    encoder.delete();
  }

openjph.onRuntimeInitialized = async _ => {
  decode('encoded.j2c');
  decode('../../extern/OpenJPH/subprojects/js/html/test.j2c');

  encode('../fixtures/CT2.RAW', {width: 512, height: 512, bitsPerSample: 16, componentCount: 1, isSigned: true});
}
