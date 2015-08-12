//
//  PathHelper.cpp
//  RicketyRacquet
//
//  Created by Sebastian Kohl on 13.11.14.
//
//

#include "PathHelper.hpp"

#ifndef WINDOWS
#include "../MacHelper.hpp"
#endif

std::string resourcePath()
{
#ifdef WINDOWS
    return "";
#else
    return resourcePathApple();
#endif
}

std::string documentPath()
{
#ifdef IOS
    return documentPathIos();
#else
    return resourcePath();
#endif
}
