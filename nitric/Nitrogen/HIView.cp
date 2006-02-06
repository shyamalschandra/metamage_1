// HIView.cp

#ifndef NITROGEN_HIVIEW_H
#include "Nitrogen/HIView.h"
#endif

namespace Nitrogen {

	HIViewCreateOffscreenImage_Result HIViewCreateOffscreenImage ( HIViewRef inView, OptionBits inOptions ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewCreateOffscreenImage_Result retVal;
		CGImageRef	returnedCG;
		ThrowOSStatus ( ::HIViewCreateOffscreenImage ( inView, inOptions, &retVal.frame, &returnedCG ));
		retVal.image = Nucleus::Owned<CGImageRef>::Seize ( returnedCG );
		return retVal;
		}
	
	
#if 0
//	Returns a new, converted region. Original region is untouched
	Nucleus::Owned<RgnHandle> HIViewConvertRegion ( RgnHandle inRgn, HIViewRef inSourceView, HIViewRef inDestView ) {
		Nucleus::Owned<RgnHandle> result = CopyRgn ( inRgn );
		ThrowOSStatus ( ::HIViewConvertRegion ( result, inSourceView, inDestView ));
		return result;
		}
#endif


//	Multiple variations of "HISegmentedViewSetSegmentImage"
	void HISegmentedViewSetSegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo cinfo;
		cinfo.contentType	= kControlNoContent;
		cinfo.u.iconSuite	= NULL;	/* It's a union */
		ThrowOSStatus ( ::HISegmentedViewSetSegmentImage ( inSegmentedView, inSegmentIndexOneBased, &cinfo ));
		}
	
	void HISegmentedViewSetSegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased, IconRef iconRef ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo cinfo;
		cinfo.contentType	= kControlContentIconRef;
		cinfo.u.iconRef		= iconRef;	/* It's a union */
		ThrowOSStatus ( ::HISegmentedViewSetSegmentImage ( inSegmentedView, inSegmentIndexOneBased, &cinfo ));
		}
	
	void HISegmentedViewSetSegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased, CGImageRef imageRef ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo cinfo;
		cinfo.contentType	= kControlContentCGImageRef;
		cinfo.u.imageRef	= imageRef;	/* It's a union */
		ThrowOSStatus ( ::HISegmentedViewSetSegmentImage ( inSegmentedView, inSegmentIndexOneBased, &cinfo ));
		}
	
#if 0
	template < Nucleus::Owned<CGImageRef> > HISegmentedViewCopySegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo result;
		cinfo.contentType	= kControlContentCGImageRef;
		cinfo.u.imageRef	= NULL;	/* It's a union */
		ThrowOSStatus ( ::HISegmentedViewCopySegmentImage ( inSegmentedView, inSegmentIndexOneBased, &result ));
		return Nucleus::Owned<CGImageRef>::Seize ( cinfo.u.imageRef );
		}
	
//	!!! Marshall says "Ewwww!"
//	!!! Should we have an Nucleus::Owned<HIViewImageContentInfo> here?
//	We should have a whole series of these, maybe templated on what kind of info we want to return
	inline HIViewImageContentInfo HISegmentedViewCopySegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo retVal;
		ThrowOSStatus ( ::HISegmentedViewCopySegmentImage ( inSegmentedView, inSegmentIndexOneBased, &retVal ));
		return retVal;
		}

	template <typename returnType, int returnTag>
	Nucleus::Owned<returnType> HISegmentedViewCopySegmentImage ( HIViewRef inSegmentedView, UInt32 inSegmentIndexOneBased ) {
     	Nucleus::OnlyOnce<RegisterHIViewErrors>();
		HIViewImageContentInfo cinfo;
		cinfo.contentType = returnTag;
		cinfo.u.iconSuite = NULL;	/* It's a union */
		ThrowOSStatus ( ::HISegmentedViewCopySegmentImage ( inSegmentedView, inSegmentIndexOneBased, &cinfo ));
		return Nucleus::Owned<returnType>::Seize ( * ((returnType *) &(cinfo.u)));
		};
#endif
			
	void RegisterHIViewErrors () {
	//	Apple has not documented any at this time
		}

	}
