//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef lightheader_hpp
#define lightheader_hpp

#include <cstdint>
#include <iostream>
#include <string>

/* **************************************************************************
 Offset             Size                    Name                Description
 0                  4                       signature           Indicates it is a blinky light file ("lght" big endian)
 4                  4                       version             Version of the light file
 8                  4                       dataOffset          Offset to the light data in bytes
 
 For Version 0 header
 
 12                 4                       sample rate         Milliseconds the frames are spaced apart (nominally 37)
 16                 4                       frame count         Number of frames the file contains
 20                 4                       frame length        Number of bytes in a frame ;
 24                 30                      music name          The ascii name (not extension) of the music this was synced to
 
 Data
 
 dataOffset         Nominally (6*4)+30      frame data          stream of bytes structured as:
 
 frame = [std::uint8_t]  where the length is "framelength"
 and then data is [frame] where the size is the "frame count"
 
 */
//======================================================================
struct LightHeader {
    static constexpr auto NAMESIZE = 30 ;
    static constexpr auto OFFSETTODATA = (6 * 4) + NAMESIZE ;
    static constexpr auto SIGNATURE = 0x5448474c ; //'THGL' in big endian

    std::uint32_t signature ;
    std::uint32_t version ;
    std::uint32_t offsetToData ;
    std::uint32_t sampleRate ;
    std::uint32_t frameCount ;
    std::uint32_t frameLength ;
    std::string sourceName ;  // NAMESIZE characters
    
    auto clear() -> void ;
    LightHeader() ;
    LightHeader( const char *ptr) ;
    LightHeader( std::istream &input) ;
    auto load(const char *ptr) -> void ;
    auto load(std::istream &input) -> void ;
    auto write(std::ostream &output) -> void ;
} ;

#endif /* lightheader_hpp */
