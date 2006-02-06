// ========================
// OpenTransportProviders.h
// ========================

#ifndef NITROGEN_OPENTRANSPORTPROVIDERS_H
#define NITROGEN_OPENTRANSPORTPROVIDERS_H

// Universal Interfaces
#ifndef __OPENTRANSPORTPROVIDERS__
#include <OpenTransportProviders.h>
#endif

// Standard C++
#include <vector>

// Nitrogen core
#ifndef NUCLEUS_IDTYPE_H
#include "Nucleus/IDType.h"
#endif
#ifndef NUCLEUS_INITIALIZE_H
#include "Nucleus/Initialize.h"
#endif
#ifndef NUCLEUS_SELECTORTYPE_H
#include "Nucleus/SelectorType.h"
#endif

// Nitrogen / Carbon support
#ifndef NITROGEN_OPENTRANSPORT_H
#include "Nitrogen/OpenTransport.h"
#endif

namespace Nitrogen
{
	
	class DefaultInternetServicesPath {};
	
}

#ifdef kDefaultInternetServicesPath
#undef kDefaultInternetServicesPath

// This definition is useless to clients because OTOpenInternetServices[InContext]() takes
// Nucleus::Owned< OTConfigurationRef >, not OTConfigurationRef.  You can't get there from here.
// Instead, we define a new class and overload on that.

//static const OTConfigurationRef kDefaultInternetServicesPath = (OTConfigurationRef)-3L;

static const Nitrogen::DefaultInternetServicesPath
       kDefaultInternetServicesPath = Nitrogen::DefaultInternetServicesPath();

#endif

// However, we still need to use it internally, so we define it under a different name.

namespace Nitrogen
{
	
	static const OTConfigurationRef
	       kDefaultInternetServicesPath_Internal = (OTConfigurationRef)-3L;
	
}


#ifdef SET_TOS
#undef SET_TOS

inline unsigned char SET_TOS( unsigned char prec, unsigned char tos )
{
	return (0x7 & prec) << 5  |  (0x1c & tos);
}

#endif

#ifdef OTOpenInternetServices
#undef OTOpenInternetServices
#endif


namespace Nitrogen
{
	
	class InetPort_Tag {};
	typedef Nucleus::IDType< InetPort_Tag, ::InetPort, 0 > InetPort;
	
	/*
	class InetHost_Tag {};
	typedef Nucleus::IDType< InetHost_Tag, ::InetHost, 0 > InetHost;
	*/
	
	// Default InetHost is kOTAnyInetAddress (i.e. 0).
	// Alternative choice is 0xFFFFFFFF (no address).
	
	class InetHost
	{
		private:
			union
			{
				::InetHost address;
				unsigned char octets[ 4 ];  // This makes debugging easier
			};
		
		public:
			InetHost() : address( kOTAnyInetAddress )  {}
			
			InetHost( ::InetHost addr ) : address( addr )  {}
			
			InetHost( unsigned char a,
			          unsigned char b,
			          unsigned char c,
			          unsigned char d ) : address( a << 24 | b << 16 | c << 8 | d )  {}
			
			static InetHost Make( ::InetHost addr )  { return InetHost( addr ); }
			
			::InetHost Get() const  { return address; }
			operator ::InetHost() const  { return Get(); }
	};
	
	using ::InetSvcRef;
	
  }

namespace Nucleus
  {
	template <> struct OwnedDefaults< Nitrogen::InetSvcRef > : OwnedDefaults< Nitrogen::ProviderRef > {};
  }

namespace Nitrogen
  {	
	using ::InetAddress;
	
	typedef std::string InetDomainName;
	
	using ::InetHostInfo;
	using ::InetSysInfo;
	using ::InetMailExchange;
	
	InetAddress& OTInitInetAddress( InetAddress& addr, InetPort port, InetHost host );
  }

namespace Nucleus
  {	
	template <>
	struct Initializer< Nitrogen::InetAddress >
	{
		Nitrogen::InetAddress& operator()( Nitrogen::InetAddress& addr, Nitrogen::InetPort port, Nitrogen::InetHost host )
		{
			return Nitrogen::OTInitInetAddress( addr, port, host );
		}
	};
  }

namespace Nitrogen
  {	
	#pragma mark -
	#pragma mark � IP address encoding �
	
	InetHost OTInetStringToHost( const char* str );
	
	inline InetHost OTInetStringToHost( const std::string& str )
	{
		return OTInetStringToHost( str.c_str() );
	}
	
	std::string OTInetHostToString( InetHost host );
  }

namespace Nucleus
  {	
	template <>
	struct Converter< Nitrogen::InetHost, const char* > : std::unary_function< const char*, Nitrogen::InetHost >
	{
		Nitrogen::InetHost operator()( const char* input ) const
		{
			return Nitrogen::OTInetStringToHost( input );
		}
	};
	
	template <>
	struct Converter< Nitrogen::InetHost, std::string > : std::unary_function< std::string, Nitrogen::InetHost >
	{
		Nitrogen::InetHost operator()( const std::string& input ) const
		{
			return Nitrogen::OTInetStringToHost( input );
		}
	};
	
	template <>
	struct Converter< std::string, Nitrogen::InetHost > : std::unary_function< Nitrogen::InetHost, std::string >
	{
		std::string operator()( Nitrogen::InetHost input ) const
		{
			return Nitrogen::OTInetHostToString( input );
		}
	};
  }

namespace Nitrogen
  {	
	#pragma mark -
	#pragma mark � DNR �
	
	Nucleus::Owned< InetSvcRef > OTOpenInternetServicesInContext( Nucleus::Owned< OTConfigurationRef >  cfig,
	                                                     OTClientContextPtr           clientContext = NULL );
	
	Nucleus::Owned< InetSvcRef > OTOpenInternetServicesInContext( DefaultInternetServicesPath,
	                                                     OTClientContextPtr           clientContext = NULL );
	
	inline Nucleus::Owned< InetSvcRef > OTOpenInternetServices( Nucleus::Owned< OTConfigurationRef > cfig )
	{
		return OTOpenInternetServicesInContext( cfig );
	}
	
	inline Nucleus::Owned< InetSvcRef > OTOpenInternetServices( DefaultInternetServicesPath )
	{
		return OTOpenInternetServicesInContext( kDefaultInternetServicesPath );
	}
	
	InetHostInfo& OTInetStringToAddress( InetSvcRef ref, char* name, InetHostInfo& hInfo );
	
	inline InetHostInfo OTInetStringToAddress( InetSvcRef ref, char* name )
	{
		InetHostInfo result;
		
		return OTInetStringToAddress( ref, name, result );
	}
	
	InetDomainName OTInetAddressToName( InetSvcRef ref, InetHost addr );
	
	// OTInetSysInfo
	
	void OTInetMailExchange( InetSvcRef ref,
	                         char* name,
	                         std::vector< InetMailExchange >& mx );
	
}

#endif

