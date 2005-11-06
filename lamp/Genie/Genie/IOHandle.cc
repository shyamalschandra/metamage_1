/*	===========
 *	IOHandle.cc
 *	===========
 */

#include "Genie/IOHandle.hh"

// Io
#include "Io/Exceptions.hh"

// Genie
#include "Genie/ResourceTable.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	typedef ResourceTable< IOHandle > IOTable;
	
	static unsigned int Poll( std::size_t offset )
	{
		//return IOTable::Table().at( offset ).resource->Poll();
		
		return kPollRead | kPollWrite;
	}
	
	void RegisterGenericIORefMod();
	void RegisterGenericIORefMod()
	{
		RegisterIOType( kGenericIODescriptor,
		                IOTable::RefMod,
		                IOTable::Read,
		                IOTable::Write,
		                Poll );
	}
	
	
	IORef NewGenericIO( const Io::Handle& io )
	{
		N::OnlyOnce< RegisterGenericIORefMod >();
		
		IOHandle* newIO = new IOHandle( io );
		
		std::size_t offset = IOTable::Add( std::auto_ptr< IOHandle >( newIO ) );
		
		return IORef( kGenericIODescriptor, offset, newIO );
	}
	
}

