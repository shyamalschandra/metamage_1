// MacTypes.cp

#ifndef NITROGEN_MACTYPES_H
#include "Nitrogen/MacTypes.h"
#endif

//  These are weird ones:
//  They are some member functions of std::char_traits < UTF16Char > that the linker wants.
//  They are not needed in CodeWarrior, because CW8 has two byte wchar_t(s), and so the standard
//  libraries provide them. In gcc, however, wchar_t takes up 4 bytes, and therefore the standard
//  library does not provide char_traits for 2 byte characters
//

//#ifndef	__MWERKS__
#if 0


#include <ext/algorithm>

namespace std
{
	
	UTF16Char * char_traits< UTF16Char >::copy( UTF16Char* dest, UTF16Char const* src, size_t n )
	{
		std::copy( src, src + n, dest );
		return dest;
	}

	int char_traits< UTF16Char >::compare( UTF16Char const* s1, UTF16Char const* s2, size_t n )
	{
		return __gnu_cxx::lexicographical_compare_3way( s1, s1 + n, s2, s2 + n );
	}

	UTF16Char* char_traits< UTF16Char >::move( UTF16Char* dest, const UTF16Char* src, size_t n )
	{
		if ( src < dest )
		{
			std::copy( src, src + n, dest );
		}
		else if ( src > dest )
		{
			std::copy_backward( src, src + n, dest );
		}
		
		return dest;
	}

	UTF16Char* char_traits< UTF16Char >::assign( UTF16Char* dest, size_t n, UTF16Char c )
	{
		std::fill( dest, dest + n, c );
		return dest;
	}

}

#endif

