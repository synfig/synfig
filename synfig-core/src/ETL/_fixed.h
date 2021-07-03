/*! ========================================================================
** Extended Template and Library
** Fixed-Point Math Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__FIXED_H
#define __ETL__FIXED_H

/* === H E A D E R S ======================================================= */

#include <cmath>

/* === M A C R O S ========================================================= */

// the "+0.5" code was commented out - maybe to make thing run faster?
// it can be re-enabled by uncommenting this next line:
// #define ROUND_TO_NEAREST_INTEGER

#ifndef ETL_FIXED_TYPE
# define ETL_FIXED_TYPE	int
#endif

#ifndef ETL_FIXED_BITS
#define ETL_FIXED_BITS	12
#endif

#ifndef ETL_FIXED_EPSILON
#define ETL_FIXED_EPSILON		_EPSILON()
#endif

#ifdef __GNUC___
#define ETL_ATTRIB_CONST	__attribute__ ((const))
#define ETL_ATTRIB_PURE		__attribute__ ((pure))
#define ETL_ATTRIB_INLINE	__attribute__ ((always_inline))
#else
#define ETL_ATTRIB_CONST
#define ETL_ATTRIB_PURE
#define ETL_ATTRIB_INLINE
#endif

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

// Forward declarations
template<typename T, unsigned int FIXED_BITS> class fixed_base;
//template<> class fixed_base<char>;

};

namespace std {
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> abs(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> cos(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> cosh(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> exp(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> log(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> log10(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> pow(const etl::fixed_base<T,FIXED_BITS>&, int);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> pow(const etl::fixed_base<T,FIXED_BITS>&, const T&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> pow(const etl::fixed_base<T,FIXED_BITS>&,
					const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> pow(const etl::fixed_base<T,FIXED_BITS>&, const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> sin(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> sinh(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> sqrt(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> tan(const etl::fixed_base<T,FIXED_BITS>&);
template<typename T, unsigned int FIXED_BITS> etl::fixed_base<T,FIXED_BITS> tanh(const etl::fixed_base<T,FIXED_BITS>&);
};
namespace etl {

/*! ========================================================================
** \class	fixed_base
** \brief	Fixed-point template base class
**
** A more detailed description needs to be written.
*/
template <class T,unsigned int FIXED_BITS>
class fixed_base
{
public:
	typedef T value_type;
private:
	T _data;

	typedef fixed_base<T,FIXED_BITS> _fixed;
	typedef fixed_base<T,FIXED_BITS> self_type;

	inline static bool _TYPE_SMALLER_THAN_INT() ETL_ATTRIB_CONST ETL_ATTRIB_INLINE;
	inline static bool _USING_ALL_BITS() ETL_ATTRIB_CONST ETL_ATTRIB_INLINE;
	inline static value_type _ONE() ETL_ATTRIB_CONST ETL_ATTRIB_INLINE;
	inline static value_type _F_MASK() ETL_ATTRIB_CONST ETL_ATTRIB_INLINE;
	inline static float _EPSILON() ETL_ATTRIB_CONST ETL_ATTRIB_INLINE;

	class raw { };
public:
	fixed_base()ETL_ATTRIB_INLINE;
	fixed_base(const float &f)ETL_ATTRIB_INLINE;
	fixed_base(const double &f)ETL_ATTRIB_INLINE;
	fixed_base(const long double &f)ETL_ATTRIB_INLINE;
	fixed_base(const int &i)ETL_ATTRIB_INLINE;
	fixed_base(const int &n,const int &d)ETL_ATTRIB_INLINE; //!< Fraction constructor
	fixed_base(const _fixed &x)ETL_ATTRIB_INLINE;
	fixed_base(value_type x,raw)ETL_ATTRIB_INLINE;

	T &data() ETL_ATTRIB_PURE ETL_ATTRIB_INLINE;
	const T &data()const ETL_ATTRIB_PURE ETL_ATTRIB_INLINE;

	const _fixed& operator+=(const _fixed &rhs) ETL_ATTRIB_INLINE;
	const _fixed& operator-=(const _fixed &rhs) ETL_ATTRIB_INLINE;
	template<typename U> const _fixed& operator*=(const U &rhs) ETL_ATTRIB_INLINE;
	template<typename U> const _fixed& operator/=(const U &rhs) ETL_ATTRIB_INLINE;
	const _fixed& operator*=(const _fixed &rhs) ETL_ATTRIB_INLINE;
	const _fixed& operator/=(const _fixed &rhs) ETL_ATTRIB_INLINE;
	const _fixed& operator*=(const int &rhs) ETL_ATTRIB_INLINE;
	const _fixed& operator/=(const int &rhs) ETL_ATTRIB_INLINE;


	template<typename U> _fixed operator+(const U &rhs)const ETL_ATTRIB_INLINE;
	template<typename U> _fixed operator-(const U &rhs)const ETL_ATTRIB_INLINE;
	template<typename U> _fixed operator*(const U &rhs)const ETL_ATTRIB_INLINE;
	template<typename U> _fixed operator/(const U &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator+(const _fixed &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator-(const _fixed &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator*(const _fixed &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator/(const _fixed &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator*(const int &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator/(const int &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator*(const float &rhs)const ETL_ATTRIB_INLINE;
	_fixed operator*(const double &rhs)const ETL_ATTRIB_INLINE;

	// Negation Operator
	_fixed operator-()const ETL_ATTRIB_INLINE;

	// Casting Operators
	inline operator float()const ETL_ATTRIB_INLINE;
	inline operator double()const ETL_ATTRIB_INLINE;
	inline operator long double()const ETL_ATTRIB_INLINE;
	inline operator int()const ETL_ATTRIB_INLINE;
	inline operator bool()const ETL_ATTRIB_INLINE;

	_fixed floor()const;
	_fixed ceil()const;
	_fixed round()const;

	bool operator==(const _fixed &rhs)const { return data()==rhs.data(); }
	bool operator!=(const _fixed &rhs)const { return data()!=rhs.data(); }
	bool operator<(const _fixed &rhs)const { return data()<rhs.data(); }
	bool operator>(const _fixed &rhs)const { return data()>rhs.data(); }
	bool operator<=(const _fixed &rhs)const { return data()<=rhs.data(); }
	bool operator>=(const _fixed &rhs)const { return data()>=rhs.data(); }
};


template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base()
{}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const _fixed &x):_data(x._data)
{}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const float &f):_data(static_cast<value_type>(f*_ONE()
#ifdef ROUND_TO_NEAREST_INTEGER
																				   +0.5f
#endif
									)) {}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const double &f):_data(static_cast<value_type>(f*_ONE()
#ifdef ROUND_TO_NEAREST_INTEGER
																					+0.5
#endif
									)) {}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const long double &f):_data(static_cast<value_type>(f*_ONE()
#ifdef ROUND_TO_NEAREST_INTEGER
																						 +0.5
#endif
									)) {}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const int &i):_data(i<<FIXED_BITS)
{}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(value_type x,raw):_data(x) { }

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::fixed_base(const int &n,const int &d):_data((n<<FIXED_BITS)/d) { }



template <class T,unsigned int FIXED_BITS> inline bool
fixed_base<T,FIXED_BITS>::_TYPE_SMALLER_THAN_INT()
{
	return sizeof(T)<sizeof(int);
}

template <class T,unsigned int FIXED_BITS> inline bool
fixed_base<T,FIXED_BITS>::_USING_ALL_BITS()
{
	return sizeof(T)*8==FIXED_BITS;
}

template <class T,unsigned int FIXED_BITS> inline T
fixed_base<T,FIXED_BITS>::_ONE()
{
	return static_cast<T>((_USING_ALL_BITS()?~T(0):1<<FIXED_BITS));
}

template <class T,unsigned int FIXED_BITS> inline T
fixed_base<T,FIXED_BITS>::_F_MASK()
{
	return static_cast<T>(_USING_ALL_BITS()?~T(0):_ONE()-1);
}

template <class T,unsigned int FIXED_BITS> inline float
fixed_base<T,FIXED_BITS>::_EPSILON()
{
	return 1.0f/((float)_ONE()*2);
}


template <class T,unsigned int FIXED_BITS>T &
fixed_base<T,FIXED_BITS>::data()
{
	return _data;
}

template <class T,unsigned int FIXED_BITS>const T &
fixed_base<T,FIXED_BITS>::data()const
{
	return _data;
}

//! fixed+=fixed
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator+=(const _fixed &rhs)
{
	_data+=rhs._data;
	return *this;
}

//! fixed-=fixed
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator-=(const _fixed &rhs)
{
	_data-=rhs._data;
	return *this;
}

//! fixed*=fixed
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator*=(const _fixed &rhs)
{
	if(_TYPE_SMALLER_THAN_INT())
		_data=static_cast<T>((int)_data*(int)rhs._data>>FIXED_BITS);
	else
	{
		_data*=rhs._data;
		_data>>=FIXED_BITS;
	}

	return *this;
}

//! fixed/=fixed
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator/=(const _fixed &rhs)
{
	if(_TYPE_SMALLER_THAN_INT())
		_data=static_cast<T>((int)_data/(int)rhs._data<<FIXED_BITS);
	else
	{
		_data/=rhs._data;
		_data<<=FIXED_BITS;
	}
	return *this;
}

template <class T,unsigned int FIXED_BITS> template<typename U> const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator*=(const U &rhs)
{
	return operator*=(fixed_base<T,FIXED_BITS>(rhs));
}

template <class T,unsigned int FIXED_BITS> template<typename U> const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator/=(const U &rhs)
{
	return operator/=(fixed_base<T,FIXED_BITS>(rhs));
}

//! fixed*=int
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator*=(const int &rhs)
{
	_data*=rhs; return *this;
}

//! fixed/=int
template <class T,unsigned int FIXED_BITS>const fixed_base<T,FIXED_BITS> &
fixed_base<T,FIXED_BITS>::operator/=(const int &rhs)
{
	_data/=rhs; return *this;
}







//! fixed + fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator+(const _fixed &rhs)const
{
	_fixed ret;
	ret._data=_data+rhs._data;
	return ret;
}

//! fixed - fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator-(const _fixed &rhs)const
{
	_fixed ret;
	ret._data=_data-rhs._data;
	return ret;
}

//! fixed * fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator*(const _fixed &rhs)const
{
	_fixed ret;
	ret._data=((_data*rhs._data)>>FIXED_BITS);
	return ret;
	//return reinterpret_cast<_fixed>((_data*rhs._data)>>FIXED_BITS);
}

//! fixed / fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator/(const _fixed &rhs)const
{
	_fixed ret;
	ret._data=((_data/rhs._data)<<FIXED_BITS);
	return ret;
	//return reinterpret_cast<_fixed>((_data/rhs._data)<<FIXED_BITS);
}

//! fixed + ...
template <class T,unsigned int FIXED_BITS> template<typename U> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator+(const U &rhs) const
{
	return operator+(fixed_base<T,FIXED_BITS>(rhs));
}

//! fixed - ...
template <class T,unsigned int FIXED_BITS> template<typename U> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator-(const U &rhs) const
{
	return operator-(fixed_base<T,FIXED_BITS>(rhs));
}

//! fixed * ...
template <class T,unsigned int FIXED_BITS> template<typename U> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator*(const U &rhs) const
{
	return operator*(fixed_base<T,FIXED_BITS>(rhs));
}

//! fixed / ...
template <class T,unsigned int FIXED_BITS> template<typename U> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator/(const U &rhs) const
{
	return operator/(fixed_base<T,FIXED_BITS>(rhs));
}

//! fixed * int
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator*(const int &rhs)const
{
	_fixed ret;
	ret._data=_data*rhs;
	return ret;
	//return reinterpret_cast<_fixed>(_data*rhs);
}

//! fixed * float
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator*(const float &rhs)const
{
    return (*this)*_fixed(rhs);
}

//! fixed * double
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator*(const double &rhs)const
{
    return (*this)*_fixed(rhs);
}


//! fixed / int
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator/(const int &rhs)const
{
	_fixed ret;
	ret._data=_data/rhs;
	return ret;
	//return reinterpret_cast<_fixed>(_data/rhs);
}

//! float * fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
operator*(const float& lhs, const fixed_base<T,FIXED_BITS> &rhs)
{
    return rhs*lhs;
}

//! double * fixed
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
operator*(const double& lhs, const fixed_base<T,FIXED_BITS> &rhs)
{
    return rhs*lhs;
}






// Negation Operator
template <class T,unsigned int FIXED_BITS>fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator-()const
{
	_fixed ret; ret._data=-_data; return ret;
}

// Casting Operators
template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator float()const
{
	return static_cast<float>(_data)/static_cast<float>(_ONE());
}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator double()const
{
	return static_cast<double>(_data)/static_cast<double>(_ONE());
}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator long double()const
{
	return static_cast<long double>(_data)/static_cast<long double>(_ONE());
}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator int()const
{
	return static_cast<int>(_data>>FIXED_BITS);
}

template <class T,unsigned int FIXED_BITS>
fixed_base<T,FIXED_BITS>::operator bool()const
{
	return static_cast<bool>(_data);
}


template <class T,unsigned int FIXED_BITS> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::floor()const
{
	_fixed ret(*this);
	ret._data&=~_F_MASK();
	return ret;
}

template <class T,unsigned int FIXED_BITS> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::ceil()const
{
	_fixed ret(*this);
	if(ret._data&_F_MASK())
		ret._data=(ret._data&~_F_MASK()) + _ONE();
	else
		ret._data&=~_F_MASK();
	return ret;
}

template <class T,unsigned int FIXED_BITS> fixed_base<T,FIXED_BITS>
fixed_base<T,FIXED_BITS>::round()const
{
	_fixed ret(*this);
	ret._data+=_ONE()>>1;
	ret._data&=~_F_MASK();
	return ret;
}


























typedef fixed_base<ETL_FIXED_TYPE,ETL_FIXED_BITS> fixed;

};

namespace std {

template <class T,unsigned int FIXED_BITS>
inline etl::fixed_base<T,FIXED_BITS>
ceil(const etl::fixed_base<T,FIXED_BITS> &rhs)
{ return rhs.ceil(); }

template <class T,unsigned int FIXED_BITS>
etl::fixed_base<T,FIXED_BITS>
floor(const etl::fixed_base<T,FIXED_BITS> &rhs)
{ return rhs.floor(); }

template <class T,unsigned int FIXED_BITS>
etl::fixed_base<T,FIXED_BITS>
round(const etl::fixed_base<T,FIXED_BITS> &rhs)
{ return rhs.round(); }

template <class T,unsigned int FIXED_BITS>
etl::fixed_base<T,FIXED_BITS>
abs(const etl::fixed_base<T,FIXED_BITS> &rhs)
{ return rhs<etl::fixed_base<T,FIXED_BITS>(0)?-rhs:rhs; }

};

/*
template <class T,unsigned int FIXED_BITS, typename U> bool
operator==(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()==rhs.data(); }

template <class T,unsigned int FIXED_BITS, typename U> bool
operator!=(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()!=rhs.data(); }

template <class T,unsigned int FIXED_BITS, typename U> bool
operator>(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()>rhs.data(); }

template <class T,unsigned int FIXED_BITS, typename U> bool
operator<(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()<rhs.data(); }

template <class T,unsigned int FIXED_BITS, typename U> bool
operator>=(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()>=rhs.data(); }

template <class T,unsigned int FIXED_BITS, typename U> bool
operator<=(const etl::fixed_base<T,FIXED_BITS>& lhs, const etl::fixed_base<T,FIXED_BITS>& rhs)
{ return lhs.data()<=rhs.data(); }
*/


#if defined(__GNUC__) && __GNUC__ == 3
template <class T,unsigned int FIXED_BITS, typename U> U
operator*(const U &a,const etl::fixed_base<T,FIXED_BITS> &b)
	{ return a*static_cast<double>(b); }

template <class T,unsigned int FIXED_BITS, typename U> U
operator/(const U &a,const etl::fixed_base<T,FIXED_BITS> &b)
	{ return a/static_cast<double>(b); }

template <class T,unsigned int FIXED_BITS, typename U> U
operator+(const U &a,const etl::fixed_base<T,FIXED_BITS> &b)
	{ return a+static_cast<double>(b); }

template <class T,unsigned int FIXED_BITS, typename U> U
operator-(const U &a,const etl::fixed_base<T,FIXED_BITS> &b)
	{ return a-static_cast<double>(b); }


/*
inline const float &
operator*=(float &a,const etl::fixed &b)
	{ a*=(float)b; return a; }

inline const float &
operator/=(float &a,const etl::fixed &b)
	{ a/=(float)b; return a; }

inline const float &
operator-=(float &a,const etl::fixed &b)
	{ a-=(float)b; return a; }

inline const float &
operator+=(float &a,const etl::fixed &b)
	{ a+=(float)b; return a; }
*/
#endif

/* === E N D =============================================================== */

#endif
