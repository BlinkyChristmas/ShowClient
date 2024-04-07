// Copyright © 2024 Charles Kerr. All rights reserved.

#include "BaseConfiguration.hpp"

#include <vector>
#include <fstream>

#include "strutil.hpp"
using namespace std::string_literals ;
// ================================================================================
auto BaseConfiguration::hasBeenUpdated(const std::filesystem::path &path) const -> bool {
    auto lastwrite = std::filesystem::last_write_time(path) ;
    
    if ( std::chrono::duration_cast<std::chrono::seconds>(lastwrite - lastRead).count() > 0 ){
        return true ;
    }
    return false ;
}

// ================================================================================
auto BaseConfiguration::parse(std::istream &input ) -> void  {
    auto buffer = std::vector<char>(2048,0) ;
    
    while (input.good() && !input.eof()) {
        input.getline(buffer.data(), buffer.size()-1) ;
        if (input.gcount() > 0) {
            buffer[input.gcount()] = 0 ;
            std::string line = buffer.data() ;
            line = util::trim(util::strip(line,"#"));
            if (!line.empty()) {
                // We need to process the line
                auto [key,value] = util::split(line, "=") ;
                processKeyValue(key, value) ;
            }
        }
    }
}

// ==================================================================================
auto BaseConfiguration::load(const std::filesystem::path &path) -> bool {
    if (!std::filesystem::exists(path)) {
        return false ;
    }
    lastRead = std::filesystem::last_write_time(path)  ; // this is here, even if we cant open it, so we dont continue to retry on refresh
    auto input = std::ifstream(path.string());
    if (!input.is_open()) {
        return false ;
    }
    configuration_file = path ;
    parse(input) ;
    input.close() ;
    return true ;
}
// ==================================================================================
auto BaseConfiguration::refresh() -> bool {
    if (hasBeenUpdated(configuration_file) && !configuration_file.empty()) {
        if (this->load(configuration_file) ) {
            throw std::runtime_error("Unable to process: "s + configuration_file.string());
        }
    }
    return false ;
}
