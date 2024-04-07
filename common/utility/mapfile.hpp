//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef mapfile_hpp
#define mapfile_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include <filesystem>

namespace util {
    //==================================================================================
    // MutableMapFile
    //==================================================================================
    struct MutableMapFile {
        std::uint8_t *ptr ;
        size_t size ;
        
        MutableMapFile() ;
        ~MutableMapFile() ;
        
        auto map(const std::filesystem::path &path, size_t length = 0, size_t offset = 0) -> std::uint8_t* ;
        auto unmap() -> void ;
    };
    
    //==================================================================================
    // MapFile
    //==================================================================================
    struct MapFile {
        const std::uint8_t *ptr ;
        size_t size ;
        
        MapFile() ;
        ~MapFile() ;
        
        auto map(const std::filesystem::path &path, size_t length = 0, size_t offset = 0) -> const std::uint8_t* ;
        auto unmap() -> void ;
        
    };
}
#endif /* mapfile_hpp */
