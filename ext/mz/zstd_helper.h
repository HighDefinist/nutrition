// This code is modified, but originally from the "The CUDA Handbook". It was published with the 2-clause BSD license.

#pragma once

#include "basics.h"
#include <zstd.h>
#include <string.h>



namespace std {
  namespace mz {
    inline bool ZstdOfBuffer(string&CompressedOutput, const char* UncompressedInput, size_t UncompressedInputSize, int quality) {
      size_t OutputSize = ZSTD_compressBound(UncompressedInputSize);
      CompressedOutput.resize(OutputSize+8);

      OutputSize = ZSTD_compress(&CompressedOutput[8], OutputSize, UncompressedInput, UncompressedInputSize, quality);
      if(ZSTD_isError(OutputSize)) return false;
      CompressedOutput.resize(OutputSize+8);
      *((ui64*)&CompressedOutput[0]) = (ui64)UncompressedInputSize;
      return true;
    }

    inline bool ZstdOfString(string&CompressedOutput, const string& UncompressedInput, int quality) {
      return ZstdOfBuffer(CompressedOutput, UncompressedInput.data(), UncompressedInput.size(), quality);
    }

    inline bool StringOfZstd(string&UncompressedOutput, const string& CompressedInput) {
      size_t OutputSize = *((ui64*)&CompressedInput[0]);
      UncompressedOutput.resize(OutputSize);
      return ZSTD_decompress(&UncompressedOutput[0], OutputSize, &CompressedInput[8], CompressedInput.size()-8)==OutputSize;
    }
  }
}

