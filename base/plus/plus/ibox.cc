/*
	ibox.cc
	-------
*/

#include "plus/ibox.hh"

// Standard C++
#include <algorithm>

// iota
#include "iota/endian.hh"

// math
#include "math/integer.hh"

// debug
#include "debug/assert.hh"

// plus
#include "plus/extent.hh"


namespace plus
{
	
	using math::integer::limb_t;
	using math::integer::cmp_t;
	
	
	struct ibox_structure
	{
		union
		{
			ibox::int_t  integer;
			ibox::ptr_t  pointer;
		};
		
		ibox::size_t  size;
		ibox::sign_t  sign;
	};
	
	void ibox::destroy_extent()
	{
		extent_release( (char*) its_pointer );
	}
	
	void ibox::unshare()
	{
		if ( has_extent() )
		{
			its_pointer = (ptr_t) extent_unshare( (char*) its_pointer );
		}
	}
	
	void ibox::construct( long long i )
	{
		sign_t sign = Sign_positive;
		
		if ( i < 0 )
		{
			const int_t      max_limb = int_t( -1 );
			const long long  min_limb = -1LL * max_limb;
			
			if ( i >= min_limb )
			{
				construct( (unsigned long) -i );
				
				its_sign = Sign_negative;
				
				return;
			}
			
			// 0x8000 0000 0000 0000
			const long long nadir = 1LL << (sizeof (long long) * 8 - 1);
			
			if ( i != nadir )
			{
				i = -i;
			}
			
			sign = Sign_negative;
		}
		
		const unsigned n_limbs = sizeof (long long) / sizeof (int_t);
		
		construct( (int_t const*) &i, n_limbs, sign );
	}
	
	void ibox::construct( int_t const*  data,
	                      size_t        n,
	                      sign_t        sign )
	{
		ASSERT( n > 1 );
		
		char* extent = extent_alloc( n * sizeof (int_t) );
		
		// Copy before clobbering the struct
		
		std::copy( data, data + n, (int_t*) extent );
		
		its_pointer = (ptr_t) extent;
		its_size    = n;
		its_sign    = sign;
	}
	
	ibox::ibox( const ibox& that )
	{
		(ibox_structure&) *this = (const ibox_structure&) that;
		
		if ( that.has_extent() )
		{
			extent_add_ref( (char*) that.its_pointer );
		}
	}
	
	ibox& ibox::operator=( const ibox& that )
	{
		if ( this != &that )
		{
			destroy();
			
			(ibox_structure&) *this = (const ibox_structure&) that;
			
			if ( that.has_extent() )
			{
				extent_add_ref( (char*) that.its_pointer );
			}
		}
		
		return *this;
	}
	
	void ibox::swap( ibox& i )
	{
		ibox_structure tmp = (ibox_structure&) *this;
		
		(ibox_structure&) *this = (ibox_structure&) i;
		
		(ibox_structure&) i = tmp;
	}
	
	limb_t ibox::extent_top() const
	{
		ASSERT( has_extent() );
		
		const bool be = ! iota::is_little_endian();
		
		limb_t const* biggest = be ? its_pointer
		                           : its_pointer + its_size - 1;
		
		return *biggest;
	}
	
	limb_t ibox::extent_bottom() const
	{
		ASSERT( has_extent() );
		
		const bool le = iota::is_little_endian();
		
		limb_t const* littlest = le ? its_pointer
		                            : its_pointer + its_size - 1;
		
		return *littlest;
	}
	
	void ibox::extend( size_t n )
	{
		if ( n < its_size )
		{
			throw limb_count_overflow();
		}
		
		ASSERT( n > 1        );
		ASSERT( n > its_size );
		
		int_t const* old_data = data();
		const size_t old_size = size();
		
		const size_t n_zeros = n - old_size;
		
		int_t* const new_data = (ptr_t) extent_alloc( n * sizeof (int_t) );
		
		int_t* p = new_data;
		
		if ( iota::is_little_endian() )
		{
			std::copy( old_data, old_data + old_size, p );
			
			p += old_size;
			
			std::fill( p, p + n_zeros, 0 );
		}
		else
		{
			std::fill( p, p + n_zeros, 0 );
			
			p += n_zeros;
			
			std::copy( old_data, old_data + old_size, p );
		}
		
		destroy();
		
		its_pointer = new_data;
		its_size    = n;
	}
	
	void ibox::shrink_to_fit()
	{
		ASSERT( its_size > 1 );
		
		if ( iota::is_little_endian() )
		{
			size_t size = its_size;
			
			while ( its_pointer[ size - 1 ] == 0 )
			{
				--size;
				
				ASSERT( size != 0 );
			}
			
			its_size = size;
		}
		else
		{
			size_t n_zeros  = 0;
			
			while ( its_pointer[ n_zeros ] == 0 )
			{
				++n_zeros;
				
				ASSERT( n_zeros != its_size );
			}
			
			if ( n_zeros != 0 )
			{
				std::copy( its_pointer + n_zeros,
				           its_pointer + its_size,
				           its_pointer );
			}
			
			its_size -= n_zeros;
		}
		
		if ( its_size == 1 )
		{
			const int_t x = *its_pointer;
			
			ASSERT( x != 0 );
			
			destroy_extent();
			
			its_size = 0;
			its_integer = x;
		}
	}
	
	
	cmp_t abs_compare( const ibox& a, const ibox& b )
	{
		using math::integer::compare;
		
		return compare( iota::is_little_endian(), a.data(), a.size(),
		                                          b.data(), b.size() );
	}
	
	cmp_t compare( const ibox& a, const ibox& b )
	{
		if ( cmp_t sign_cmp = a.sign() - b.sign() )
		{
			return sign_cmp;
		}
		
		if ( a.sign() == 0 )
		{
			return 0;
		}
		
		cmp_t abs_cmp = abs_compare( a, b );
		
		if ( a.sign() < 0 )
		{
			abs_cmp = -abs_cmp;
		}
		
		return abs_cmp;
	}
	
	
	static inline
	bool sum_is_one_limb( ibox::size_t  a_size,
	                      ibox::size_t  b_size,
	                      limb_t        a,
	                      limb_t        b )
	{
		using math::integer::will_carry;
		
		// These are the raw size fields, not size().
		
		return a_size == 0  &&  b_size == 0  &&  ! will_carry( a, b );
	}
	
	static inline
	limb_t one_limb_product( ibox::size_t  a_size,
	                         ibox::size_t  b_size,
	                         limb_t        a,
	                         limb_t        b )
	{
		/*
			Return the product a * b if it fits within a single limb.
			Return zero otherwise.
			
			Since the result is ambiguous when either operand is zero,
			callers must check for zero operands in advance and not call
			this function in that case.
		*/
		
		// These are the raw size fields, not size().
		
		if ( a_size != 0  ||  b_size != 0 )
		{
			return 0;
		}
		
		using math::integer::long_t;
		using math::integer::long_multiply;
		
		if ( sizeof (long_t) > sizeof (limb_t) )
		{
			const long_t product = long_multiply( a, b );
			
			return (limb_t) product == product ? product : 0;
		}
		
		/*
			In the absence of a means to detect multiplication overflow,
			we examine the width of the operands and guess conservatively.
			If this function returns nonzero, then the product of the operands
			will fit in a single limb.  The converse doesn't hold, however.
			
			zenith:  The maximum int_t value (unsigned).
			factor:  The maximum squarable factor.
			
			E.g. For 64-bit systems, zenith is 0xFFFFFFFFFFFFFFFF and factor
			is 0xFFFFFFFF.  (32-bit systems are handled above.)
		*/
		
		const ibox::int_t zenith = ibox::int_t( -1 );
		const ibox::int_t factor = zenith >> (sizeof zenith * 8 / 2);
		
		return (a <= factor  &&  b <= factor) ? a * b : 0;
	}
	
	
	void ibox::add( const ibox& y )
	{
		const bool single = sum_is_one_limb( its_size,    y.its_size,
		                                     its_integer, y.its_integer );
		
		if ( single )
		{
			its_integer += y.its_integer;
			
			return;
		}
		
		using math::integer::sum_size;
		using math::integer::add;
		
		ibox tmp = y;  // in case this == &y
		
		limb_t const* x_data = data();
		size_t        x_size = size();
		limb_t const* y_data = tmp.data();
		size_t        y_size = tmp.size();
		
		ASSERT( x_size != 0 );
		ASSERT( y_size != 0 );
		
		const size_t r_size = sum_size( iota::is_little_endian(),
		                                x_data, x_size,
		                                y_data, y_size );
		
		ASSERT( r_size > 1 );
		
		if ( r_size > x_size )
		{
			extend( r_size );
			
			x_size = r_size;
		}
		else
		{
			unshare();
		}
		
		add( iota::is_little_endian(), its_pointer, x_size, y_data, y_size );
	}
	
	void ibox::subtract_lesser( const ibox& y )
	{
		ASSERT( abs_compare( y, *this ) < 0 );
		
		if ( ! has_extent() )
		{
			its_integer -= y.its_integer;
			
			return;
		}
		
		using math::integer::subtract;
		
		ibox tmp = y;  // in case this == &y
		
		unshare();
		
		limb_t*       x_data = its_pointer;
		size_t        x_size = size();
		limb_t const* y_data = tmp.data();
		size_t        y_size = tmp.size();
		
		subtract( iota::is_little_endian(), x_data, x_size, y_data, y_size );
		
		shrink_to_fit();
	}
	
	void ibox::subtract( const ibox& y )
	{
		const cmp_t rel = abs_compare( *this, y );
		
		if ( rel == 0 )
		{
			destroy();
			construct( 0ul );
			
			return;
		}
		
		if ( rel > 0 )
		{
			subtract_lesser( y );
		}
		else
		{
			ibox result = y;
			
			result.subtract_lesser( *this );
			
			swap( result );
		}
	}
	
	void ibox::multiply_by( const ibox& y )
	{
		ASSERT(   its_sign != 0 );
		ASSERT( y.its_sign != 0 );
		
		const limb_t product = one_limb_product( its_size,    y.its_size,
		                                         its_integer, y.its_integer );
		
		if ( product != 0 )
		{
			its_integer = product;
		}
		else
		{
			using math::integer::multiply;
			
			ibox tmp = y;  // in case this == &y
			
			size_t x_size = size();
			size_t y_size = y.size();
			
			x_size += y_size;
			
			extend( x_size );
			
			limb_t*       x_data = its_pointer;
			limb_t const* y_data = tmp.data();
			
			multiply( iota::is_little_endian(),
			          x_data, x_size,
			          y_data, y_size );
			
			shrink_to_fit();
		}
		
		its_sign *= y.its_sign;
	}
	
	void ibox::halve()
	{
		if ( has_extent() )
		{
			using math::integer::shift_right;
			
			unshare();
			
			shift_right( iota::is_little_endian(), its_pointer, size() );
			
			shrink_to_fit();
		}
		else
		{
			its_integer /= 2;
			
			if ( its_integer == 0 )
			{
				its_sign = 0;
			}
		}
		
	}
	
}