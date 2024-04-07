//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef lightfile_hpp
#define lightfile_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>

#include "utility/mapfile.hpp"
#include "lightheader.hpp"

/* **************************************************************************
 Offset             Size                    Name                Description
 0                  4                       signature           Indicates it is a blinky light file ("lght" big endian)
 4                  4                       version             Version of the light file
 8                  4                       header size         Size in bytes of the header (does not include the 4 bytes for the header)
 
 For Version 0 header
 
 12                 4                       sample rate         Milliseconds the frames are spaced apart (nominally 37)
 16                 4                       frame count         Number of frames the file contains
 20                 4                       frame length        Number of bytes in a frame ;
 24                 30                      music name          The ascii name (not extension) of the music this was synced to
 
 Data
 
 12 + headersize    framecount*framelength  frame data          stream of bytes structured as:
 
 frame = [std::uint8_t]  where the length is "framelength"
 and then data is [frame] where the size is the "frame count"
 
 */

//======================================================================
class LightFile {
private:

    LightHeader lightHeader ;
    util::MapFile lightData ;
    
public:
    LightFile() ;
    LightFile(const std::filesystem::path &lightfile) ;
    auto loadFile(const std::filesystem::path &lightfile) -> bool ;
    
    auto isLoaded() const -> bool ;
    
    auto frameCount() const -> std::int32_t ;
    auto frameLength() const -> std::int32_t ;
    
    auto copy(std::uint32_t frame,  unsigned char *buffer, int offset = 0 , int length = 0)  -> int ;
    
    auto clear(bool nothrow = false) -> void ;
};

#endif /* lightfile_hpp */
