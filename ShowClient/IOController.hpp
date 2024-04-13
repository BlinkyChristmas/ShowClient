// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef IOController_hpp
#define IOController_hpp

#include <iostream>
#include <mutex>
#include <filesystem>

class IOController {
    
protected:
    mutable std::mutex frame_access ;
    int current_frame;
    
    bool is_loaded ;
    bool has_error ;
    bool is_enabled ;
    bool is_playing ;
    
    std::filesystem::path data_location ;
    std::string data_extension ;
    std::string data_name ;
  
    virtual auto userSetEnabled(bool state) -> void {}
    virtual auto userSetSync(int syncframe) -> void{} 
public:
    static constexpr int FRAMEPERIOD = 37 ;

    IOController() ;
    virtual ~IOController();
    
    auto name() const -> const std::string& ;
    auto hasError() const -> bool ;
    auto isLoaded() const -> bool ;
    auto isEnabled() const -> bool ;
    virtual auto isPlaying() const -> bool ;

    auto setEnabled(bool state) -> void ;
    
    auto setDataInformation(const std::filesystem::path &location, const std::string &extension) -> void ;
    
    virtual auto load(const std::string &dataname) -> bool = 0;
    virtual auto start(int frame  ,int frame_period ) -> bool = 0;
    virtual auto stop() -> void ;
    virtual auto clear() -> void ;
    
    auto syncFrame(int sync_frame) -> void ;
};


#endif /* IOController_hpp */
