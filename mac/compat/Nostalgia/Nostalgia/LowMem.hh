/*
	Nostalgia/LowMem.hh
	-------------------
*/

#ifndef NOSTALGIA_LOWMEM_HH
#define NOSTALGIA_LOWMEM_HH

// Mac OS X
#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

#ifndef __LOWMEM__
#include <LowMem.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if TARGET_API_MAC_CARBON

inline pascal UInt32 LMGetTicks()
{
	return ::TickCount();
}

#endif

#ifdef __cplusplus
}
#endif

#endif
