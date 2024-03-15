//Copyright © 2023 Charles Kerr. All rights reserved.

#include "mwavfile.hpp"

#include <algorithm>
#include <stdexcept>
#include <cmath>

#include "utility/dbgutil.hpp"
#include "utility/mapfile.hpp"

#include "fileheader.hpp"
#include "chunkheader.hpp"

using namespace std::string_literals ;

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
MWAVFile::MWAVFile():currentOffset(0),ptrToData(nullptr),dataSize(0) {
    
}

//======================================================================
MWAVFile::MWAVFile( const std::filesystem::path &filepath) : MWAVFile() {
    if (!load(filepath) ) {
        throw std::runtime_error("Invalid file type: "s + filepath.string());
    }
}

//=====================================================================
MWAVFile::~MWAVFile() {
    if (this->isLoaded()) {
        this->close() ;
    }
}
//======================================================================
auto MWAVFile::load(const std::filesystem::path &filepath) -> bool {
    this->close() ;
    ptrToData = nullptr ;
    dataSize = 0 ;
    this->memoryMap.map(filepath) ;
    if (memoryMap.ptr == nullptr) {
        throw std::runtime_error("Unable to map: "s + filepath.string());
    }
    if (memoryMap.size <= 12) {
        throw std::runtime_error("Invalid file type: "s + filepath.string());
    }
    try {
        auto ptr = reinterpret_cast<const std::uint8_t*>(memoryMap.ptr) ;
        auto offset = 0 ;
        // Now, lets see if this file is something we can work with?
        auto fileHeader = FileHeader(ptr) ;
        if (!fileHeader.valid()) {
            // We should unmap and return false
            DBGMSG(std::cerr, "Not a recognized wav file: "s + filepath.string());
            this->memoryMap.unmap() ;
            return false ;
        }
        offset =  12 ;
        auto chunk = ChunkHeader(ptr + offset) ;
        if (!chunk.isFormat()){
            // we have an issue, or bad assumption that format is always first)
            DBGMSG(std::cerr, "Did not find a format chunk first: "s + filepath.string());
            this->memoryMap.unmap() ;
            return false ;
        }
        formatChunk.clear() ;
        offset += 8 ;
        formatChunk.load(ptr+offset) ;
        if (!formatChunk.valid()){
            DBGMSG(std::cerr, "Seems to be not a PCM (uncompressed) or 441000 format: "s + filepath.string());
            this->memoryMap.unmap() ;
            return false ;
        }
        offset += chunk.size ;
        // Now we loop unti we get a data chunk
        while (offset < this->memoryMap.size) {
            auto chunk = ChunkHeader(ptr+offset) ;
            if (chunk.isData()){
                offset += 8 ;
                dataSize = chunk.size ;
                ptrToData = ptr + offset ;
                break;
            }
            else {
                offset += 8 + chunk.size ;
            }
        }
        return ptrToData != nullptr ;
    }
    catch (const std::exception &e) {
        DBGMSG(std::cerr, "Unable to process: "s + filepath.string() + "\n"s + e.what()) ;
        return false ;
    }
    catch (...){
        return false ;
    }
}

//======================================================================
auto MWAVFile::close() -> void {
    memoryMap.unmap() ;
    formatChunk.clear() ;
    ptrToData = nullptr ;
    dataSize = 0 ;
}

//======================================================================
auto  MWAVFile::isLoaded() const -> bool {
    return memoryMap.ptr != nullptr ;
}


//======================================================================
auto MWAVFile::setFrame(std::int32_t frame) -> bool {
    auto time = (SSDRATE * double(frame))  ;
    auto sample = std::round(double(formatChunk.sampleRate) * time) ;
    auto offset = static_cast<std::int32_t>(sample) * formatChunk.samplesize ;
    if (offset >= static_cast<int>(dataSize)){
        return false ;
    }
    currentOffset = offset ;
    return true ;
}
//======================================================================
auto MWAVFile::loadBuffer(std::uint8_t *buffer, std::uint32_t samplecount ) -> std::uint32_t {
    auto location = currentOffset ;
    auto bytecount = samplecount * formatChunk.samplesize ;
    if (location + bytecount >= dataSize) {
        bytecount = static_cast<std::uint32_t>(dataSize - location) ;
    }
    std::copy(ptrToData + location,ptrToData+location+bytecount,buffer);
    currentOffset += bytecount ;
    
    return static_cast<std::uint32_t>(bytecount/formatChunk.samplesize)   ;
}

//======================================================================
auto MWAVFile::frameCount() const -> std::int32_t {
    if (dataSize == 0 || formatChunk.sampleRate == 0 || formatChunk.samplesize == 0) {
        return 0 ;
    }
    auto seconds = ( double(dataSize) / double(formatChunk.samplesize)) / double(formatChunk.sampleRate) ;
    return std::int32_t(std::round(seconds/SSDRATE)) ;
    
}

//======================================================================
auto MWAVFile::sampleRate() const -> std::uint32_t {
    return formatChunk.sampleRate ;
}
//======================================================================
auto MWAVFile::channels() const -> std::uint32_t {
    return static_cast<std::uint32_t>(formatChunk.channelCount) ;
}
