// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <iostream>
#include <vector>
#include <iterator>
#include <time.h> 
#include <algorithm>

#include "../../src/HTJ2KDecoder.hpp"
#include "../../src/HTJ2KEncoder.hpp"

void readFile(std::string fileName, std::vector<uint8_t>& vec) {
    // open the file:
    std::ifstream file(fileName, std::ios::in | std::ios::binary);
    // Stop eating new lines in binary mode!!!
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(),
                std::istream_iterator<uint8_t>(file),
                std::istream_iterator<uint8_t>());

    //std::istreambuf_iterator iter(file);
    //std::copy(iter.begin(), iter.end(), std::back_inserter(vec));
}

void writeFile(std::string fileName, const std::vector<uint8_t>& vec) {
    std::ofstream file(fileName, std::ios::out | std::ofstream::binary);
    std::copy(vec.begin(), vec.end(), std::ostreambuf_iterator<char>(file));
}

enum { NS_PER_SECOND = 1000000000 };

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

void decodeFile(const char* path) {
    HTJ2KDecoder decoder;
    std::vector<uint8_t>& encodedBytes = decoder.getEncodedBytes();
    readFile(path, encodedBytes);

    timespec start, finish, delta;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    decoder.readHeader();
    //Size resolutionAtLevel = decoder.calculateDecompositionLevel(1);
    //std::cout << resolutionAtLevel.width << ',' << resolutionAtLevel.height << std::endl;

    decoder.decodeSubResolution(1);

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);
    sub_timespec(start, finish, &delta);
    const double ms = (double)(delta.tv_nsec) / 1000000.0;
    printf("Decode of %s took %f ms\n", path, ms);
}

void encodeFile(const char* inPath, const FrameInfo frameInfo, const char* outPath) {
    HTJ2KEncoder encoder;
    std::vector<uint8_t>& rawBytes = encoder.getDecodedBytes(frameInfo);
    readFile(inPath, rawBytes);

    timespec start, finish, delta;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    encoder.encode();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finish);
    sub_timespec(start, finish, &delta);
    const double ms = (double)(delta.tv_nsec) / 1000000.0;
    printf("Encode of %s took %f ms\n", inPath, ms);

    if(outPath) {
        const std::vector<uint8_t>& encodedBytes = encoder.getEncodedBytes();
        writeFile(outPath, encodedBytes);
    }
}

int main(int argc, char** argv) {
    decodeFile("test/fixtures/j2c/CT1.j2c");
    //decodeFile("test/fixtures/j2c/CT2.j2c");
    //decodeFile("test/fixtures/j2c/MG1.j2c");

    //encodeFile("test/fixtures/raw/CT1.RAW", {.width = 512, .height = 512, .bitsPerSample = 16, .componentCount = 1, .isSigned = true}, "test/fixtures/j2c/CT1.j2c");
    return 0;
}
