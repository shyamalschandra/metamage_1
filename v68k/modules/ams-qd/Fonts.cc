/*
	Fonts.cc
	--------
*/

#include "Fonts.hh"

// Mac OS
#ifndef __FONTS__
#include <Fonts.h>
#endif
#ifndef __RESOURCES__
#include <Resources.h>
#endif

// POSIX
#include <unistd.h>


#define STR_LEN( s )  "" s, (sizeof s - 1)

#define SYSTEM_FONT_NOT_LOADED "warning: couldn't load the system font"

FontRec** ROMFont0 : 0x0980;
short     ApFontID : 0x0984;


static inline
short resID_for_font_and_size( short font, short size )
{
	return font * 128 + size;
}

static inline
short specific_font( short font )
{
	return font == applFont ? ApFontID : font;
}

const short default_font_size = 12;

const short system_font_resID = resID_for_font_and_size( 0, default_font_size );

static short the_current_font_resID;

static FontRec** the_current_font_record;

static FMOutput the_current_FMOutput;


pascal void InitFonts_patch()
{
	const FMInput input = { 0 };
	
	FMSwapFont( &input );
	
	ROMFont0 = the_current_font_record;
	
	if ( the_current_font_record == NULL )
	{
		write( STDERR_FILENO, STR_LEN( SYSTEM_FONT_NOT_LOADED "\n" ) );
	}
}

pascal FMOutPtr FMSwapFont_patch( const FMInput* input )
{
	const short fontNum  = specific_font( input->family );
	const short fontSize = input->size ? input->size : default_font_size;
	
	const short resID = resID_for_font_and_size( fontNum, fontSize );
	
	if ( resID != the_current_font_resID )
	{
		FontRec** newFont = ROMFont0;
		
		if ( resID != system_font_resID  ||  newFont == NULL )
		{
			if ( Handle h = GetResource( 'FONT', resID ) )
			{
				newFont = (FontRec**) h;
			}
			else if ( newFont == NULL )
			{
				return &the_current_FMOutput;
			}
		}
		
		if ( the_current_font_record != ROMFont0 )
		{
			ReleaseResource( (Handle) the_current_font_record );
		}
		
		the_current_font_resID  = resID;
		the_current_font_record = newFont;
		
		FontRec& rec = **newFont;
		
		the_current_FMOutput.fontHandle = (Handle) newFont;
		the_current_FMOutput.ascent     = rec.ascent;
		the_current_FMOutput.descent    = rec.descent;
		the_current_FMOutput.widMax     = rec.widMax;
		the_current_FMOutput.leading    = rec.leading;
		the_current_FMOutput.numer      = input->numer;
		the_current_FMOutput.denom      = input->denom;
	}
	
	return &the_current_FMOutput;
}
