/* === S Y N F I G ========================================================= */
/*!	\file synfig/rendering/software/function/array.h
**	\brief Template Header
**
**	\legal
**	......... ... 2015 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_RENDERING_SOFTWARE_ARRAY_H
#define __SYNFIG_RENDERING_SOFTWARE_ARRAY_H

/* === H E A D E R S ======================================================= */

#include <cassert>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {
namespace rendering {
namespace software {

template<typename T0, T0 F()>
struct Function0 { T0 operator() () { return F(); } };

template<typename F, typename T0, typename T1>
struct Function1 { T0 operator() (T1 x1) { return F(x1); } };

template<typename T0, typename T1, typename T2, T0 F(T1, T2)>
struct Function2 { T0 operator() (T1 x1, T2 x2) { return F(x1, x2); } };



template<typename T, unsigned int rank>
class Array;

template<typename T>
class Array<T, 0u> {
public:
	enum { Rank = 0 };
	typedef T Type;
	typedef Type& ChildTarget;
	typedef Type ChildTargetRef;

	Type *pointer;

	Array(): pointer() { }
	explicit Array(Type *pointer):
		pointer(pointer) { }
	Array(Type *pointer, int, int):
		pointer(pointer) { }
	template<typename TT>
	Array(Type *pointer, const Array<TT, Rank>&):
		pointer(pointer) { }
	template<typename TT>
	Array(Type *pointer, int, int, const Array<TT, Rank>&):
		pointer(pointer) { }

	template<typename TT>
	void set_dims(const Array<TT, Rank> &) { }

	Array& set_count(int, int)
		{ assert(false); return *this; }
	Array& set_stride(int, int)
		{ assert(false); return *this; }
	Array& set_dim(int, int, int)
		{ assert(false); return *this; }
	int get_count(int = 0) const
		{ assert(false); return 0; }
	int get_stride(int = 0) const
		{ assert(false); return 0; }

	void fill(const Type &x) const
		{ *pointer = x; }

	template<typename TT>
	void assign(const Array<TT, Rank> &x) const
		{ *pointer = *x.pointer;}

	template<typename function>
	void process(const Type &x) const
		{ *pointer = function()(*pointer, x); }

	template<typename function, typename TT>
	void process(const Array<TT, Rank> &x) const
		{ *pointer = function()(*pointer, *x.pointer); }

	template<typename function>
	void process() const
		{ *pointer = function()(*pointer); }

protected:
	template<typename TT, unsigned int RR>
	friend class Array;

	ChildTarget target() const { return *pointer; }
	ChildTargetRef& target_ref() const { return *pointer; }

	template<typename TT>
	Array<TT, Rank+1> split_items_ptr(TT*) const
	{
		assert(sizeof(T) % sizeof(TT) == 0);
		return Array<TT, Rank+1>((TT*)pointer, sizeof(T)/sizeof(TT), 1);
	}

	template<typename TT>
	Array<TT, Rank> group_items_ptr(TT*) const
	{
		return Array<TT, Rank>((TT*)pointer);
	}
};

template<typename T, unsigned int rank>
class Array: public Array<T, rank-1> {
public:
	enum { Rank = rank };
	typedef T Type;
	typedef Array<Type, Rank-1> Parent;
	typedef typename Parent::ChildTarget Target;
	typedef Array ChildTarget;
	typedef typename Parent::ChildTargetRef TargetRef;
	typedef const Array ChildTargetRef;

	int count, stride;

protected:
	template<typename TT, unsigned int RR>
	friend class Array;

	ChildTarget target() const { return *this; }
	ChildTargetRef& target_ref() const { return *this; }

	template<typename TT>
	Array<TT, Rank+1> split_items_ptr(TT*) const { return split_items<TT>(); }

	template<typename TT>
	Array<TT, Rank-1> group_items_ptr(TT*) const { return group_items<TT>(); }

public:
	Array(): count(), stride() { }
	explicit Array(Type *pointer): Parent(pointer), count(), stride() { }
	Array(Type *pointer, int count, int stride): Parent(pointer), count(count), stride(stride) { }
	Array(int count, int stride): count(count), stride(stride) { }

	template<typename TT>
	Array(Type *pointer, const Array<TT, Rank> &other):
		Parent(pointer, other), count(other.count), stride(other.stride) { }
	template<typename TT>
	Array(Type *pointer, int count, int stride, const Array<TT, Rank-1> &other):
		Parent(pointer, other), count(count), stride(stride) { }

	operator const Array<const Type, Rank>& () const
		{ return *(const Array<const Type, Rank>*)this; }

	Parent& sub() { return *this; }
	const Parent& sub() const { return *this; }

	template<typename TT>
	void set_dims(const Array<TT, Rank> &other)
	{
		count = other.count;
		stride = other.stride;
		Parent::set_dims(other);
	}

	Parent& set_count(int x)
		{ this->count = x; return this; }
	Parent& set_stride(int x)
		{ this->count = x; return this; }
	Parent& set_dim(int count, int stride)
		{ this->count = count, this->stride = stride; return *this; }

	Array& set_count(int r, int x)
		{ if (r) Parent::set_count(r-1, x); else count = x; return *this; }
	Array& set_stride(int r, int x)
		{ if (r) Parent::set_stride(r-1, x); else stride = x; return *this; }
	Array& set_dim(int r, int count, int stride)
		{ if (r) Parent::set_dim(r-1, count, stride); else this->count = count, this->stride = stride; return *this; }

	int get_count(int r = 0) const
		{ return r ? Parent::get_count(r-1) : count; }
	int get_stride(int r = 0) const
		{ return r ? Parent::get_stride(r-1) : stride; }

	Target item(int i) const
		{ return Parent(Parent::pointer + i*stride, *this).target(); }
	Target operator[] (int i) const
		{ return item(i); }

	template<typename TT>
	Array<TT, Rank+1> split_items() const {
		assert(sizeof(T) % sizeof(TT) == 0);
		return Array<TT, Rank+1>(
			(TT*)Parent::pointer,
			count,
			(sizeof(T)/sizeof(TT))*stride,
			Parent::split_items_ptr((TT*)NULL) );
	}

	template<typename TT>
	Array<TT, Rank-1> group_items() const {
		assert(sizeof(TT) % sizeof(T) == 0);
		assert(Rank > 1 || sizeof(TT)/sizeof(T) == count);
		assert(Rank == 1 ? stride == 1 : stride % (sizeof(TT)/sizeof(T)) == 0);
		return Array<TT, Rank-1>(
			(TT*)Parent::pointer,
			count,
			stride / (sizeof(TT)/sizeof(T)),
			Parent::group_items_ptr((TT*)NULL) );
	}

	Array get_range(int r, int begin, int end) const
	{
		assert(begin >= 0 && begin <= end && end <= get_count(r));
		Array a(*this);
		a.pointer += a.get_stride(r)*begin;
		a.set_count(r, end - begin);
		return a;
	}

	Array get_range(int r, int begin) const
		{ return get_range(r, begin, get_count(r)); }

	Array<Type, 1> reorder(int d0) const
	{
		Array<Type, 1> a(Parent::pointer);
		a.set_dim(get_count(d0), get_stride(d0));
		return a;
	}

	Array<Type, 2> reorder(int d0, int d1) const
	{
		Array<Type, 2> a(Parent::pointer);
		a.set_dim(get_count(d0), get_stride(d0))
		 .set_dim(get_count(d1), get_stride(d1));
		return a;
	}

	Array<Type, 3> reorder(int d0, int d1, int d2) const
	{
		Array<Type, 3> a(Parent::pointer);
		a.set_dim(get_count(d0), get_stride(d0))
		 .set_dim(get_count(d1), get_stride(d1))
		 .set_dim(get_count(d2), get_stride(d2));
		return a;
	}

	Array<Type, 4> reorder(int d0, int d1, int d2, int d3) const
	{
		Array<Type, 4> a(Parent::pointer);
		a.set_dim(get_count(d0), get_stride(d0))
		 .set_dim(get_count(d1), get_stride(d1))
		 .set_dim(get_count(d2), get_stride(d2))
		 .set_dim(get_count(d3), get_stride(d3));
		return a;
	}

	Array<Type, 5> reorder(int d0, int d1, int d2, int d3, int d4) const
	{
		Array<Type, 5> a(Parent::pointer);
		a.set_dim(get_count(d0), get_stride(d0))
		 .set_dim(get_count(d1), get_stride(d1))
		 .set_dim(get_count(d2), get_stride(d2))
		 .set_dim(get_count(d3), get_stride(d3))
		 .set_dim(get_count(d4), get_stride(d4));
		return a;
	}

	class ReverseIterator;

	class Iterator
	{
	private:
		Array<Type, Rank-1> array;
		int stride;
		Type *end;

	public:
		Iterator(): stride(), end() { }
		explicit Iterator(const Array &array):
			array(array),
			stride(array.stride),
			end(array.pointer + array.count*array.stride) { }
		Iterator(const Array &array, int begin):
			array(array.pointer + begin*array.stride, array),
			stride(array.stride),
			end(array.pointer + array.count*array.stride) { }
		Iterator(const Array &array, int begin, int end):
			array(array.pointer + begin*array.stride, array),
			stride(array.stride),
			end(array.pointer + end*array.stride) { }
		Iterator(const Array &array, Type *current):
			array(current, array),
			stride(array.stride),
			end(array.pointer + end*array.stride) { }
		Iterator(const Array &array, Type *current, Type *end):
			array(current, array),
			stride(array.stride),
			end(end) { }

		explicit Iterator(const ReverseIterator &reverse):
			array(reverse.get_end() - reverse.get_stride(), reverse.get_array()),
			stride(-reverse.get_stride()),
			end(reverse.get_pointer() - reverse.get_stride()) { }

		operator bool () const { return array.pointer < end; }
		TargetRef& operator *  () const { assert(*this); return array.target_ref(); }
		TargetRef* operator -> () const { assert(*this); return &array.target_ref(); }

		Iterator& operator ++ () { array.pointer += stride; return *this; }
		Iterator& operator -- () { array.pointer -= stride; return *this; }
		Iterator  operator ++ () const { Iterator copy(*this); array.pointer += stride; return copy; }
		Iterator  operator -- () const { Iterator copy(*this); array.pointer -= stride; return copy; }
		Iterator& operator += (int i) { array.pointer += i*stride; return *this; }
		Iterator& operator -= (int i) { array.pointer -= i*stride; return *this; }

		Iterator operator + (int i) const { return Iterator(array, array.pointer + i*stride, end); }
		Iterator operator - (int i) const { return Iterator(array, array.pointer - i*stride, end); }

		int operator - (const Iterator &i) const { return (array.pointer - i.array.pointer)/stride; }

		Array<Type, Rank-1> get_array() const { return array; }
		Type* get_pointer() const { return array.pointer; }
		Type& get_reference() const { return *array.pointer; }
		int get_stride() const { return stride; }
		Type* get_end() const { return end; }
		int get_count_down() const { return end - array.pointer; }

		ReverseIterator get_reverse() const
			{ return ReverseIterator(*this); }

		Iterator get_sub_range(int count) const
		{
			Iterator j(*this);
			j.end = array.pointer + stride*min(get_count_down(), count);
			return j;
		}
	};

	class ReverseIterator
	{
	private:
		Array<Type, Rank-1> array;
		int stride;
		Type *end;

	public:
		ReverseIterator(): stride(), end() { }
		explicit ReverseIterator(const Array &array):
			array(array.pointer + (array.count - 1)*array.stride, array),
			stride(-array.stride),
			end(array.pointer - array.stride) { }
		ReverseIterator(const Array &array, int begin):
			array(array.pointer + (array.count - 1 - begin)*array.stride, array),
			stride(-array.stride),
			end(array.pointer - array.stride) { }
		ReverseIterator(const Array &array, int begin, int end):
			array(array.pointer + (array.count - 1 - begin)*array.stride, array),
			stride(array.stride),
			end(array.pointer + (array.count - 1 - end)*array.stride, array) { }
		ReverseIterator(const Array &array, Type *current):
			array(current, array),
			stride(-array.stride),
			end(array.pointer - array.stride) { }
		ReverseIterator(const Array &array, Type *current, Type *end):
			array(current, array),
			stride(-array.stride),
			end(end) { }

		explicit ReverseIterator(const Iterator &base):
			array(base.get_end() - base.get_stride(), base.get_array()),
			stride(-base.get_stride()),
			end(base.get_pointer() - base.get_stride()) { }

		operator bool () const { return array.pointer > end; }
		TargetRef& operator *  () const { assert(*this); return array.target_ref(); }
		TargetRef* operator -> () const { assert(*this); return &array.target_ref(); }

		ReverseIterator& operator ++ () { array.pointer += stride; return *this; }
		ReverseIterator& operator -- () { array.pointer -= stride; return *this; }
		ReverseIterator  operator ++ () const { Iterator copy(*this); array.pointer += stride; return copy; }
		ReverseIterator  operator -- () const { Iterator copy(*this); array.pointer -= stride; return copy; }
		ReverseIterator& operator += (int i) { array.pointer += i*stride; return *this; }
		ReverseIterator& operator -= (int i) { array.pointer -= i*stride; return *this; }

		ReverseIterator operator + (int i) const { return Iterator(array, array.pointer + i*stride, end); }
		ReverseIterator operator - (int i) const { return Iterator(array, array.pointer - i*stride, end); }

		int operator - (const Iterator &i) const { return (array.pointer - i.array.pointer)/stride; }

		Array<Type, Rank-1> get_array() const { return array; }
		Type* get_pointer() const { return array.pointer; }
		Type& get_reference() const { return *array.pointer; }
		int get_stride() const { return stride; }
		Type* get_end() const { return end; }
		int get_count_down() const { return end - array.pointer; }

		Iterator get_base() const
			{ return Iterator(*this); }

		ReverseIterator get_sub_range(int count) const
		{
			ReverseIterator j(*this);
			j.end = array.pointer + stride*min(get_count_down(), count);
			return j;
		}
	};

	void fill(const Type &x) const
		{ for(Iterator i(*this); i; ++i) i.get_array().template fill(x); }

	template<typename TT>
	void assign(const Array<TT, Rank> &x) const
	{
		typename Array<TT, Rank>::Iterator j(x);
		for(Iterator i(*this); i && j; ++i, ++j) i.get_array().template assign<TT>(j.get_array());
	}

	template<typename function>
	void process(const Type &x) const
		{ for(Iterator i(*this); i; ++i) i.get_array().template process<function>(x); }

	template<typename function, typename TT>
	void process(const Array<TT, Rank> &x) const
	{
		typename Array<TT, Rank>::Iterator j(x);
		for(Iterator i(*this); i && j; ++i, ++j) i.get_array().template process<function, TT>(j.get_array());
	}

	template<typename function>
	void process() const
		{ for(Iterator i(*this); i; ++i) i.get_array().template process<function>(); }
};

template<typename T>
Array<T, 1> make_array(T *pointer, int count, int stride)
	{ return Array<T, 1>(pointer, count, stride == 0 ? 1 : stride); }

template<typename T>
Array<T, 1> make_array(T *pointer, int count)
	{ return Array<T, 1>(pointer, count, 1); }

template<typename T>
Array<T, 2> make_array_2d(T *pointer, int rows, int row_stride, int cols, int col_stride)
	{ return Array<T, 2>(pointer, rows, row_stride).set_dim(0, cols, col_stride); }

template<typename T>
Array<T, 2> make_array_2d(T *pointer, int rows, int cols)
	{ return Array<T, 2>(pointer, rows, cols).set_dim(0, cols, 1); }

}; // END of namespace synfig
};
};

/* === E N D =============================================================== */

#endif
