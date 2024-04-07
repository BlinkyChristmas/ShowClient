//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef dbgutil_hpp
#define dbgutil_hpp

#include <cstdint>
#include <iostream>
#include <string>
#include "utility/timeutil.hpp"



#if defined(_DEBUG) || defined(DEBUG)
//======================================================================
#define DBGMSG( os, msg ) \
(os) << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
<< util::sysTimeToString(util::ourclock::now(),"%H:%M:%S") << ": " << (msg)  << std::endl

#else
#define DBGMSG( os, var)
#endif


#endif /* dbgutil_hpp */
