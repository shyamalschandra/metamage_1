// HITheme.cp

#ifndef NITROGEN_THEME_H
#include "Nitrogen/HITheme.h"
#endif

namespace Nitrogen {
	
	/*
	HIThemeErrorsRegistrationDependency::HIThemeErrorsRegistrationDependency()
	{
		// does nothing, but guarantees construction of theRegistration
	}
	*/
	
	
	static void RegisterHIThemeErrors();
	
	
	class HIThemeErrorsRegistration
	{
		public:
			HIThemeErrorsRegistration()  { RegisterHIThemeErrors(); }
	};
	
	static HIThemeErrorsRegistration theRegistration;
	
	
	void RegisterHIThemeErrors () {
	//	Apple hasn't documented any yet!
		}
	
	}
