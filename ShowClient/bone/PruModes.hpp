// Copyright © 2024 Charles Kerr. All rights reserved.

#ifndef PruModes_hpp
#define PruModes_hpp

enum class PruNumber {
  zero=0,one=1,invalid=2
};

enum class PruModes {
    SSD,DMX,WS2812,UNKNOWN
};
enum class PruModeSize {
    SSD = 3072, DMX = 512 , WS2812 = 8176, UNKNOWN = 0
};


#endif /* PruModes_hpp */
