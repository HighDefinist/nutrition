// This code is modified, but originally from the "The CUDA Handbook". It was published with the 2-clause BSD license.

#pragma once

#include "basics.h"
#include <brotli/decode.h>
#include <brotli/encode.h>
#include <string.h>



namespace std {
  namespace mz {
    inline bool BrotliOfBuffer(string&CompressedOutput, const char* UncompressedInput, size_t UncompressedInputSize, int quality) {
      size_t OutputSize = (size_t)((UncompressedInputSize*1.1)+100);
      CompressedOutput.resize(OutputSize+8);
      bool Result = BrotliEncoderCompress(quality, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE, UncompressedInputSize, (const uint8_t*)UncompressedInput, &OutputSize, (uint8_t*)&CompressedOutput[8]);
      CompressedOutput.resize(OutputSize+8);
      *((ui64*)&CompressedOutput[0]) = (ui64)UncompressedInputSize;
      return Result;
    }

    inline bool BrotliOfString(string&CompressedOutput, const string& UncompressedInput, int quality) {
      return BrotliOfBuffer(CompressedOutput, UncompressedInput.data(), UncompressedInput.size(), quality);
    }

    inline bool StringOfBrotli(string&UncompressedOutput, const string& CompressedInput) {
      size_t OutputSize = *((ui64*)&CompressedInput[0]);
      UncompressedOutput.resize(OutputSize);
      bool Result = BrotliDecoderDecompress(CompressedInput.size()-8, (uint8_t*)&CompressedInput[8], &OutputSize, (uint8_t*)&UncompressedOutput[0]);

      return Result;
    }
  }
}

