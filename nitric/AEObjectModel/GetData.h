/*	=========
 *	GetData.h
 *	=========
 */

#ifndef AEOBJECTMODEL_GETDATA_H
#define AEOBJECTMODEL_GETDATA_H

#include <map>

#ifndef NITROGEN_AEOBJECTS_HH
#include "Nitrogen/AEObjects.hh"
#endif


namespace Nitrogen
{
	
	inline nucleus::owned< AEDesc_ObjectSpecifier > GetRootObjectSpecifier()
	{
		return AEInitializeDesc< AEDesc_Data >();
	}
	
	nucleus::owned< AEDesc_ObjectSpecifier > AECreateObjectSpecifier( Mac::AEObjectClass             objectClass,
	                                                                  const AEDesc_ObjectSpecifier&  container,
	                                                                  Mac::AEKeyForm                 keyForm,
	                                                                  const AEDesc_Data&             keyData );
	
	nucleus::owned< AEDesc_Data > GetData( const AEDesc_Token& obj, Mac::DescType desiredType = Mac::typeWildCard );
	
	class DataGetter
	{
		public:
			typedef nucleus::owned< AEDesc_Data > (*Callback)( const AEDesc_Token&, Mac::DescType );
		
		private:
			typedef std::map< Mac::DescType, Callback >  Map;
			
			Map map;
			
			// not implemented:
			DataGetter( const DataGetter& );
			DataGetter& operator=( const DataGetter& );
		
		public:
			DataGetter();
			
			void Register( Mac::DescType tokenType, DataGetter::Callback callback )
			{
				map[ tokenType ] = callback;
			}
			
			nucleus::owned< AEDesc_Data > GetData( const AEDesc_Token& obj, Mac::DescType desiredType );
	};
	
	DataGetter& TheGlobalDataGetter();
	
	inline void RegisterDataGetter( Mac::DescType tokenType, DataGetter::Callback callback )
	{
		TheGlobalDataGetter().Register( tokenType, callback );
	}
	
}

#endif
