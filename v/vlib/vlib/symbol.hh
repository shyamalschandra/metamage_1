/*
	symbol.hh
	---------
*/

#ifndef VLIB_SYMBOL_HH
#define VLIB_SYMBOL_HH

// plus
#include "plus/string.hh"

// vlib
#include "vlib/value.hh"


namespace vlib
{
	
	class Symbol
	{
		private:
			plus::string  its_name;
			Value         its_vtype;
			Value         its_value;
			symbol_type   its_type;
		
		public:
			Symbol() : its_type()
			{
			}
			
			Symbol( symbol_type          type,
					const plus::string&  name,
					const Value&         value = Value_undefined )
			:
				its_name( name ),
				its_value( value ),
				its_type( type )
			{
			}
			
			void constify()  { its_type = Symbol_const; }
			
			void denote( const Value& vtype );
			
			void expire()
			{
				its_value = Value_expired;
			}
			
			void assign( const Value& v, bool coercive = false );
			
			Value& deref();
			
			const Value& get() const  { return its_value; }
			
			const Value& vtype() const  { return its_vtype; }
			
			bool is_const() const  { return its_type == Symbol_const; }
			bool is_var()   const  { return its_type != Symbol_const; }
			
			const plus::string& name() const  { return its_name; }
			
			bool is_defined() const  { return ! is_undefined( its_value ); }
			
			bool is_immutable() const  { return is_const()  &&  is_defined(); }
			
			Value clone() const;
	};
	
	inline
	void assign_symbol( Symbol* symbol, const Value& value )
	{
		symbol->assign( value );
	}
	
	inline
	const Value& lookup_symbol( const Symbol* symbol )
	{
		return symbol->get();
	}
	
	inline
	Value& modify_symbol( Symbol* symbol )
	{
		return symbol->deref();
	}
	
}

#endif
