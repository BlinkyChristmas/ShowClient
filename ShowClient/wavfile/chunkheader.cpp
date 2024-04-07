//Copyright © 2023 Charles Kerr. All rights reserved.

#include "chunkheader.hpp"

#include <algorithm>
#include <stdexcept>


using namespace std::string_literals ;

/* ************************************************************************************************
 Offset     Size    Name             Description
 
 The canonical WAVE format starts with the file chunk :
 
 0          4       ChunkID         Contains the letters "RIFF" in ASCII form (0x52494646 big-endian form).
 4          4       ChunkSize       Size of all the chunks, chunk headers  + 4 (to include this headers "Format":
 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size) ... + (8 + subChunkNSize)
 8          4       Format          Contains the letters "WAVE" (0x57415645 big-endian form).
 
 It then has "chunks" that follow.  A chunk has a header, and then chunk unique data.
 
 ChunkHeader
 0          4       signature       Normally a 4 character text identifier, bigendian
 4          4       size            The size of the chunk, not including the chunk header
 
 Chunk type: format -  signature = "fmt " , (0x666d7420 big-endian format) (fyi, a PCM fmt audio format is 16 in size)
 The "fmt " subchunk describes the sound data's format:
 
 0          2       AudioFormat     PCM = 1 (i.e. Linear quantization)
 Values other than 1 indicate some form of compression.
 2          2       NumChannels     Mono = 1, Stereo = 2, etc.
 4          4       SampleRate      8000, 44100, etc.
 8          4       ByteRate        SampleRate * NumChannels * BitsPerSample/8
 12         2       Samplesize      NumChannels * BitsPerSample/8
 The number of bytes for one sample including all channels.
 14         2       BitsPerSample   8 bits = 8, 16 bits = 16, etc.
 16         2       ExtraParamSize  if PCM, then doesn't exist
 18         X       ExtraParams     space for extra parameters
 
 Chunk type: data -  signature = "data" , (0x64617461 big-endian format)
 The data follows the header
 
 Chunk type: LIST -  signature = "LIST" , (0x5453494c big-endian format)
 ************************************************************************************************ */


//======================================================================
ChunkHeader::ChunkHeader():size(0),signature(0){
}

//======================================================================
ChunkHeader::ChunkHeader(const std::uint8_t *ptr):ChunkHeader(){
    if (!load(ptr)) {
        throw std::runtime_error("Error processing wav chunk header");
    }
}

//======================================================================
auto ChunkHeader::load(const std::uint8_t *ptr) -> bool {
    try{
        std::copy(ptr,ptr+4, reinterpret_cast<std::uint8_t*>(&signature)) ;
        std::copy(ptr+4,ptr+8, reinterpret_cast<std::uint8_t*>(&size)) ;
        return true ;
    }
    catch(...) {
        return false ;
    }
}
//======================================================================
auto ChunkHeader::isFormat() const -> bool {
    return signature == FMTSIGNATURE ;
}
//======================================================================
auto ChunkHeader::isData() const -> bool {
    return signature == DATASIGNATURE ;
}

