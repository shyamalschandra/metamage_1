/*
	Icons.cc
	--------
*/

#include "Icons.hh"

// Mac OS
#ifndef __ICONS__
#include <Icons.h>
#endif
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif
#ifndef __RESOURCES__
#include <Resources.h>
#endif
#ifndef __TRAPS__
#include <Traps.h>
#endif

// ams-core
#include "QDGlobals.hh"


BitMap IconBitmap : 0x0A0E;
OSErr  ResErr     : 0x0A60;


static inline
void CopyIconBits( Ptr baseAddr, const Rect* r, short mode )
{
	IconBitmap.baseAddr = baseAddr;
	
	GrafPtr port = *get_addrof_thePort();
	
	CopyBits( &IconBitmap, &port->portBits, &IconBitmap.bounds, r, mode, NULL );
}

static
pascal OSErr PlotIconID_method( const Rect*        rect,
                                IconAlignmentType  align,
                                IconTransformType  xform,
                                short              resID )
{
	Handle h = GetResource( 'ICN#', resID );
	
	if ( h == NULL )
	{
		if ( ResErr != noErr )
		{
			return ResErr;
		}
		
		return resNotFound;
	}
	
	CopyIconBits( *h + 128, rect, srcBic );
	CopyIconBits( *h,       rect, srcXor );
	
	ReleaseResource( h );
	
	return noErr;
}

asm void IconDispatch_patch( short method : __D0 )
{
	CMPI.W   #0x0500,D0
	BEQ      dispatch_PlotIconID
	
	_Debugger
	_ExitToShell
	
dispatch_PlotIconID:
	JMP      PlotIconID_method
}
