//Copyright © 2023 Charles Kerr. All rights reserved.

#include "mapfile.hpp"

#include <algorithm>
#include <stdexcept>

#if defined(_WIN32)
#include <Windows.h>

#else
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#endif
using namespace std::string_literals ;

namespace util {
    //==================================================================================
    // MutableMapFile
    //==================================================================================
    
    //==================================================================================
    MutableMapFile::MutableMapFile():ptr(nullptr),size(0){
    }
    
    //==================================================================================
    MutableMapFile::~MutableMapFile() {
        if (ptr!= nullptr) {
            unmap() ;
        }
    }
    
    //==================================================================================
    auto MutableMapFile::map(const std::filesystem::path &path, size_t length, size_t offset) -> std::uint8_t* {
        if (ptr != nullptr) {
            unmap();
        }
        
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Unable to map nonexistent file: "s + path.string());
        }
        auto filesize = static_cast<size_t>(std::filesystem::file_size(path)) ;
        if (length == 0) {
            length = filesize ;
        }
        
#if defined(_WIN32)
        auto hfile = ::CreateFileA(path.string().c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
        if (hfile == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Unable to open: "s + path.string());
        }
        auto hmap = CreateFileMapping(hfile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (hmap == nullptr) {
            CloseHandle(hfile);
            throw std::runtime_error("Error mapping file: "s + path.string());
        }
        // We need to figure out offset high/low for this call, riht now use zero
        ptr = reinterpret_cast<std::uint8_t*>(MapViewOfFile(hmap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, length));
        if (ptr == nullptr) {
            CloseHandle(hmap);
            CloseHandle(hfile);
            throw std::runtime_error("Error mapping file: "s + path.string());
        }
        CloseHandle(hmap);
        CloseHandle(hfile);
        size = length;
        
        
#else
        
        // we need to get a fd (and the size of the file)
        auto fd = ::open(path.string().c_str(),O_RDWR) ;
        if (fd < 0) {
            throw std::runtime_error( "Unable to open: "s + path.string() ) ;
        }
        auto temp = mmap(NULL,length,PROT_READ | PROT_WRITE, MAP_SHARED,fd,offset) ;
        ::close(fd) ;
        if (temp == MAP_FAILED) {
            size = 0 ;
            ptr = nullptr ;
            throw std::runtime_error( "Unable to map file: "s + path.string() );
        }
        size = length ;
        ptr = reinterpret_cast<std::uint8_t*>(temp) ;
        
#endif
        return ptr ;
    }
    
    //==================================================================================
    auto MutableMapFile::unmap() -> void {
        if (ptr == nullptr){
            size = 0 ;
            return ;
        }
#if defined(_WIN32)
        UnmapViewOfFile(reinterpret_cast<void*>(ptr));
#else
        munmap(ptr,size) ;
#endif
    }
    //==================================================================================
    // MapFile
    //==================================================================================
    
    //==================================================================================
    MapFile::MapFile():ptr(nullptr),size(0){
        
    }
    
    //==================================================================================
    MapFile::~MapFile(){
        if (ptr!= nullptr) {
            unmap() ;
        }
    }
    
    //==================================================================================
    auto MapFile::map(const std::filesystem::path &path, size_t length, size_t offset) -> const std::uint8_t* {
        if (ptr != nullptr) {
            unmap();
        }
        
        if (!std::filesystem::exists(path)) {
            throw std::runtime_error("Unable to map nonexistent file: "s + path.string());
        }
        auto filesize = static_cast<size_t>(std::filesystem::file_size(path)) ;
        if (length == 0) {
            length = filesize ;
        }
        
#if defined(_WIN32)
        auto hfile = ::CreateFileA(path.string().c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
        if (hfile == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Unable to open: "s + path.string());
        }
        auto hmap = CreateFileMapping(hfile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (hmap == nullptr) {
            CloseHandle(hfile);
            throw std::runtime_error("Error mapping file: "s + path.string());
        }
        // We need to figure out offset high/low for this call, riht now use zero
        ptr = const_cast<const std::uint8_t *>(reinterpret_cast< std::uint8_t*>(MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, length)));
        if (ptr == nullptr) {
            CloseHandle(hmap);
            CloseHandle(hfile);
            throw std::runtime_error("Error mapping file: "s + path.string());
        }
        CloseHandle(hmap);
        CloseHandle(hfile);
        size = length;
        
#else
        
        // we need to get a fd (and the size of the file)
        auto fd = ::open(path.string().c_str(),O_RDONLY) ;
        if (fd < 0) {
            throw std::runtime_error( "Unable to open: "s + path.string() ) ;
        }
        auto temp = mmap(NULL,length,PROT_READ , MAP_SHARED,fd,offset) ;
        ::close(fd) ;
        if (temp == MAP_FAILED) {
            size = 0 ;
            ptr = nullptr ;
            throw std::runtime_error( "Unable to map file: "s + path.string() );
        }
        size = length ;
        ptr = const_cast<const std::uint8_t*>(reinterpret_cast<std::uint8_t*>(temp)) ;
        
#endif
        return ptr ;
    }
    
    //==================================================================================
    auto MapFile::unmap() -> void {
        if (ptr == nullptr){
            size = 0 ;
            return ;
        }
#if defined(_WIN32)
        UnmapViewOfFile(reinterpret_cast<void*>(const_cast<std::uint8_t*>(ptr)));
        
#else
        munmap(const_cast<std::uint8_t*>(ptr),size) ;
#endif
    }
    
}
