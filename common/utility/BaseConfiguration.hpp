// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef BaseConfiguration_hpp
#define BaseConfiguration_hpp

#include <istream>
#include <filesystem>

class BaseConfiguration {
    std::filesystem::path configuration_file ;
    std::filesystem::file_time_type lastRead ;
    auto hasBeenUpdated(const std::filesystem::path &path) const -> bool ;
    auto parse(std::istream &input ) -> void ;
    
    virtual auto processKeyValue(const std::string &key, const std::string &value) ->void = 0;
    
public:
    BaseConfiguration() = default ;
    virtual ~BaseConfiguration() = default ;
    
    auto load(const std::filesystem::path &path) -> bool ;
    auto refresh() -> bool ;
};

#endif /* BaseConfiguration_hpp */
