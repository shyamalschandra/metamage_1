/*	================
 *	ProjectConfig.cc
 *	================
 */

#include "CompileDriver/ProjectConfig.hh"

// Standard C++
#include <algorithm>

// Iota
#include "iota/strings.hh"

// Nucleus
#include "Nucleus/ResourceTransfer.h"

// POSeven
#include "POSeven/FileDescriptor.hh"
#include "POSeven/Pathnames.hh"

// MoreFunctional
#include "FunctionalExtensions.hh"
#include "PointerToFunction.hh"

// Orion
#include "Orion/Main.hh"

// A-line
#include "A-line/A-line.hh"
#include "A-line/Locations.hh"
#include "CompileDriver/Platform.hh"
#include "CompileDriver/ProjectCatalog.hh"


namespace CompileDriver
{
	
	namespace NN = Nucleus;
	namespace p7 = poseven;
	
	
	using namespace io::path_descent_operators;
	
	
	static bool c_string_less( const char* a, const char* b )
	{
		return std::strcmp( a, b ) < 0;
	}
	
	static bool DirectiveIsRecognized( const std::string& directive )
	{
		char const* const recognized[] =
		{
			"arch",
			"creator",
			"desc",
			"developer",
			"distributor",
			"frameworks",
			"imports",
			"name",
			"platform",
			"precompile",
			"product",
			"program",
			"rez",
			"rsrc",
			"runtime",
			"search",
			"sources",
			"subprojects",
			"tools",
			"use",
			"uses",
			"version",
			"website",
			"~"
		};
		
		std::size_t length = sizeof recognized / sizeof (const char*);
		
		char const* const* end = recognized + length;
		
		char const* const* edge = std::lower_bound( recognized + 0, end, directive.c_str(), std::ptr_fun( c_string_less ) );
		
		bool found = edge != end && directive == *edge;
		
		return found;
	}
	
	static std::map< std::string, Platform > MakePlatformMap()
	{
		std::map< std::string, Platform > map;
		
		map[ "68k" ] = arch68K;
		map[ "ppc" ] = archPPC;
		map[ "x86" ] = archX86;
		
		map[ "a4"     ] = runtimeA4CodeResource;
		map[ "a5"     ] = runtimeA5CodeSegments;
		map[ "cfm"    ] = runtimeCodeFragments;
		map[ "mach-o" ] = runtimeMachO;
		
		map[ "blue"    ] = apiMacBlue;
		map[ "classic" ] = apiMacBlue;
		map[ "carbon"  ] = apiMacCarbon;
		
		map[ "mac"  ] = platformMac;
		map[ "unix" ] = platformUnix;
		
		return map;
	}
	
	static void SetPlatformInfo( NN::ResourceTransfer< PlatformDemands >  cumulative_demands,
	                             const std::string&                       new_spec )
	{
		static std::map< std::string, Platform > map = MakePlatformMap();
		
		if ( new_spec.empty() )
		{
			return;
		}
		
		const bool inverted = new_spec[0] == '!';
		
		std::string new_prohibition;
		
		if ( inverted )
		{
			new_prohibition = new_spec.substr( 1, new_spec.npos );
		}
		
		const std::string& platform_name = inverted ? new_prohibition : new_spec;
		
		Platform platform = map[ platform_name ];
		
		if ( platform == Platform() )
		{
			throw NoSuchPlatform( platform_name );
		}
		
		PlatformDemands new_demands = PlatformDemands( platform, Platform() );
		
		if ( inverted )
		{
			new_demands = -new_demands;
		}
		
		*cumulative_demands |= new_demands;
		
		cumulative_demands->Verify();
		
		/*
			// Only classic Toolbox is supported on 68K
			// A4 implies 68K (where only classic Toolbox is available)
			// A5 implies 68K (where only classic Toolbox is available)
			// Only Carbon is supported for Mac OS X
		*/
	}
	
	static PlatformDemands MakePlatformInfo( const std::vector< std::string >& infos )
	{
		PlatformDemands result;
		
		std::for_each( infos.begin(),
		               infos.end(),
		               std::bind1st( more::ptr_fun( SetPlatformInfo ),
		                             NN::ResourceTransfer< PlatformDemands >( result ) ) );
		
		return result;
	}
	
	
	static std::vector< std::string >& Subprojects()
	{
		static std::vector< std::string > gSubprojects;
		
		return gSubprojects;
	}
	
	static std::string DescendPathToDir( const std::string& dir, const std::string& path )
	{
		return dir / path;
	}
	
	static bool ends_with( const std::string& string, const char* substring, std::size_t length )
	{
		return std::equal( string.end() - length, string.end(), substring );
	}
	
	static std::string get_project_dir_from_config_file( const std::string& config_pathname )
	{
		std::string config_dir = io::get_preceding_directory( config_pathname );
		
		const bool has_confd = ends_with( config_dir, STR_LEN( "A-line.confd/" ) );
		
		return has_confd ? io::get_preceding_directory( config_dir )
		                 :                              config_dir;
	}
	
	static void AddConfigFile( const std::string& config_pathname, const ConfData& conf )
	{
		std::string project_dir = get_project_dir_from_config_file( config_pathname );
		
		typedef ConfData::const_iterator const_iterator;
		
		const_iterator it = conf.find( "name" );
		
		std::string project_name = it != conf.end() ? it->second[ 0 ]
		                                            : io::get_filename( project_dir );
		
		it = conf.find( "platform" );
		
		PlatformDemands demands = it != conf.end() ? MakePlatformInfo( it->second )
		                                           : PlatformDemands();
		
		try
		{
			AddProjectConfigFile( project_name,
			                      demands,
			                      ProjectConfig( config_pathname, project_dir, conf ) );
		}
		catch ( const NoSuchPlatform& e )
		{
			std::fprintf( stderr, "No such platform '%s' in %s\n",
			                                         e.name.c_str(),
			                                                config_pathname.c_str() );
			
			Orion::ThrowExitStatus( EXIT_FAILURE );
		}
		
	}
	
	static void AddPendingConfigFile( const std::string& filePath )
	{
		std::string filename = io::get_filename_string( filePath );
		std::string extension = ".conf";
		
		std::string::difference_type rootSize = filename.size() - extension.size();
		
		if ( rootSize <= 0  ||  filename.substr( rootSize, std::string::npos ) != extension )
		{
			return;
		}
		
		DotConfData data;
		ReadProjectDotConf( filePath, data );
		ConfData conf = MakeConfData( data );
		
		AddConfigFile( filePath, conf );
		
		std::string project_dir = get_project_dir_from_config_file( filePath );
		
		std::vector< std::string >& conf_subprojects = conf[ "subprojects" ];
		
		std::transform( conf_subprojects.begin(),
		                conf_subprojects.end(),
		                std::back_inserter( Subprojects() ),
		                std::bind1st( more::ptr_fun( DescendPathToDir ),
		                              project_dir ) );
		
	}
	
	void AddPendingSubproject( const std::string& dir )
	{
		std::vector< std::string > configs;
		std::vector< std::string > folders;
		
		ScanDirForProjects( dir,
		                    std::back_inserter( configs ),
		                    std::back_inserter( folders ) );
		
		std::vector< std::string >& subprojects = Subprojects();
		
		subprojects.insert( subprojects.end(), folders.begin(), folders.end() );
		
		std::for_each( configs.begin(),
		               configs.end(),
		               std::ptr_fun( AddPendingConfigFile ) );
	}
	
	bool AddPendingSubprojects()
	{
		std::vector< std::string > subprojects;
		
		std::swap( subprojects, Subprojects() );
		
		std::for_each( subprojects.begin(),
		               subprojects.end(),
		               std::ptr_fun( AddPendingSubproject ) );
		
		return subprojects.size() > 0;
	}
	
	class ConfDataMaker
	{
		private:
			ConfData& conf;
		
		public:
			ConfDataMaker( ConfData& conf ) : conf( conf )  {}
			
			void operator()( const DotConfLine& line ) const
			{
				if ( !DirectiveIsRecognized( line.key ) )
				{
					std::fprintf( stderr,
					              "Unrecognized directive '%s' in project config\n",
					                                       line.key.c_str() );
				}
				
				std::vector< std::string >& conf_key = conf[ line.key ];
				
				conf_key.insert( conf_key.end(), line.values.begin(), line.values.end() );
			}
	};
	
	ConfData MakeConfData( const DotConfData& data )
	{
		ConfData conf;
		
		std::for_each( data.begin(),
		               data.end(),
		               ConfDataMaker( conf ) );
		
		return conf;
	}
	
}

