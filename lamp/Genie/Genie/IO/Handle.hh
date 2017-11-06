/*
	Handle.hh
	---------
*/

#ifndef GENIE_IO_HANDLE_HH
#define GENIE_IO_HANDLE_HH

// nucleus
#ifndef NUCLEUS_OWNED_HH
#include "nucleus/owned.hh"
#endif

// Nitrogen
#ifndef MAC_MEMORY_TYPES_HANDLE_HH
#include "Mac/Memory/Types/Handle.hh"
#endif

// vfs
#include "vfs/filehandle.hh"


namespace Genie
{
	
	struct Mac_Handle_extra
	{
		char** handle;
	};
	
	class Handle_IOHandle : public vfs::filehandle
	{
		public:
			Handle_IOHandle( const vfs::node&               file,
			                 int                            flags,
			                 nucleus::owned< Mac::Handle >  h );
			
			~Handle_IOHandle();
			
			ssize_t Positioned_Read( char* buffer, size_t n_bytes, off_t offset );
			
			ssize_t Positioned_Write( const char* buffer, size_t n_bytes, off_t offset );
			
			void SetEOF( off_t length );
	};
	
}

#endif
