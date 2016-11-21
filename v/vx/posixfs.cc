/*
	posixfs.cc
	----------
*/

#include "posixfs.hh"

#ifdef __APPLE__
#include <CoreServices/CoreServices.h>
#endif

// POSIX
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Standard C
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// plus
#include "plus/integer.hh"
#include "plus/posix/dirname.hh"

// vlib
#include "vlib/types.hh"
#include "vlib/iterators/list_builder.hh"
#include "vlib/types/integer.hh"
#include "vlib/types/stdint.hh"
#include "vlib/types/string.hh"
#include "vlib/types/vector.hh"

// vx
#include "exception.hh"


namespace vlib
{
	
	static
	void fd_error( int fd, int err = errno )
	{
		Value error( "errno", Op_mapping, Integer ( err ) );
		Value desc ( "desc",  Op_mapping, strerror( err ) );
		Value fd_  ( "fd",    Op_mapping, Integer ( fd  ) );
		
		Value exception( error, Value( desc, fd_ ) );
		
		throw_exception_object( exception );
	}
	
	static
	void path_error( const char* path, int err = errno )
	{
		Value error( "errno", Op_mapping, Integer ( err ) );
		Value desc ( "desc",  Op_mapping, strerror( err ) );
		Value path_( "path",  Op_mapping, path );
		
		Value exception( error, Value( desc, path_ ) );
		
		throw_exception_object( exception );
	}
	
	static
	void path_error( const char* path, const char* msg )
	{
		Value desc ( "desc",  Op_mapping, msg  );
		Value path_( "path",  Op_mapping, path );
		
		Value exception( desc, path_ );
		
		throw_exception_object( exception );
	}
	
	static const char* file_types[] =
	{
		"(0)",
		"FIFO",
		"CHR",
		"(3)",
		"DIR",
		"(5)",
		"BLK",
		"(7)",
		"REG",
		"(9)",
		"LNK",
		"(11)",
		"SOCK",
		"(13)",
		"(14)",
		"(15)",
	};
	
	static
	Value make_stat( const struct stat& st )
	{
		list_builder result;
		
		const char* type = file_types[ (st.st_mode & S_IFMT) >> 12 ];
		
		result.append( Value( "type", Op_mapping, type ) );
		
		#define APPEND( field )  \
		result.append( Value( #field, Op_mapping, Integer( st.st_##field ) ) )
		
		APPEND( dev   );
		APPEND( ino   );
		APPEND( mode  );
		APPEND( nlink );
		APPEND( uid   );
		APPEND( gid   );
		APPEND( rdev  );
		APPEND( atime );
		APPEND( mtime );
		APPEND( ctime );
		
		APPEND( size    );
		APPEND( blocks  );
		APPEND( blksize );
		
		#undef APPEND
		
		return Value( string_vtype, Op_empower, make_array( result ) );
	}
	
	
	static
	Value v_append( const Value& v )
	{
		const char*          path = first( v ).string().c_str();
		const plus::string&  data = rest ( v ).string();
		
		int fd = open( path, O_WRONLY | O_APPEND, 0 );
		
		if ( fd < 0 )
		{
			fd_error( fd );
		}
		
		const size_t size = data.size();
		
		ssize_t n_written = write( fd, data.data(), size );
		
		if ( n_written < size )
		{
			int saved_errno = errno;
			
			close( fd );
			
			path_error( path, n_written < 0 ? saved_errno : EINTR );
		}
		
		close( fd );
		
		return Value();
	}
	
	static
	Value v_close( const Value& v )
	{
		const int fd = v.number().clipped();
		
		int nok = close( fd );
		
		if ( nok )
		{
			fd_error( fd );
		}
		
		return Value();
	}
	
	static
	Value v_dirname( const Value& v )
	{
		return plus::dirname( v.string() );
	}
	
	static
	Value v_dup( const Value& v )
	{
		const int fd = v.number().clipped();
		
		int new_fd = dup( fd );
		
		if ( new_fd < 0 )
		{
			fd_error( fd );
		}
		
		return Integer( new_fd );
	}
	
	static
	Value v_dup2( const Value& v )
	{
		const int old_fd = first( v ).number().clipped();
		const int req_fd = rest ( v ).number().clipped();
		
		int new_fd = dup2( old_fd, req_fd );
		
		if ( new_fd < 0 )
		{
			fd_error( old_fd );
		}
		
		// new_fd should be same as req_fd
		
		return Integer( new_fd );
	}
	
	static
	Value v_fstat( const Value& v )
	{
		const int fd = v.number().clipped();
		
		struct stat st;
		
		int nok = fstat( fd, &st );
		
		if ( nok )
		{
			fd_error( fd );
		}
		
		return make_stat( st );
	}
	
	static
	Value v_listdir( const Value& v )
	{
		const char* path = v.string().c_str();
		
		DIR* dir = opendir( path );
		
		if ( dir == NULL )
		{
			path_error( path );
		}
		
		list_builder list;
		
		while ( const dirent* entry = readdir( dir ) )
		{
			const char* name = entry->d_name;
			
			if ( name[ 0 ] == '.'  &&  name[ 1 + (name[ 1 ] == '.') ] == '\0' )
			{
				// "." or ".."
				continue;
			}
			
			list.append( name );
		}
		
		closedir( dir );
		
		return make_array( list );
	}
	
	static
	Value v_load( const Value& v )
	{
		const char* path = v.string().c_str();
		
		/*
			Open nonblocking in case it's a FIFO.
			Nonblocking I/O shouldn't affect reading from a regular file.
		*/
		
		int fd = open( path, O_RDONLY | O_NONBLOCK );
		
		if ( fd < 0 )
		{
			path_error( path );
		}
		
		struct stat st;
		
		int nok = fstat( fd, &st );
		
		if ( nok )
		{
			int saved_errno = errno;
			
			close( fd );
			
			path_error( path, saved_errno );
		}
		
		if ( ! S_ISREG( st.st_mode ) )
		{
			close( fd );
			
			path_error( path, "not a regular file" );
		}
		
		const off_t len = st.st_size;
		
		plus::string result;
		
		char* p = result.reset( len );
		
		ssize_t n_read = read( fd, p, len );
		
		if ( n_read < 0 )
		{
			int saved_errno = errno;
			
			close( fd );
			
			path_error( path, saved_errno );
		}
		
		close( fd );
		
		if ( n_read != len )
		{
			path_error( path, "unexpected short read" );
		}
		
		return result;
	}
	
	static
	Value v_lstat( const Value& v )
	{
		const char* path = v.string().c_str();
		
		struct stat st;
		
		int nok = lstat( path, &st );
		
		if ( nok )
		{
			path_error( path );
		}
		
		return make_stat( st );
	}
	
	static
	Value v_pipe( const Value& v )
	{
		int pipe_ends[ 2 ];
		
		int nok = pipe( pipe_ends );
		
		if ( nok )
		{
			path_error( "<pipe>" );
		}
		
		return Value( Integer( pipe_ends[ 0 ] ),
		              Integer( pipe_ends[ 1 ] ) );
	}
	
	static
	Value v_read( const Value& v )
	{
		const int    fd = first( v ).number().clipped();
		const size_t n  = rest ( v ).number().clipped();
		
		plus::string s;
		
		char* buffer = s.reset( n );
		
		ssize_t n_read = read( fd, buffer, n );
		
		if ( n_read < 0 )
		{
			fd_error( fd );
		}
		
		return make_vector( s.substr( 0, n_read ) );
	}
	
	static
	Value v_realpath( const Value& v )
	{
		const char* path = v.string().c_str();
		
	#if TARGET_API_MAC_OSX  &&  ! MAC_OS_X_VERSION_10_6
		
		char buffer[ PATH_MAX ];
		
	#else
		
		char* const buffer = NULL;
		
	#endif
		
		const char* real = realpath( path, buffer );
		
		if ( real == NULL )
		{
			path_error( path );
		}
		
		const size_t len = strlen( real );
		
		if ( buffer != NULL )
		{
			return plus::string( real, len );
		}
		
		plus::string s( real, len, plus::delete_free );
		
		return plus::string( s.data(), len );
	}
	
	static
	Value v_stat( const Value& v )
	{
		const char* path = v.string().c_str();
		
		struct stat st;
		
		int nok = stat( path, &st );
		
		if ( nok )
		{
			path_error( path );
		}
		
		return make_stat( st );
	}
	
	static
	Value v_truncate( const Value& v )
	{
		const char*  path   = first( v ).string().c_str();
		const off_t  offset = rest ( v ).number().clipped();
		
		int nok = truncate( path, offset );
		
		if ( nok )
		{
			path_error( path );
		}
		
		return Value();
	}
	
	static
	Value v_write( const Value& v )
	{
		const int           fd = first( v ).number().clipped();
		const plus::string& s  = rest ( v ).string();
		
		const char*  buffer = s.data();
		const size_t n      = s.size();
		
		ssize_t n_written = write( fd, buffer, n );
		
		if ( n_written < 0 )
		{
			fd_error( fd );
		}
		
		return Integer( n_written );
	}
	
	static const Value c_str = c_str_vtype;
	static const Value int32 = i32_vtype;
	
	static const Value int32_x2( i32_vtype, i32_vtype );
	static const Value i32_u32 ( i32_vtype, u32_vtype );
	
	static const Value bytes( string_vtype, Op_union, vector_vtype );
	static const Value i32_bytes( i32_vtype, bytes );
	
	static const Value c_str_bytes( c_str_vtype, bytes );
	static const Value c_str_u32( c_str_vtype, u32_vtype );
	
	const proc_info proc_close    = { "close",    &v_close,    &int32 };
	const proc_info proc_dirname  = { "dirname",  &v_dirname,  &c_str };
	const proc_info proc_dup      = { "dup",      &v_dup,      &int32 };
	const proc_info proc_dup2     = { "dup2",     &v_dup2,     &int32_x2 };
	const proc_info proc_fstat    = { "fstat",    &v_fstat,    &int32 };
	const proc_info proc_listdir  = { "listdir",  &v_listdir,  &c_str };
	const proc_info proc_load     = { "load",     &v_load,     &c_str };
	const proc_info proc_lstat    = { "lstat",    &v_lstat,    &c_str };
	const proc_info proc_pipe     = { "pipe",     &v_pipe,     &empty_list };
	const proc_info proc_read     = { "read",     &v_read,     &i32_u32 };
	const proc_info proc_realpath = { "realpath", &v_realpath, &c_str };
	const proc_info proc_stat     = { "stat",     &v_stat,     &c_str };
	const proc_info proc_write    = { "write",    &v_write,    &i32_bytes };
	
	const proc_info proc_append   = { "append",   &v_append,   &c_str_bytes };
	const proc_info proc_truncate = { "truncate", &v_truncate, &c_str_u32 };
	
}
