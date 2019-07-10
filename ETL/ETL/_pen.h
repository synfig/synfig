/*! ========================================================================
** Extended Template Library
** Pen Template Class Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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

#ifndef __ETL__PEN_H
#define __ETL__PEN_H

/* === H E A D E R S ======================================================= */

#include "_curve_func.h"
#include <cassert>
#include <iterator>
#include <algorithm>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

template<typename T>
class generic_pen_row_iterator
{
public:
	struct iterator_category : public std::random_access_iterator_tag {};
	typedef T value_type;
	typedef int difference_type;
	typedef value_type* pointer;
	typedef value_type& reference;

	typedef generic_pen_row_iterator<value_type> self_type;

	pointer data_;
	int pitch_;

	reference operator[](int i)const { assert(data_); return *(pointer)( (char*)data_+pitch_*i ); }
	reference operator*()const { assert(data_); return *data_; }
	pointer operator->() const { assert(data_); return &(operator*()); }

	void inc() { assert(data_); data_ = (pointer)((char*)data_ + pitch_); }
	void inc(int n) { assert(data_); data_ = (pointer)((char*)data_ + n*pitch_); }

	void dec() { assert(data_); data_ = (pointer)((char*)data_ - pitch_); }
	void dec(int n) { assert(data_); data_ = (pointer)((char*)data_ - n*pitch_); }

	const self_type &operator++() { assert(data_); inc(); return *this; }
	const self_type &operator--() { assert(data_); dec(); return *this; }

	self_type operator++(int)
		{ assert(data_); self_type ret(*this); inc(); return ret; }
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

	operator bool()const { return (bool)data_; }
	bool operator!()const { return !data_; }

	generic_pen_row_iterator(pointer data, int pitch):data_(data), pitch_(pitch) { }
	generic_pen_row_iterator():data_(NULL), pitch_(0) { }
};

template<typename T, typename AT=T>
class generic_pen
{
public:
	typedef T value_type;
	typedef AT accumulator_type;
	typedef value_type* pointer;
	typedef accumulator_type* accumulator_pointer;
	typedef const value_type* const_pointer;
	typedef const accumulator_type* const_accumulator_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	typedef pointer iterator_x;
	typedef const_pointer const_iterator_x;

	typedef generic_pen_row_iterator<value_type> iterator_y;
	typedef generic_pen_row_iterator<const value_type> const_iterator_y;

	struct difference_type
	{
		typedef int value_type;
		value_type x,y;
		difference_type(value_type x, value_type y):x(x),y(y) { }
		const value_type &operator[](int i) const { return i?y:x; }
		value_type &operator[](int i) { return i?y:x; }
	};

protected:
	int x_,y_;
	int w_,h_;
private:
	int pitch_;
	value_type value_;
	value_type *data_;

	typedef generic_pen<T,AT> self_type;

	void addptr(int nbytes)
	{
		data_ = (pointer)((char*)data_ + nbytes);
	}

	void subptr(int nbytes)
	{
		data_ = (pointer)((char*)data_ - nbytes);
	}

public:

	generic_pen(value_type *data, int w, int h, int pitch):
		x_(0),
		y_(0),
		w_(w),
		h_(h),
		pitch_(pitch),
		data_(data)
	{
	}

	generic_pen(value_type *data, int w, int h):
		x_(0),
		y_(0),
		w_(w),
		h_(h),
		pitch_(sizeof(value_type)*w),
		data_(data)
	{
	}

	generic_pen():data_(NULL) { }

	self_type& move(int a, int b)
	{
		assert(data_);
		x_ += a, y_ += b;
		addptr(b*pitch_ + a*sizeof(value_type));
		return *this;
	}
	self_type& move_to(int x, int y) { assert(data_); return move(x - x_,y - y_);}

	template<typename TT, typename ATT>
	self_type& move_to(const generic_pen<TT, ATT> &p) { assert(data_ && p.data_); return move_to(p.x_,p.y_);}

	void set_value(const value_type &v) { value_=v; }

	void inc_x() { assert(data_); x_++; data_++; }
	void dec_x() { assert(data_); x_--; data_--; }
	void inc_y() { assert(data_); y_++; addptr(pitch_); }
	void dec_y() { assert(data_); y_--; subptr(pitch_); }

	void inc_x(int n) { assert(data_); x_+=n; data_+=n; }
	void dec_x(int n) { assert(data_); x_-=n; data_-=n; }
	void inc_y(int n) { assert(data_); y_+=n; data_ = (pointer)((char*)data_ + pitch_*n); }
	void dec_y(int n) { assert(data_); y_-=n; data_ = (pointer)((char*)data_ - pitch_*n); }

	void put_value(const value_type &v)const { assert(data_); *data_=v; }
	void put_value()const { assert(data_); put_value(value_); }

	void put_value_clip(const value_type &v)const
		{ if(!clipped()) put_value(v); }
	void put_value_clip()const { put_value_clip(value_); }

	const_reference get_value()const { assert(data_); return *data_; }

	const_reference get_value_at(int x, int y)const { assert(data_); return ((pointer)(((char*)data_)+y*pitch_))[x]; }

	const_reference get_value_clip_at(int x, int y)const { assert(data_); if(clipped(x,y))return value_type(); return ((pointer)(((char*)data_)+y*pitch_))[x]; }

	const value_type get_value_clip()const { assert(data_); if(clipped())return value_type(); return *data_; }

	const value_type get_pen_value()const { return value_; }

	void put_hline(int l,const value_type &v)
	{for(;l>0;l--,inc_x())put_value(v);}

	void put_hline(int l) {put_hline(l,value_);}

	void put_hline_clip(int l, const value_type &v)
	{l=std::min(l,w_-x_);for(;l>0;l--,inc_x())put_value_clip(v);}

	void put_hline_clip(int l) {put_hline_clip(l,value_);}

	//the put_block functions do not modify the pen
	void put_block(int h, int w, const value_type &v)
	{
		self_type row(*this);
		for(;h>0;h--,row.inc_y())
		{
			self_type col(row);
			col.put_hline(w,v);
		}
	}

	void put_block(int h, int w) { put_block(h,w,value_); }

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

	void put_block_clip(int h, int w) { put_block(h,w,value_); }


	iterator_x operator[](int i)const { assert(data_); return (pointer)(((char*)data_)+i*pitch_); }

	iterator_x x() { assert(data_); return data_; }
	iterator_x begin_x() { assert(data_); return data_-x_; }
	iterator_x end_x() { assert(data_); return data_-x_+w_; }

	iterator_y y() { assert(data_); return iterator_y(data_,pitch_); }
	iterator_y begin_y() { assert(data_); return iterator_y((pointer)((char*)data_ - y_*pitch_),pitch_); }
	iterator_y end_y() { assert(data_); return iterator_y((pointer)((char*)data_ + (h_-y_)*pitch_),pitch_); }

	operator bool()const { return (bool)data_; }
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

	difference_type	diff_begin()const {return difference_type(-x_,-y_);}
	difference_type	diff_end()const {return difference_type(w_-x_,h_-y_);}

	self_type get_start()const 	{return *this + diff_begin(); }
	self_type get_end()const 	{return *this + diff_end(); }

	int get_width()const {return w_;}
	int get_height()const {return h_;}

	int get_w()const {return w_;}
	int get_h()const {return h_;}
	int get_pitch()const {return pitch_;}
};

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
	void get_alpha(alpha_type &a) const { a=alpha_; }
	void set_alpha(alpha_type a) { alpha_=a; }

	void put_value(const value_type &v, alpha_type a=1)const
		{ PEN_::put_value(affine_func_(get_value(),v,alpha_*a)); }
	void put_value()const { put_value(get_pen_value()); }
	void put_value_alpha(alpha_type a)const { put_value(get_pen_value(),a); }
	void put_hline(int l, const alpha_type &a = 1){for(;l>0;l--,inc_x())put_value_alpha(a);}

	void put_value_clip(const value_type &v, alpha_type a=1)const
		{ if(!clipped())PEN_::put_value(affine_func_(get_value(),v,alpha_*a)); }
	void put_value_clip()const { put_value_clip(get_pen_value()); }
	void put_value_clip_alpha(alpha_type a)const { put_value_clip(get_pen_value(),a); }
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

#endif
