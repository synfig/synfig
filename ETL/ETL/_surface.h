/*! ========================================================================
** Extended Template and Library
** Surface Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
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

#ifndef __ETL__SURFACE_H
#define __ETL__SURFACE_H

/* === H E A D E R S ======================================================= */

#include "_pen.h"
#include "_misc.h"
#include <algorithm>
#include <cstring>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

template <typename T, typename AT>
class value_prep
{
public:
	typedef T value_type;
	typedef AT accumulator_type;

	accumulator_type cook(const value_type& x)const { return (accumulator_type)x; }
	value_type uncook(const accumulator_type& x)const { return (value_type)x; }
};

template <typename T, typename AT=T, class VP=value_prep<T,AT> >
class surface
{
public:
	typedef T value_type;
	typedef AT accumulator_type;
	typedef value_type* pointer;
	typedef accumulator_type* accumulator_pointer;
	typedef const value_type* const_pointer;
	typedef const accumulator_type* const_accumulator_pointer;
	typedef value_type& reference;
	typedef generic_pen<value_type,accumulator_type> pen;
	typedef generic_pen<const value_type,accumulator_type> const_pen;
	typedef VP value_prep_type;

	typedef alpha_pen<const_pen> const_alpha_pen;
	typedef alpha_pen<pen> non_const_alpha_pen;

	typedef typename pen::difference_type size_type;
	typedef typename pen::difference_type difference_type;

	typedef typename pen::iterator_x iterator_x;
	typedef typename pen::iterator_y iterator_y;
	typedef typename pen::const_iterator_x const_iterator_x;
	typedef typename pen::const_iterator_y const_iterator_y;

private:
	value_type *data_;
	value_type *zero_pos_;
	typename difference_type::value_type pitch_;
	int w_, h_;
	bool deletable_;

	value_prep_type cooker_;

	void swap(const surface &x)
	{
		std::swap(data_,x.data_);
		std::swap(zero_pos_,x.zero_pos_);
		std::swap(pitch_,x.pitch_);
		std::swap(w_,x.w_);
		std::swap(h_,x.h_);
		std::swap(deletable_,x.deletable_);
	}

public:
	surface():
		data_(0),
		zero_pos_(data_),
		pitch_(0),
		w_(0),h_(0),
		deletable_(false) { }

	surface(value_type* data, int w, int h, bool deletable=false):
		data_(data),
		zero_pos_(data),
		pitch_(sizeof(value_type)*w),
		w_(w),h_(h),
		deletable_(deletable) { }

	surface(value_type* data, int w, int h, typename difference_type::value_type pitch, bool deletable=false):
		data_(data),
		zero_pos_(data),
		pitch_(pitch),
		w_(w),h_(h),
		deletable_(deletable) { }
	
	surface(const typename size_type::value_type &w, const typename size_type::value_type &h):
		data_(new value_type[w*h]),
		zero_pos_(data_),
		pitch_(sizeof(value_type)*w),
		w_(w),h_(h),
		deletable_(true) { }

	surface(const size_type &s):
		data_(new value_type[s.x*s.y]),
		zero_pos_(data_),
		pitch_(sizeof(value_type)*s.x),
		w_(s.x),h_(s.y),
		deletable_(true) { }

	template <typename _pen>
	surface(const _pen &_begin, const _pen &_end)
	{
		typename _pen::difference_type size=_end-_begin;

		data_=new value_type[size.x*size.y];
		w_=size.x;
		h_=size.y;
		zero_pos_=data_;
		pitch_=sizeof(value_type)*w_;
		deletable_=true;

		int x,y;

		for(y=0;y<h_;y++)
			for(x=0;x<w_;x++)
				(*this)[y][x]=_begin.get_value_at(x,y);
	}

	surface(const surface &s):
		data_(s.data_?(pointer)(new char[s.pitch_*s.h_]):0),
		zero_pos_(data_+(s.zero_pos_-s.data_)),
		pitch_(s.pitch_),
		w_(s.w_),
		h_(s.h_),
		deletable_(s.data_?true:false)
	{
		assert(&s);
		if(s.data_)
		{
			assert(data_);
			memcpy(data_,s.data_,abs(pitch_)*h_);
		}
	}

public:
	~surface()
	{
		if(deletable_)
			delete [] data_;
	}

	size_type
	size()const
	{ return size_type(w_,h_); }

	typename size_type::value_type get_pitch()const { return pitch_; }
	typename size_type::value_type get_w()const { return w_; }
	typename size_type::value_type get_h()const { return h_; }

	const surface &mirror(const surface &rhs)
	{
		if(deletable_)delete [] data_;

		data_=rhs.data_;
		zero_pos_=rhs.zero_pos_;
		pitch_=rhs.pitch_;
		w_=rhs.w_;
		h_=rhs.h_;
		deletable_=false;

		return *this;
	}

	const surface &operator=(const surface &rhs)
	{
		set_wh(rhs.w_,rhs.h_);
		zero_pos_=data_+(rhs.zero_pos_-rhs.data_);
		pitch_=rhs.pitch_;
		deletable_=true;

		memcpy(data_,rhs.data_,pitch_*h_);

		return *this;
	}

	void
	copy(const surface &rhs)
	{
		if(pitch_!=rhs.pitch_ || w_!=rhs.w_ || h_!=rhs.h_)
			return;
		memcpy(data_, rhs.data_, pitch_*h_);
	}
	
	void
	set_wh(typename size_type::value_type w, typename size_type::value_type h, const typename size_type::value_type &pitch=0)
	{
		if(data_)
		{
			if(w==w_ && h==h_ && deletable_)
				return;
			if(deletable_)
				delete [] data_;
		}

		w_=w;
		h_=h;
		if(pitch)
			pitch_=pitch;
		else
			pitch_=sizeof(value_type)*w_;
		zero_pos_=data_=(pointer)(new char[pitch_*h_]);
		deletable_=true;
	}

	void
	set_wh(typename size_type::value_type w, typename size_type::value_type h, unsigned char* newdata, const typename size_type::value_type &pitch)
	{
		if(data_ && deletable_)
		{
			delete [] data_;
		}
		w_=w;
		h_=h;
		zero_pos_=data_=(pointer)newdata;
		pitch_=pitch;
		deletable_=false;	
	}

	void
	fill(value_type v, int x, int y, int w, int h)
	{
		assert(data_);
		if(w<=0 || h<=0)return;
		int i;
		pen PEN(get_pen(x,y));
		PEN.set_value(v);
		for(i=0;i<h;i++,PEN.inc_y(),PEN.dec_x(w))
			PEN.put_hline(w);
	}

	template <class _pen> void
	fill(value_type v, _pen& PEN, int w, int h)
	{
		assert(data_);
		if(w<=0 || h<=0)return;
		int y;
		PEN.set_value(v);
		for(y=0;y<h;y++,PEN.inc_y(),PEN.dec_x(w))
			PEN.put_hline(w);
	}

	void
	fill(value_type v)
	{
		assert(data_);
		int y;
		pen pen_=begin();
		pen_.set_value(v);
		for(y=0;y<h_;y++,pen_.inc_y(),pen_.dec_x(w_))
			pen_.put_hline(w_);
	}

	template <class _pen> void blit_to(_pen &pen)
	{ return blit_to(pen,0,0, get_w(),get_h()); }

	template <class _pen> void
	blit_to(_pen &DEST_PEN,
			int x, int y, int w, int h) //src param
	{
		if(x>=w_ || y>=h_)
			return;

		//clip source origin
		if(x<0)
		{
			w+=x;	//decrease
			x=0;
		}

		if(y<0)
		{
			h+=y;	//decrease
			y=0;
		}

		//clip width against dest width
		w = std::min((long)w,(long)(DEST_PEN.end_x()-DEST_PEN.x()));
		h = std::min((long)h,(long)(DEST_PEN.end_y()-DEST_PEN.y()));

		//clip width against src width
		w = std::min(w,w_-x);
		h = std::min(h,h_-y);

		if(w<=0 || h<=0)
			return;

		pen SOURCE_PEN(get_pen(x,y));

		for(; h>0; h--,DEST_PEN.inc_y(),SOURCE_PEN.inc_y())
		{
			int i;
			for(i=0; i<w; i++,DEST_PEN.inc_x(),SOURCE_PEN.inc_x())
			{
				DEST_PEN.put_value(SOURCE_PEN.get_value());
			}
			DEST_PEN.dec_x(w);
			SOURCE_PEN.dec_x(w);
		}
	}

	void
	clear()
	{
		assert(data_);
		if(pitch_==(signed int)sizeof(value_type)*w_)
			memset(data_,0,h_*pitch_);
		else
			fill(value_type());
	}

	iterator_x
	operator[](const int &y)
	{ assert(data_); return (pointer)(((char*)zero_pos_)+y*pitch_); }

	const_iterator_x
	operator[](const int &y)const
	{ assert(data_); return (const_pointer)(((const char*)zero_pos_)+y*pitch_); }

	void
	flip_v()
	{
		assert(data_);

		zero_pos_=(pointer)(((char*)zero_pos_)+pitch_*h_);

		pitch_=-pitch_;
	}

	bool is_valid()const
	{
		return 	data_!=0
			&&	zero_pos_!=0
			&&	w_>0
			&&	h_>0
			&&	pitch_!=0
		;
	}

	operator bool()const { return is_valid(); }

	pen begin() { assert(data_); return pen(data_,w_,h_,pitch_); }
	pen get_pen(int x, int y) { assert(data_); return begin().move(x,y); }
	pen end() { assert(data_); return get_pen(w_,h_); }

	const_pen begin()const { assert(data_); return const_pen(data_,w_,h_,pitch_); }
	const_pen get_pen(int x, int y)const { assert(data_); return begin().move(x,y); }
	const_pen end()const { assert(data_); return get_pen(w_,h_); }

	//! Linear sample
	value_type linear_sample(const float x, const float y)const
	{
		int u(floor_to_int(x)), v(floor_to_int(y));
		float a, b;
		static const float epsilon(1.0e-6);

		if(x<0.0f)u=0,a=0.0f;
		else if(x>w_-1)u=w_-1,a=0.0f;
		else a=x-u;

		if(y<0.0f)v=0,b=0.0f;
		else if(y>h_-1)v=h_-1,b=0.0f;
		else b=y-v;

		const float
			c(1.0f-a), d(1.0f-b),
			e(a*d),f(c*b),g(a*b);

		accumulator_type ret(cooker_.cook((*this)[v][u])*(c*d));
		if(e>=epsilon)ret+=cooker_.cook((*this)[v][u+1])*e;
		if(f>=epsilon)ret+=cooker_.cook((*this)[v+1][u])*f;
		if(g>=epsilon)ret+=cooker_.cook((*this)[v+1][u+1])*g;
		return cooker_.uncook(ret);
	}

	//! Linear sample for already "cooked" surfaces
	value_type linear_sample_cooked(const float x, const float y)const
	{
#define h(j,i)	(((accumulator_type)((*this)[j][i])))
		int u(floor_to_int(x)), v(floor_to_int(y));
		float a, b;
		static const float epsilon(1.0e-6);
		
		if(x<0.0f)u=0,a=0.0f;
		else if(x>w_-1)u=w_-1,a=0.0f;
		else a=x-u;
		
		if(y<0.0f)v=0,b=0.0f;
		else if(y>h_-1)v=h_-1,b=0.0f;
		else b=y-v;
		
		const float
		c(1.0f-a), d(1.0f-b),
		e(a*d),f(c*b),g(a*b);
		
		accumulator_type ret(h(v,u)*(c*d));
		if(e>=epsilon)ret+=h(v,u+1)*e;
		if(f>=epsilon)ret+=h(v+1,u)*f;
		if(g>=epsilon)ret+=h(v+1,u+1)*g;

		return (value_type)(ret);
#undef h
	}

	//! Cosine sample
	value_type cosine_sample(const float x, const float y)const
	{
		int u(floor_to_int(x)), v(floor_to_int(y));
		float a, b;
		static const float epsilon(1.0e-6);

		if(x<0.0f)u=0,a=0.0f;
		else if(x>w_-1)u=w_-1,a=0.0f;
		else a=x-u;

		if(y<0.0f)v=0,b=0.0f;
		else if(y>h_-1)v=h_-1,b=0.0f;
		else b=y-v;

		a=(1.0f-cos(a*3.1415927f))*0.5f;
		b=(1.0f-cos(b*3.1415927f))*0.5f;

		const float
			c(1.0f-a), d(1.0f-b),
			e(a*d),f(c*b),g(a*b);

		accumulator_type ret(cooker_.cook((*this)[v][u])*(c*d));
		if(e>=epsilon)ret+=cooker_.cook((*this)[v][u+1])*e;
		if(f>=epsilon)ret+=cooker_.cook((*this)[v+1][u])*f;
		if(g>=epsilon)ret+=cooker_.cook((*this)[v+1][u+1])*g;

		return cooker_.uncook(ret);
	}

	//! Cosine sample for already "cooked" surfaces
	value_type cosine_sample_cooked(const float x, const float y)const
	{
#define h(j,i)	(((accumulator_type)((*this)[j][i])))

		int u(floor_to_int(x)), v(floor_to_int(y));
		float a, b;
		static const float epsilon(1.0e-6);
		
		if(x<0.0f)u=0,a=0.0f;
		else if(x>w_-1)u=w_-1,a=0.0f;
		else a=x-u;
		
		if(y<0.0f)v=0,b=0.0f;
		else if(y>h_-1)v=h_-1,b=0.0f;
		else b=y-v;
		
		a=(1.0f-cos(a*3.1415927f))*0.5f;
		b=(1.0f-cos(b*3.1415927f))*0.5f;
		
		const float
		c(1.0f-a), d(1.0f-b),
		e(a*d),f(c*b),g(a*b);
		
		accumulator_type ret(h(v,u)*(c*d));
		if(e>=epsilon)ret+=h(v,u+1)*e;
		if(f>=epsilon)ret+=h(v+1,u)*f;
		if(g>=epsilon)ret+=h(v+1,u+1)*g;
		
		return (value_type)(ret);
#undef h
	}

	//! Cubic sample
	value_type cubic_sample(float x, float y)const
	{
		#if 0
	#define P(x)	(((x)>=0)?((x)*(x)*(x)):0.0f)
	#define R(x)	( P(x+2) - 4.0f*P(x+1) + 6.0f*P(x) - 4.0f*P(x-1) )*(1.0f/6.0f)
	#define F(i,j)	(cooker_.cook((*this)[max(min(j+v,h_-1),0)][max(min(i+u,w_-1),0)])*(R((i)-a)*R(b-(j))))
	#define Z(i,j) ret+=F(i,j)
	#define X(i,j)	// placeholder... To make box more symmetric

		int u(floor_to_int(x)), v(floor_to_int(y));
		float a, b;

		// Clamp X
		if(x<0.0f)u=0,a=0.0f;
		else if(u>w_-1)u=w_-1,a=0.0f;
		else a=x-u;

		// Clamp Y
		if(y<0.0f)v=0,b=0.0f;
		else if(v>h_-1)v=h_-1,b=0.0f;
		else b=y-v;

		// Interpolate
		accumulator_type ret(F(0,0));
		Z(-1,-1); Z(-1, 0); Z(-1, 1); Z(-1, 2);
		Z( 0,-1); X( 0, 0); Z( 0, 1); Z( 0, 2);
		Z( 1,-1); Z( 1, 0); Z( 1, 1); Z( 1, 2);
		Z( 2,-1); Z( 2, 0); Z( 2, 1); Z( 2, 2);

		return cooker_.uncook(ret);

	#undef X
	#undef Z
	#undef F
	#undef P
	#undef R
		#else

		#define f(j,i)	(cooker_.cook((*this)[j][i]))
		//Using catmull rom interpolation because it doesn't blur at all
		//bezier curve with intermediate ctrl pts: 0.5/3(p(i+1) - p(i-1)) and similar
		accumulator_type xfa [4];

		//precalculate indices (all clamped) and offset
		const int xi = (int)floor(x);
		const int yi = (int)floor(y);
		int xa[] = { xi-1, xi, xi+1, xi+2 };
		int ya[] = { yi-1, yi, yi+1, yi+2 };

		// clamp all
		const int w = w_ - 1;
		const int h = h_ - 1;
		xa[0] < 0 && ((xa[1] < 0 && ((xa[2] < 0 && ((xa[3] < 0 && (xa[3] = 0)), (xa[2] = 0))), (xa[1] = 0))), (xa[0] = 0));
		ya[0] < 0 && ((ya[1] < 0 && ((ya[2] < 0 && ((ya[3] < 0 && (ya[3] = 0)), (ya[2] = 0))), (ya[1] = 0))), (ya[0] = 0));
		xa[3] > w && ((xa[2] > w && ((xa[1] > w && ((xa[0] > w && (xa[0] = w)), (xa[1] = w))), (xa[2] = w))), (xa[3] = w));
		ya[3] > h && ((ya[2] > h && ((ya[1] > h && ((ya[0] > h && (ya[0] = h)), (ya[1] = h))), (ya[2] = h))), (ya[3] = h));

		// offset
		const float xf = x-xi;
		const float yf = y-yi;

		//figure polynomials for each point
		const float txf[] =
		{
			0.5f*xf*(xf*(xf*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
			0.5f*(xf*(xf*(3.f*xf - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
			0.5f*xf*(xf*(-3.f*xf + 4.f) + 1.f),		//t + 4t^2 - 3t^3
			0.5f*xf*xf*(xf-1.f)						//-t^2 + t^3
		};

		const float tyf[] =
		{
			0.5f*yf*(yf*(yf*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
			0.5f*(yf*(yf*(3.f*yf - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
			0.5f*yf*(yf*(-3.f*yf + 4.f) + 1.f),		//t + 4t^2 - 3t^3
			0.5f*yf*yf*(yf-1.f)						//-t^2 + t^3
		};

		//evaluate polynomial for each row
		for(int i = 0; i < 4; ++i)
		{
			xfa[i] = f(ya[i],xa[0])*txf[0] + f(ya[i],xa[1])*txf[1] + f(ya[i],xa[2])*txf[2] + f(ya[i],xa[3])*txf[3];
		}

		//return the cumulative column evaluation
		return cooker_.uncook(xfa[0]*tyf[0] + xfa[1]*tyf[1] + xfa[2]*tyf[2] + xfa[3]*tyf[3]);
#undef f
#endif
	}

	//! Cubic sample for already "cooked" surfaces
	value_type cubic_sample_cooked(float x, float y)const
	{
#define f(j,i)	(((accumulator_type)((*this)[j][i])))
		//Using catmull rom interpolation because it doesn't blur at all
		//bezier curve with intermediate ctrl pts: 0.5/3(p(i+1) - p(i-1)) and similar
		accumulator_type xfa [4];
		
		//precalculate indices (all clamped) and offset
		const int xi = (int)floor(x);
		const int yi = (int)floor(y);
		int xa[] = { xi-1, xi, xi+1, xi+2 };
		int ya[] = { yi-1, yi, yi+1, yi+2 };

		// clamp all
		const int w = w_ - 1;
		const int h = h_ - 1;
		xa[0] < 0 && ((xa[1] < 0 && ((xa[2] < 0 && ((xa[3] < 0 && (xa[3] = 0)), (xa[2] = 0))), (xa[1] = 0))), (xa[0] = 0));
		ya[0] < 0 && ((ya[1] < 0 && ((ya[2] < 0 && ((ya[3] < 0 && (ya[3] = 0)), (ya[2] = 0))), (ya[1] = 0))), (ya[0] = 0));
		xa[3] > w && ((xa[2] > w && ((xa[1] > w && ((xa[0] > w && (xa[0] = w)), (xa[1] = w))), (xa[2] = w))), (xa[3] = w));
		ya[3] > h && ((ya[2] > h && ((ya[1] > h && ((ya[0] > h && (ya[0] = h)), (ya[1] = h))), (ya[2] = h))), (ya[3] = h));

		// offset
		const float xf = x-xi;
		const float yf = y-yi;
		
		//figure polynomials for each point
		const float txf[] =
		{
			0.5f*xf*(xf*(xf*(-1) + 2) - 1),	//-t + 2t^2 -t^3
			0.5f*(xf*(xf*(3*xf - 5)) + 2), 	//2 - 5t^2 + 3t^3
			0.5f*xf*(xf*(-3*xf + 4) + 1),	//t + 4t^2 - 3t^3
			0.5f*xf*xf*(xf-1)				//-t^2 + t^3
		};
		
		const float tyf[] =
		{
			0.5f*yf*(yf*(yf*(-1.f) + 2.f) - 1.f),	//-t + 2t^2 -t^3
			0.5f*(yf*(yf*(3.f*yf - 5.f)) + 2.f), 	//2 - 5t^2 + 3t^3
			0.5f*yf*(yf*(-3.f*yf + 4.f) + 1.f),		//t + 4t^2 - 3t^3
			0.5f*yf*yf*(yf-1.f)						//-t^2 + t^3
		};
		
		//evaluate polynomial for each row
		for(int i = 0; i < 4; ++i)
		{
			xfa[i] = f(ya[i],xa[0])*txf[0] + f(ya[i],xa[1])*txf[1] + f(ya[i],xa[2])*txf[2] + f(ya[i],xa[3])*txf[3];
		}
		
		//return the cumulative column evaluation
		return (value_type)(xfa[0]*tyf[0] + xfa[1]*tyf[1] + xfa[2]*tyf[2] + xfa[3]*tyf[3]);
#undef f
	}

	//! Rectangle sample
	value_type	sample_rect(float x0,float y0,float x1,float y1) const
	{
		const surface &s = *this;

		//assumes it's clamped to the boundary of the image
		//force min max relationship for x0,x1 and y0,y1
		if(x0 > x1) std::swap(x0,x1);
		if(y0 > y1) std::swap(y0,y1);

		//local variable madness
		//all things that want to inter-operate should provide a default value constructor for = 0
		accumulator_type acum = 0;
		int xi=0,yi=0;

		int	xib=(int)floor(x0),
			xie=(int)floor(x1);

		int	yib=(int)floor(y0),
			yie=(int)floor(y1);

		//the weight for the pixel should remain the same...
		float weight = (y1-y0)*(x1-x0);
		assert(weight != 0);

		float ylast = y0, xlastb = x0;
		const_pen	pen_ = s.get_pen(xib,yib);

		for(yi = yib; yi < yie; ylast = ++yi, pen_.inc_y())
		{
			const float yweight = yi+1 - ylast;

			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi, pen_.inc_x())
			{
				const float w = yweight*(xi+1 - xlast);
				acum += cooker_.cook(pen_.get_value())*w;
			}

			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += cooker_.cook(pen_.get_value())*w;

			pen_.dec_x(xie-xib);
		}

		//post in y direction... must have all x...
		{
			const float yweight = y1 - ylast;

			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi)
			{
				const float w = yweight*(xi+1 - xlast);

				acum += cooker_.cook(pen_.get_value())*w;
			}

			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += cooker_.cook(pen_.get_value())*w;
		}

		acum *= 1/weight;
		return cooker_.uncook(acum);
	}

	//! Rectangle sample for already "cooked" surfaces
	value_type	sample_rect_cooked(float x0,float y0,float x1,float y1) const
	{
		const surface &s = *this;
		
		//assumes it's clamped to the boundary of the image
		//force min max relationship for x0,x1 and y0,y1
		if(x0 > x1) std::swap(x0,x1);
		if(y0 > y1) std::swap(y0,y1);
		
		//local variable madness
		//all things that want to inter-operate should provide a default value constructor for = 0
		accumulator_type acum = 0;
		int xi=0,yi=0;
		
		int	xib=(int)floor(x0),
		xie=(int)floor(x1);
		
		int	yib=(int)floor(y0),
		yie=(int)floor(y1);
		
		//the weight for the pixel should remain the same...
		float weight = (y1-y0)*(x1-x0);
		assert(weight != 0);
		
		float ylast = y0, xlastb = x0;
		const_pen	pen_ = s.get_pen(xib,yib);
		
		for(yi = yib; yi < yie; ylast = ++yi, pen_.inc_y())
		{
			const float yweight = yi+1 - ylast;
			
			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi, pen_.inc_x())
			{
				const float w = yweight*(xi+1 - xlast);
				acum += pen_.get_value()*w;
			}
			
			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += pen_.get_value()*w;
			
			pen_.dec_x(xie-xib);
		}
		
		//post in y direction... must have all x...
		{
			const float yweight = y1 - ylast;
			
			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi)
			{
				const float w = yweight*(xi+1 - xlast);
				
				acum += pen_.get_value()*w;
			}
			
			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += pen_.get_value()*w;
		}
		
		acum *= 1/weight;
		return (value_type)(acum);
	}

	//! Rectangle sample clipped
	value_type	sample_rect_clip(float x0,float y0,float x1,float y1) const
	{
		const surface &s = *this;

		//assumes it's clamped to the boundary of the image
		//force min max relationship for x0,x1 and y0,y1
		if(x0 > x1) std::swap(x0,x1);
		if(y0 > y1) std::swap(y0,y1);

		//local variable madness
		//all things that want to inter-operate should provide a default value constructor for = 0
		accumulator_type acum = 0;
		int xi=0,yi=0;

		int	xib=(int)floor(x0),
			xie=(int)floor(x1);

		int	yib=(int)floor(y0),
			yie=(int)floor(y1);

		//the weight for the pixel should remain the same...
		float weight = (y1-y0)*(x1-x0);

		assert(weight != 0);

		//clip to the input region
		if(x0 >= s.get_w() || x1 <= 0) return acum;
		if(y0 >= s.get_h() || y1 <= 0) return acum;

		if(x0 < 0) { x0 = 0; xib = 0; }
		if(x1 >= s.get_w())
		{
			x1 = s.get_w(); //want to be just below the last pixel...
			xie = s.get_w()-1;
		}

		if(y0 < 0) { y0 = 0; yib = 0; }
		if(y1 >= s.get_h())
		{
			y1 = s.get_h(); //want to be just below the last pixel...
			yie = s.get_h()-1;
		}

		float ylast = y0, xlastb = x0;
		const_pen	pen = s.get_pen(xib,yib);

		for(yi = yib; yi < yie; ylast = ++yi, pen.inc_y())
		{
			const float yweight = yi+1 - ylast;

			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi, pen.inc_x())
			{
				const float w = yweight*(xi+1 - xlast);
				acum += cooker_.cook(pen.get_value())*w;
			}

			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += cooker_.cook(pen.get_value())*w;

			pen.dec_x(xie-xib);
		}

		//post in y direction... must have all x...
		{
			const float yweight = y1 - ylast;

			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi)
			{
				const float w = yweight*(xi+1 - xlast);

				acum += cooker_.cook(pen.get_value())*w;
			}

			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += cooker_.cook(pen.get_value())*w;
		}

		acum *= 1/weight;
		return cooker_.uncook(acum);
	}

	//! Rectangle sample clipped for already "cooked" surfaces
	value_type	sample_rect_clip_cooked(float x0,float y0,float x1,float y1) const
	{
		const surface &s = *this;
		
		//assumes it's clamped to the boundary of the image
		//force min max relationship for x0,x1 and y0,y1
		if(x0 > x1) std::swap(x0,x1);
		if(y0 > y1) std::swap(y0,y1);
		
		//local variable madness
		//all things that want to inter-operate should provide a default value constructor for = 0
		accumulator_type acum = 0;
		int xi=0,yi=0;
		
		int	xib=(int)floor(x0),
		xie=(int)floor(x1);
		
		int	yib=(int)floor(y0),
		yie=(int)floor(y1);
		
		//the weight for the pixel should remain the same...
		float weight = (y1-y0)*(x1-x0);
		
		assert(weight != 0);
		
		//clip to the input region
		if(x0 >= s.get_w() || x1 <= 0) return acum;
		if(y0 >= s.get_h() || y1 <= 0) return acum;
		
		if(x0 < 0) { x0 = 0; xib = 0; }
		if(x1 >= s.get_w())
		{
			x1 = s.get_w(); //want to be just below the last pixel...
			xie = s.get_w()-1;
		}
		
		if(y0 < 0) { y0 = 0; yib = 0; }
		if(y1 >= s.get_h())
		{
			y1 = s.get_h(); //want to be just below the last pixel...
			yie = s.get_h()-1;
		}
		
		float ylast = y0, xlastb = x0;
		const_pen	pen = s.get_pen(xib,yib);
		
		for(yi = yib; yi < yie; ylast = ++yi, pen.inc_y())
		{
			const float yweight = yi+1 - ylast;
			
			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi, pen.inc_x())
			{
				const float w = yweight*(xi+1 - xlast);
				acum += pen.get_value()*w;
			}
			
			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += pen.get_value()*w;
			
			pen.dec_x(xie-xib);
		}
		
		//post in y direction... must have all x...
		{
			const float yweight = y1 - ylast;
			
			float xlast = xlastb;
			for(xi = xib; xi < xie; xlast = ++xi)
			{
				const float w = yweight*(xi+1 - xlast);
				
				acum += pen.get_value()*w;
			}
			
			//post... with next being fractional...
			const float w = yweight*(x1 - xlast);
			acum += pen.get_value()*w;
		}
		
		acum *= 1/weight;
		return (value_type)(acum);
	}
};

_ETL_END_NAMESPACE

/* === T Y P E D E F S ===================================================== */


/* === E N D =============================================================== */

#endif
