//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef mwavfile_hpp
#define mwavfile_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>

#include "utility/mapfile.hpp"

#include "wavfmtchunk.hpp"

//======================================================================
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

class MWAVFile {
private:
    static constexpr auto SSDRATE = 0.037 ;
    
    util::MapFile memoryMap ;
    size_t currentOffset ;
    
    WAVFmtChunk formatChunk ;
    const std::uint8_t *ptrToData ;
    std::uint32_t  dataSize ;
    
    std::string filename ;
    
public:
    MWAVFile()  ;
    MWAVFile( const std::filesystem::path &filepath) ;
    ~MWAVFile() ;
    
    auto load(const std::filesystem::path &filepath) -> bool ;
    auto close() -> void ;
    
    auto isLoaded() const -> bool ;
    auto fileName() const -> std::string ;
    
    auto setFrame(std::int32_t frame) -> bool ;
    
    auto loadBuffer(std::uint8_t *buffer, std::uint32_t samplecount ) -> std::uint32_t ;
    
    auto frameCount() const -> std::int32_t ;
    
    auto sampleRate() const -> std::uint32_t ;
    auto channels() const -> std::uint32_t ;
};


#endif /* mwavfile_hpp */
