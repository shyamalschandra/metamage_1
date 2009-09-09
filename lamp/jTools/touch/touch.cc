/*	========
 *	touch.cc
 *	========
 */

// Standard C/C++
#include <cstdio>
#include <cstdlib>
#include <cstring>

// POSIX
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>


#pragma exceptions off


int main( int argc, char *const *argv )
{
	// Check for sufficient number of args
	if ( argc < 2 )
	{
		std::fprintf( stderr, "touch: missing arguments\n" );
		
		return 1;
	}
	
	// Try to touch each item.  Return whether any errors occurred.
	int exit_status = EXIT_SUCCESS;
	
	for ( std::size_t index = 1;  index < argc;  ++index )
	{
		const char* pathname = argv[ index ];
		
		int result = utime( pathname, NULL );
		
		if ( result < 0 )
		{
			if ( errno == ENOENT )
			{
				result = open( pathname, O_WRONLY | O_CREAT, 0666 );
				
				if ( result >= 0 )
				{
					close( result );
					
					continue;
				}
			}
			
			std::fprintf( stderr, "touch: %s: %s\n", pathname, std::strerror( errno ) );
			
			exit_status = EXIT_FAILURE;
		}
	}
	
	return exit_status;
}

