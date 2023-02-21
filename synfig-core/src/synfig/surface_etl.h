/* === S Y N F I G ========================================================= */
/*! \file surface_etl.h
** \brief surface Template Class
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2008 Chris Moore
**
** This file is part of Synfig.
**
** Synfig is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Synfig is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
** \endlegal
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_SURFACE_ETL_H
#define SYNFIG_SURFACE_ETL_H

/* === H E A D E R S ======================================================= */

#include <algorithm>
#include <cstring>

#include <synfig/misc.h>
#include <synfig/pen.h>

/* === M A C R O S ========================================================= */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace clamping
{
	/**
	 * Clamp @a x to a natural number between [0, bound)
	 * @return false if @a bound is invalid (less or equal to zero)
	 */
	inline static bool clamp(int &x, int bound) {
		if (bound <= 0) return false;
		if (x < 0) x = 0; else
			if (x >= bound) x = bound - 1;
		return true;
	}
};


template <typename T, typename AT>
class value_prep
{
public:
	typedef T value_type;
	typedef AT accumulator_type;

	accumulator_type cook(const value_type& x)const { return (accumulator_type)x; }
	value_type uncook(const accumulator_type& x)const { return (value_type)x; }
};

/**
 * Allow to get an interpolated sample from image from real coordinates instead of exact pixels
 */
template <typename VT, typename CT, VT reader(const void*, int, int)>
class sampler
{
public:
	typedef VT value_type;
	typedef CT coord_type;
	typedef coord_type float_type;
	typedef value_type func(const void*, const coord_type, const coord_type);

	inline static void prepare_coord(const coord_type x, int &u, float_type &a) {
		u=static_cast<int>(std::floor(x));
		a=float_type(x)-float_type(u);
	}

	inline static void prepare_coords(const coord_type x, const coord_type y, int &u, int &v, float_type &a, float_type &b)
	{
		prepare_coord(x, u, a);
		prepare_coord(y, v, b);
	}

	inline static void fill_cubic_polinomial(float_type x, float_type tx[])
	{
		tx[0] = float_type(0.5)*x*(x*(float_type(-1)*x + float_type(2)) - float_type(1));	// -t + 2t^2 -t^3
		tx[1] = float_type(0.5)*(x*(x*(float_type(3)*x - float_type(5))) + float_type(2)); 	// 2 - 5t^2 + 3t^3
		tx[2] = float_type(0.5)*x*(x*(float_type(-3)*x + float_type(4)) + float_type(1));	// t + 4t^2 - 3t^3
		tx[3] = float_type(0.5)*x*x*(x-float_type(1));						                // -t^2 + t^3
	}

	/** Retrieve the nearest sample */
	static value_type nearest_sample(const void *surface, const coord_type x, const coord_type y)
		{ return (value_type)reader(surface, synfig::round_to_int(x), synfig::round_to_int(y)); }

	/** Linear interpolation */
	static value_type linear_sample(const void *surface, const coord_type x, const coord_type y)
	{
		int u, v; float_type a, b;
		prepare_coords(x, y, u, v, a, b);

		const float_type c(float_type(1)-a), d(float_type(1)-b);

		return (value_type)(reader(surface, u  ,v  ))*c*d
			 + (value_type)(reader(surface, u+1,v  ))*a*d
			 + (value_type)(reader(surface, u  ,v+1))*c*b
			 + (value_type)(reader(surface, u+1,v+1))*a*b;
	}

	/** Cosine sample */
	static value_type cosine_sample(const void *surface, const coord_type x, const coord_type y)
	{
		int u, v; float_type a, b;
		prepare_coords(x, y, u, v, a, b);

		a=(float_type(1) - cos(a*float_type(3.1415927)))*float_type(0.5);
		b=(float_type(1) - cos(b*float_type(3.1415927)))*float_type(0.5);

		const float_type c(float_type(1)-a), d(float_type(1)-b);

		return (value_type)(reader(surface, u  ,v  ))*c*d
			 + (value_type)(reader(surface, u+1,v  ))*a*d
			 + (value_type)(reader(surface, u  ,v+1))*c*b
			 + (value_type)(reader(surface, u+1,v+1))*a*b;
	}

	/** Cubic sample: uses Catmull-Rom spline */
	static value_type cubic_sample(const void *surface, const coord_type x, const coord_type y)
	{
		//Using catmull rom interpolation because it doesn't blur at all
		//bezier curve with intermediate ctrl pts: 0.5/3(p(i+1) - p(i-1)) and similar

		//precalculate indices (all clamped) and offset
		const int xi = (int)floor(x);
		const int yi = (int)floor(y);
		int xa[] = { xi-1, xi, xi+1, xi+2 };
		int ya[] = { yi-1, yi, yi+1, yi+2 };

		// offset
		const float_type xf = float_type(x)-float_type(xi);
		const float_type yf = float_type(y)-float_type(yi);

		float_type txf[4], tyf[4];
		fill_cubic_polinomial(xf, txf);
		fill_cubic_polinomial(yf, tyf);

		#define f(i,j)  (value_type)(reader(surface, i, j))
		#define ff(i,j) f(xa[i], ya[j])*txf[i]
		#define fff(j)  (ff(0,j) + ff(1,j) + ff(2,j) + ff(3,j))*tyf[j]

		return fff(0) + fff(1) + fff(2) + fff(3);

		#undef fff
		#undef ff
		#undef f
	}
};

/**
 * A 2D image data.
 *
 * You can think it as a 2D matrix too.
 *
 * Its size is defined on construction, but it can be changed later (set_wh()).
 *
 * Data can be accessed as a 2D matrix (integer coordinates via operator []).
 * They also can be get by samplers, to get pixel values in real number coordinates,
 * by using some interpolation methods. @see nearest_sample(), linear_sample() and others.
 * Another way is by using iterators (begin(), end()) or pen (get_pen())
 *
 * Pixels can be changed directly by their coordinates, or via iterators or pens.
 * There are also the painting methods like fill() and blit_to().
 */
template <typename T, class VP=value_prep<T,T> >
class surface
{
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef generic_pen<value_type> pen;
	typedef generic_pen<const value_type> const_pen;
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
	/** a contiguous memory space.
	 *  If it is deletable, it is expected to be allocated by `new value_type[]` allocator
	 */
	value_type *data_;
	/** the byte length of a row, possibly including some padding for byte-alignment.
	 *  It must be a positive number
	*/
	typename difference_type::value_type pitch_;
	/** surface width (number of pixels/samples per row) */
	int w_;
	/** surface height (number of pixels/samples per column or number of rows) */
	int h_;
	/** if the data is dynamically allocated and can be deleted (on destructor too) */
	bool deletable_;

	value_prep_type cooker_;

public:
	surface():
		data_(nullptr),
		pitch_(0),
		w_(0),h_(0),
		deletable_(false) { }

	surface(value_type* data, int w, int h, bool deletable=false):
		data_(data),
		pitch_(sizeof(value_type)*w),
		w_(w),h_(h),
		deletable_(deletable) { }

	surface(value_type* data, int w, int h, typename difference_type::value_type pitch, bool deletable=false):
		data_(data),
		pitch_(pitch),
		w_(w),h_(h),
		deletable_(deletable) { }
	
	surface(const typename size_type::value_type &w, const typename size_type::value_type &h):
		data_(new value_type[w*h]),
		pitch_(sizeof(value_type)*w),
		w_(w),h_(h),
		deletable_(true) { }

	surface(const size_type &s):
		data_(new value_type[s.x*s.y]),
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
		pitch_=sizeof(value_type)*w_;
		deletable_=true;

		for(int y = 0; y < h_; y++)
			for(int x = 0; x < w_; x++)
				(*this)[y][x]=_begin.get_value_at(x,y);
	}

	surface(const surface &s):
		data_(s.data_?(pointer)(new char[s.pitch_*s.h_]):0),
		pitch_(s.pitch_),
		w_(s.w_),
		h_(s.h_),
		deletable_(s.data_?true:false)
	{
		assert(&s);
		if(s.data_)
		{
			assert(data_);
			memcpy(data_, s.data_, pitch_ * h_);
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

	const surface &operator=(const surface &rhs)
	{
		set_wh(rhs.w_,rhs.h_);
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

	/** Change the surface size. It doesn't keep the previous pixel/sample values */
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
		data_=(pointer)(new char[pitch_*h_]);
		deletable_=true;
	}


	void
	fill(value_type v, int x, int y, int w, int h)
	{
		assert(data_);
		if(w<=0 || h<=0)return;
		pen PEN(get_pen(x,y));
		PEN.set_value(v);
		for(int i = 0; i < h; i++, PEN.inc_y(), PEN.dec_x(w))
			PEN.put_hline(w);
	}

	template <class _pen> void
	fill(value_type v, _pen& PEN, int w, int h)
	{
		assert(data_);
		if(w<=0 || h<=0)return;
		PEN.set_value(v);
		for(int y = 0; y < h; y++, PEN.inc_y(), PEN.dec_x(w))
			PEN.put_hline(w);
	}

	void
	fill(value_type v)
	{
		assert(data_);
		pen pen_=begin();
		pen_.set_value(v);
		for(int y = 0; y < h_; y++, pen_.inc_y(), pen_.dec_x(w_))
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
			for(int i = 0; i < w; i++, DEST_PEN.inc_x(), SOURCE_PEN.inc_x())
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
			memset(static_cast<void*>(data_), 0, h_*pitch_);
		else
			fill(value_type());
	}

	iterator_x
	operator[](const int &y)
	{ assert(data_); return (pointer)(((char*)data_)+y*pitch_); }

	const_iterator_x
	operator[](const int &y)const
	{ assert(data_); return (const_pointer)(((const char*)data_)+y*pitch_); }


	bool is_valid()const
	{
		return data_ != nullptr
			&&	w_>0
			&&	h_>0
			&&	pitch_ > 0
		;
	}

	operator bool()const { return is_valid(); }

	pen begin() { assert(data_); return pen(data_,w_,h_,pitch_); }
	pen get_pen(int x, int y) { assert(data_); return begin().move(x,y); }
	pen end() { assert(data_); return get_pen(w_,h_); }

	const_pen begin()const { assert(data_); return const_pen(data_,w_,h_,pitch_); }
	const_pen get_pen(int x, int y)const { assert(data_); return begin().move(x,y); }
	const_pen end()const { assert(data_); return get_pen(w_,h_); }

	inline static value_type reader(const void *surf, int x, int y) {
		const surface &s = *(const surface*)surf;
		return clamping::clamp(x, s.get_w()) && clamping::clamp(y, s.get_h()) ? s[y][x] : value_type();
	}

	inline static value_type reader_cook(const void *surf, int x, int y) {
		const surface &s = *(const surface*)surf;
		return clamping::clamp(x, s.get_w()) && clamping::clamp(y, s.get_h()) ? s.cooker_.cook(s[y][x]) : value_type();
	}

	template<value_type reader(const void*, int, int)>
	class sampler: public synfig::sampler<value_type, float, reader> { };

	typedef sampler<surface::reader_cook> sampler_cook;
	typedef sampler<surface::reader> sampler_nocook;

	//! Nearest sample
	value_type nearest_sample(const float x, const float y)const
		{ return cooker_.uncook(sampler_cook::nearest_sample(this, x, y)); }

	//! Nearest sample for already "cooked" surfaces
	value_type nearest_sample_cooked(const float x, const float y)const
		{ return (value_type)(sampler_nocook::nearest_sample(this, x, y)); }

	//! Linear sample
	value_type linear_sample(const float x, const float y)const
		{ return cooker_.uncook(sampler_cook::linear_sample(this, x, y)); }

	//! Linear sample for already "cooked" surfaces
	value_type linear_sample_cooked(const float x, const float y)const
		{ return (value_type)(sampler_nocook::linear_sample(this, x, y)); }

	//! Cosine sample
	value_type cosine_sample(const float x, const float y)const
		{ return cooker_.uncook(sampler_cook::cosine_sample(this, x, y)); }

	//! Cosine sample for already "cooked" surfaces
	value_type cosine_sample_cooked(const float x, const float y)const
		{ return (value_type)(sampler_nocook::cosine_sample(this, x, y)); }

	//! Cubic sample
	value_type cubic_sample(float x, float y)const
		{ return cooker_.uncook(sampler_cook::cubic_sample(this, x, y)); }

	//! Cubic sample for already "cooked" surfaces
	value_type cubic_sample_cooked(float x, float y)const
		{ return (value_type)(sampler_nocook::cubic_sample(this, x, y)); }
};

};

/* === T Y P E D E F S ===================================================== */


/* === E N D =============================================================== */

#endif // SYNFIG_SURFACE_ETL_H
