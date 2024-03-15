//Copyright © 2023 Charles Kerr. All rights reserved.

#ifndef dbgutil_hpp
#define dbgutil_hpp

#include <cstdint>
#include <iostream>
#include <string>

#if defined(_DEBUG) || defined(DEBUG)
//======================================================================
#define DBGMSG( os, msg ) \
(os) << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
<<  (msg)  << std::endl

#else
#define DBGMSG( os, var)
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && defined(DBGLOCK)
//======================================================================
#define DBGLCK( os, msg ) \
(os) << "DBG: " << __FILE__ << "(" << __LINE__ << ") "\
<<  (msg)  << std::endl

#else
#define DBGLCK( os, var)
#endif

#endif /* dbgutil_hpp */
