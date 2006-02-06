// IBCarbonRuntime.h

#ifndef NITROGEN_IBCARBONRUNTIME_H
#define NITROGEN_IBCARBONRUNTIME_H

#ifndef NITROGEN_FRAMEWORKHEADER_H
#include "Nitrogen/FrameworkHeader.h"
#endif
#ifndef __IBCARBONRUNTIME__
#include FRAMEWORK_HEADER(HIToolbox,IBCarbonRuntime.h)
#endif
#ifndef NUCLEUS_OWNED_H
#include "Nucleus/Owned.h"
#endif
#ifndef NITROGEN_MACWINDOWS_H
#include "Nitrogen/MacWindows.h"
#endif
#ifndef NITROGEN_MENUS_H
#include "Nitrogen/Menus.h"
#endif

namespace Nitrogen
  {
   using ::IBNibRef;
   
  }

namespace Nucleus
  {
   template <> struct Disposer< Nitrogen::IBNibRef >: public std::unary_function< Nitrogen::IBNibRef, void >
     {
      void operator()( Nitrogen::IBNibRef n ) const
        {
         ::DisposeNibReference( n );
        }
     };
  }

namespace Nitrogen
  {
   
   Nucleus::Owned< IBNibRef > CreateNibReference( CFStringRef inNibName );
   Nucleus::Owned< IBNibRef > CreateNibReferenceWithCFBundle( CFBundleRef inBundle, CFStringRef inNibName );

   inline void DisposeNibReference( Nucleus::Owned<IBNibRef> /*toDispose*/ )
     {
     }

   Nucleus::Owned< WindowRef > CreateWindowFromNib ( IBNibRef inNibRef, CFStringRef inName );
   Nucleus::Owned< MenuRef >   CreateMenuFromNib   ( IBNibRef inNibRef, CFStringRef inName );
#if 0
   Nucleus::Owned< Handle >    CreateMenuBarFromNib( IBNibRef inNibRef, CFStringRef inName );
#endif
   void SetMenuBarFromNib( IBNibRef inNibRef, CFStringRef inName );
   
   void RegisterInterfaceBuilderServicesErrors();
  }

#endif
