/*! ========================================================================
** \file pen.h
** \brief Pen Template Class Implementation
**
** \legal
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef SYNFIG_PEN_H
#define SYNFIG_PEN_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include <cassert>
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/**
 * A 2D surface row iterator: access surface samples of a single row.
 *
 * You can get or set a sample value with operator[] or the current sample by derefencing the iterator.
 *
 * Check if iterator is valid by casting it to bool.
 *
 * Go to next row via inc() or increment operators. Analog for previous row.
 * Please note that the iterator does not check if you are passing the surface boundaries
 * (before first row or beyond the last one).
 */
template<typename T>
class generic_pen_row_iterator
{
public:
	typedef T value_type;
	typedef int difference_type;
	typedef value_type* pointer;
	typedef value_type& reference;

	typedef generic_pen_row_iterator<value_type> self_type;

private:
	pointer data_ = nullptr;
	int pitch_ = 0;

public:
	reference operator[](int i)const { assert(data_); return *(pointer)( (char*)data_+pitch_*i ); }
	reference operator*()const { assert(data_); return *data_; }
	pointer operator->() const { assert(data_); return data_; }

	/** Go to next surface row */
	void inc() { assert(data_); data_ = (pointer)((char*)data_ + pitch_); }
	/** Skip @a n surface rows */
	void inc(int n) { assert(data_); data_ = (pointer)((char*)data_ + n*pitch_); }

	/** Go back to previous surface row */
	void dec() { assert(data_); data_ = (pointer)((char*)data_ - pitch_); }
	/** Go back @a n surface rows */
	void dec(int n) { assert(data_); data_ = (pointer)((char*)data_ - n*pitch_); }

	/** Go to next surface row */
	const self_type &operator++() { assert(data_); inc(); return *this; }
	/** Go back to previous surface row */
	const self_type &operator--() { assert(data_); dec(); return *this; }

	/** Go to next surface row */
	self_type operator++(int)
		{ assert(data_); self_type ret(*this); inc(); return ret; }
	/** Go back to previous surface row */
	self_type operator--(int)
		{ assert(data_); self_type ret(*this); dec(); return ret; }

	bool operator==(const self_type &rhs)const
		{ return data_==rhs.data_; }

	bool operator!=(const self_type &rhs)const
		{ return data_!=rhs.data_; }

	difference_type operator-(const self_type &rhs)const
		{ assert(data_); return ((char*)data_-(char*)rhs.data_-1)/pitch_+1; }

	self_type operator+(const difference_type &rhs)const
	{
		assert(data_);
		self_type ret(*this);
		ret.inc(rhs);
		return ret;
	}

	self_type operator-(const difference_type &rhs)const
	{
		assert(data_);
		self_type ret(*this);
		ret.dec(rhs);
		return ret;
	}

	operator const generic_pen_row_iterator<const value_type>()const
	{
		return generic_pen_row_iterator<const value_type>(data_,pitch_);
	}

	/** Check if iterator is valid. DOES NOT check if it off the 2D surface */
	explicit operator bool()const { return (bool)data_; }
	/** Check if iterator is invalid. DOES NOT check if it off the 2D surface */
	bool operator!()const { return !data_; }

	generic_pen_row_iterator(pointer data, int pitch):data_(data), pitch_(pitch) { }
	generic_pen_row_iterator():data_(nullptr), pitch_(0) { }
};

/**
 * A 2D cursor to walk/read/write on a 2D surface.
 *
 * You can change the surface by writing:
 * - a 2D block: put_block()
 * - a horizontal line: put_hline()
 * - a single sample: put_value()
 *
 * You can set a default value to write via set_value() and retrieve it back with get_pen_value().
 *
 * Get sample iterators with operator[], x(), y() and similar methods or a specific sample with get_value() method.
 *
 * The pen can move around the surface with inc(), dec(), move(), move_to() and some other methods.
 *
 * You should write/read samples inside valid area. Look for the _clip() methods.
 */
template<typename T>
class generic_pen
{
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	typedef pointer iterator_x;
	typedef const_pointer const_iterator_x;

	typedef generic_pen_row_iterator<value_type> iterator_y;
	typedef generic_pen_row_iterator<const value_type> const_iterator_y;

	struct difference_type
	{
		typedef int value_type;
		value_type x, y;
		difference_type(value_type x, value_type y) : x(x), y(y) { }
	};

protected:
	int x_ = 0, y_ = 0;
	int w_ = 0, h_ = 0;
private:
	/** how many bytes have a surface row, including possible padding */
	int pitch_ = 0;
	/** the default sample value to write on the surface */
	value_type value_ {};
	/** current pointer to surface data */
	value_type* data_ = nullptr;

	typedef generic_pen<T> self_type;

	/** convenient method to advance @c data_ some @a nbytes */
	void addptr(int nbytes)
	{
		data_ = (pointer)((char*)data_ + nbytes);
	}

	/** conveniet method to return @c data_ some @a nbytes */
	void subptr(int nbytes)
	{
		data_ = (pointer)((char*)data_ - nbytes);
	}

public:

	/**
	 * A generic_pen
	 * @param data the sample vector
	 * @param w width: the number of samples per surface row
	 * @param h height: the number of samples per surface column
	 * @param pitch number of BYTES in a surface row
	 */
	generic_pen(value_type *data, int w, int h, int pitch):
		x_(0),
		y_(0),
		w_(w),
		h_(h),
		pitch_(pitch),
		value_{},
		data_(data)
	{
	}

	/**
	 * A generic_pen
	 * @param data the sample vector
	 * @param w width: the number of samples per surface row
	 * @param h height: the number of samples per surface column
	 */
	generic_pen(value_type *data, int w, int h):
		x_(0),
		y_(0),
		w_(w),
		h_(h),
		pitch_(sizeof(value_type)*w),
		value_{},
		data_(data)
	{
	}

	generic_pen(): value_{}, data_(nullptr) { }

	/**
	 * Move this pen by a relative distance (@a a, @a b)
	 * @param a how many samples to move in horizontal axis
	 * @param b how many samples to move in vertical axis
	 * @return this object
	 */
	self_type& move(int a, int b)
	{
		assert(data_);
		x_ += a, y_ += b;
		addptr(b*pitch_ + a*sizeof(value_type));
		return *this;
	}
	/**
	 * Move this pen to the absolute position (@a x, @a y)
	 * @param x how many samples to move in horizontal axis from the start of the row (0-indexed)
	 * @param y how many samples to move in vertical axis from the start of the column (0-indexed)
	 * @return this object
	 */
	self_type& move_to(int x, int y) { assert(data_); return move(x - x_,y - y_);}

	/**
	 * Move this pen to the same position of pen @a p
	 * @param p the reference pen to get the position
	 * @return this object
	 */
	template<typename TT>
	self_type& move_to(const generic_pen<TT> &p) { assert(data_ && p.data_); return move_to(p.x_,p.y_);}

	/**
	 * The default sample value to write on surface
	 * @param v the value
	 */
	void set_value(const value_type &v) { value_=v; }

	/** Move this pen to the next horizontal sample */
	void inc_x() { assert(data_); x_++; data_++; }
	/** Move this pen to the previous horizontal sample */
	void dec_x() { assert(data_); x_--; data_--; }
	/** Move this pen to the next vertical sample */
	void inc_y() { assert(data_); y_++; addptr(pitch_); }
	/** Move this pen to the previous vertical sample */
	void dec_y() { assert(data_); y_--; subptr(pitch_); }

	/** Move this pen @a n samples along the horizontal axis */
	void inc_x(int n) { assert(data_); x_+=n; data_+=n; }
	/** Move this pen @a n samples along the horizontal axis before current position */
	void dec_x(int n) { assert(data_); x_-=n; data_-=n; }
	/** Move this pen @a n samples along the vertical axis after current position */
	void inc_y(int n) { assert(data_); y_+=n; data_ = (pointer)((char*)data_ + pitch_*n); }
	/** Move this pen @a n samples along the vertical axis before current position */
	void dec_y(int n) { assert(data_); y_-=n; data_ = (pointer)((char*)data_ - pitch_*n); }

	/** Replace the sample at current position with @a v */
	void put_value(const value_type &v)const { assert(data_); *data_=v; }
	/** Replace the sample at current position with pen value set by set_value() */
	void put_value()const { assert(data_); put_value(value_); }

	/** Replace the sample at current position with @a v if inside valid area */
	void put_value_clip(const value_type &v)const
		{ if(!clipped()) put_value(v); }
	/** Replace the sample at current position with pen value set by set_value() if inside valid area */
	void put_value_clip()const { put_value_clip(value_); }

	/** Get the current sample value */
	const_reference get_value()const { assert(data_); return *data_; }

	/** Get the sample value at (@a x, @a y) coordinates from current point */
	const_reference get_value_at(int x, int y)const { assert(data_); return ((pointer)(((char*)data_)+y*pitch_))[x]; }

	/** Get the sample value at (@a x, @a y) coordinates from current point if it is in a valid region */
	const_reference get_value_clip_at(int x, int y)const { assert(data_); if(clipped(x,y))return value_type(); return ((pointer)(((char*)data_)+y*pitch_))[x]; }

	/** Get the current sample value if it is in a valid region */
	const value_type get_value_clip()const { assert(data_); if(clipped())return value_type(); return *data_; }

	/** Get the default sample value set by set_value() */
	const value_type get_pen_value()const { return value_; }

	/**
	 *  Write @a l sample values in a row. It moves the pen.
	 * @param l the number of samples to write
	 * @param v the sample value to be written
	 */
	void put_hline(int l,const value_type &v)
	{for(;l>0;l--,inc_x())put_value(v);}

	/**
	 *  Write @a l sample values in a row, with the pen default value. It moves the pen.
	 * @param l the number of samples to write
	 */
	void put_hline(int l) {put_hline(l,value_);}

	/**
	 *  Write at most @a l sample values in a row, as long as they are in the valid region. It moves the pen.
	 * @param l the number of samples to write
	 * @param v the sample value to be written
	 */
	void put_hline_clip(int l, const value_type &v)
	{l=std::min(l,w_-x_);for(;l>0;l--,inc_x())put_value_clip(v);}

	/**
	 *  Write at most @a l samples in a row, as long as they are in the valid region.
	 *  It moves the pen.
	 *  It uses the default sample value set by set_value().
	 * @param l the number of samples to write
	 */
	void put_hline_clip(int l) {put_hline_clip(l,value_);}

	/**
	 * Write the same sample value @a v in a 2D-rectangle from current position
	 * It moves the pen.
	 * @param h the rectangle height
	 * @param w the rectangle width
	 * @param v the sample value to be used
	 */
	void put_block(int h, int w, const value_type &v)
	{
		self_type row(*this);
		for(;h>0;h--,row.inc_y())
		{
			self_type col(row);
			col.put_hline(w,v);
		}
	}

	/**
	 * Write the pen value @a v in a 2D-rectangle from current position
	 * It moves the pen.
	 * @param h the rectangle height
	 * @param w the rectangle width
	 */
	void put_block(int h, int w) { put_block(h,w,value_); }

	/**
	 * Write the same sample value @a v in a 2D-rectangle from current position,
	 * where the rectangle is in the valid area.
	 * It moves the pen.
	 * @param h the rectangle height
	 * @param w the rectangle width
	 * @param v the sample value to be used
	 */
	void put_block_clip(int h, int w, const value_type &v)
	{
		self_type row(*this);

		//clip start position
		if(row.x_ < 0) { w+=row.x_; row.inc_x(-row.x_); }
		if(row.y_ < 0) { h+=row.y_; row.inc_y(-row.y_);	}

		//clip width and height of copy rect
		h = std::min(h,h_-y_);
		w = std::min(w,w_-x_);

		//copy rect
		for(;h>0;h--,row.inc_y())
		{
			self_type col(row);
			col.put_hline(w,v);	//already clipped
		}
	}

	/**
	 * Write the pen value @a v in a 2D-rectangle from current position
	 * where the rectangle is in the valid area.
	 * It moves the pen.
	 * @param h the rectangle height
	 * @param w the rectangle width
	 */
	void put_block_clip(int h, int w) { put_block_clip(h,w,value_); }


	iterator_x operator[](int i)const { assert(data_); return (pointer)(((char*)data_)+i*pitch_); }

	iterator_x x() { assert(data_); return data_; }
	iterator_x begin_x() { assert(data_); return data_-x_; }
	iterator_x end_x() { assert(data_); return data_-x_+w_; }

	iterator_y y() { assert(data_); return iterator_y(data_,pitch_); }
	iterator_y begin_y() { assert(data_); return iterator_y((pointer)((char*)data_ - y_*pitch_),pitch_); }
	iterator_y end_y() { assert(data_); return iterator_y((pointer)((char*)data_ + (h_-y_)*pitch_),pitch_); }

	explicit operator bool()const { return (bool)data_; }
	bool operator!()const { return !data_; }
	bool operator==(const self_type &rhs)const { return data_==rhs.data_; }
	bool operator!=(const self_type &rhs)const { return data_!=rhs.data_; }
	bool clipped(int x, int y)const { return !(x_+x>=0 && y_+y>=0 && x_+x<w_ && y_+y<h_); }
	bool clipped()const { return !(x_>=0 && y_>=0 && x_<w_ && y_<h_); }

	difference_type operator-(const self_type &rhs)const
	{
		assert(data_);
		assert(pitch_==rhs.pitch_);
		int ptr_diff=(char*)data_-(char*)rhs.data_-1;
		return difference_type(ptr_diff%pitch_/sizeof(value_type)+1,ptr_diff/pitch_);
	}

	self_type operator+(const difference_type &rhs)const
	{
		assert(data_);
		self_type ret(*this);
		ret.move(rhs.x,rhs.y);
		return ret;
	}

	int get_width()const {return w_;}
	int get_height()const {return h_;}
	int get_pitch()const {return pitch_;}
};

/**
 * The alpha_pen class is a variation of generic_pen, with a transparency effect.
 *
 * You can set an transparency fade when writing with this pen by setting the
 * default alpha value with set_alpha(), and/or another additional alpha effect
 * available on some put_xxxx() methods.
 */
template <
	typename PEN_,
	typename A_=float,
	class AFFINE_=affine_combo<typename PEN_::value_type,A_>
>
class alpha_pen : public PEN_
{
public:
	typedef A_ alpha_type;
	typedef AFFINE_ affine_func_type;

	typedef typename PEN_::value_type value_type;
	typedef alpha_pen self_type;

private:
	alpha_type alpha_;

protected:
	affine_func_type affine_func_;

public:
	using PEN_::get_value;
	using PEN_::get_pen_value;
	using PEN_::inc_x;
	using PEN_::dec_x;
	using PEN_::inc_y;
	using PEN_::dec_y;
	using PEN_::clipped;
	using PEN_::w_;
	using PEN_::h_;
	using PEN_::x_;
	using PEN_::y_;

	alpha_pen(const alpha_type &a = 1, const affine_func_type &func = affine_func_type()):alpha_(a),affine_func_(func) { }
	alpha_pen(const PEN_ &x, const alpha_type &a=1, const affine_func_type &func=affine_func_type())
		:PEN_(x),alpha_(a),affine_func_(func) { }

	const alpha_type& get_alpha()const { return alpha_; }
	void set_alpha(alpha_type a) { alpha_=a; }

	void put_value(const value_type &v, const alpha_type& a=1)const
		{ PEN_::put_value(affine_func_(get_value(),v,alpha_*a)); }
	void put_value()const { put_value(get_pen_value()); }
	void put_value_alpha(const alpha_type& a)const { put_value(get_pen_value(),a); }
	void put_hline(int l, const alpha_type &a = 1){for(;l>0;l--,inc_x())put_value_alpha(a);}

	void put_value_clip(const value_type &v, const alpha_type& a=1)const
		{ if(!clipped())PEN_::put_value(affine_func_(get_value(),v,alpha_*a)); }
	void put_value_clip()const { put_value_clip(get_pen_value()); }
	void put_value_clip_alpha(const alpha_type& a)const { put_value_clip(get_pen_value(),a); }
	void put_hline_clip(int l, const alpha_type &a = 1){l=std::min(l,w_-x_);for(;l>0;l--,inc_x())put_value_clip_alpha(a);}

	//the put_block functions do not modify the pen
	void put_block(int h, int w, const alpha_type &a = 1)
	{
		self_type row(*this);
		for(;h>0;h--,row.inc_y())
		{
			self_type col(row);
			col.put_hline(w,a);
		}
	}

	void put_block_clip(int h, int w, const alpha_type &a = 1)
	{
		self_type row(*this);

		//clip start position
		if(row.x_ < 0) { w+=row.x_; row.inc_x(-row.x_); }
		if(row.y_ < 0) { h+=row.y_; row.inc_y(-row.y_);	}

		//clip width and height of copy rect
		h = std::min(h,h_-y_);
		w = std::min(w,w_-x_);

		//copy rect
		for(;h>0;h--,row.inc_y())
		{
			self_type col(row);
			col.put_hline(w,a);	//already clipped
		}
	}
};

};

/* === E X T E R N S ======================================================= */

/* === E N D =============================================================== */

#endif // SYNFIG_PEN
