/*	==============
 *	FSTree_Null.cc
 *	==============
 */

#include "Genie/FS/FSTree_Null.hh"

// Standard C/C++
#include <cerrno>

// poseven
#include "poseven/types/errno_t.hh"

// Genie
#include "Genie/FS/FSTree.hh"
#include "Genie/FS/misc_method_set.hh"
#include "Genie/FS/node_method_set.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	static FSTreePtr null_parent( const FSTree* node )
	{
		p7::throw_errno( ENOENT );
		
		throw;
	}
	
	static const misc_method_set null_misc_methods =
	{
		&null_parent
	};
	
	static node_method_set null_methods =
	{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		&null_misc_methods
	};
	
	FSTreePtr FSNull()
	{
		return new FSTree( FSTreePtr(), plus::string::null, 0, &null_methods );
	}
	
}

