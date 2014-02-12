/* === S Y N F I G ========================================================= */
/*!	\file valueoperations.h
**	\brief Affine Transformation of ValueBase class
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_VALUEOPERATIONS_H
#define __SYNFIG_VALUEOPERATIONS_H

/* === H E A D E R S ======================================================= */

#include "value.h"
#include "transformation.h"
#include <vector>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

/*!	\class ValueTransformation
**	\todo writeme
*/
class ValueTransformation
{
private:
	//! it's static class
	ValueTransformation() { }

public:
	static bool check_type(ValueBase::Type type) {
		switch(type) {
		case ValueBase::TYPE_ANGLE:
		case ValueBase::TYPE_BLINEPOINT:
		case ValueBase::TYPE_MATRIX:
		case ValueBase::TYPE_SEGMENT:
		case ValueBase::TYPE_TRANSFORMATION:
		case ValueBase::TYPE_VECTOR:
		case ValueBase::TYPE_WIDTHPOINT:
			return true;
		default:
			break;
		}
		return false;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase transform(const Transformation &transformation, const ValueBase &value) {
		switch(value.get_type()) {
		case ValueBase::TYPE_ANGLE:
			return value.get(Angle()) + transformation.angle;
		case ValueBase::TYPE_BLINEPOINT:
			{
				BLinePoint bp(value.get(BLinePoint()));
				bp.set_vertex( transformation.transform(bp.get_vertex()) );
				bp.set_tangent1( transformation.transform(bp.get_tangent1(), false) );
				bp.set_tangent2( transformation.transform(bp.get_tangent2(), false) );
				return bp;
			}
			break;
		case ValueBase::TYPE_MATRIX:
			return transformation.transform(value.get(Matrix()));
		case ValueBase::TYPE_SEGMENT:
			{
				Segment s(value.get(Segment()));
				s.p1 = transformation.transform(s.p1);
				s.t1 = transformation.transform(s.t1, false);
				s.p2 = transformation.transform(s.p2);
				s.t2 = transformation.transform(s.t2, false);
				return s;
			}
			break;
		case ValueBase::TYPE_TRANSFORMATION:
			return transformation.transform(value.get(Transformation()));
		case ValueBase::TYPE_VECTOR:
			return transformation.transform(value.get(Vector()));
		case ValueBase::TYPE_WIDTHPOINT:
			{
				WidthPoint wp(value.get(WidthPoint()));
				wp.set_width( wp.get_width()*transformation.scale[1] );
				return wp;
			}
			break;
		default:
			break;
		}
		return value;
	}

	static ValueBase back_transform(const Transformation &transformation, const ValueBase &value)
		{ return transform(transformation.get_back_transformation(), value); }
};

/*!	\class ValueAverage
**	\todo writeme
*/
class ValueAverage
{
private:
	//! it's static class
	ValueAverage() { }

public:
	static bool check_type(ValueBase::Type type) {
		switch(type) {
		case ValueBase::TYPE_REAL:
		case ValueBase::TYPE_BLINEPOINT:
		case ValueBase::TYPE_MATRIX:
		case ValueBase::TYPE_SEGMENT:
		case ValueBase::TYPE_TRANSFORMATION:
		case ValueBase::TYPE_VECTOR:
		case ValueBase::TYPE_WIDTHPOINT:
			return true;
		default:
			break;
		}
		return false;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase add(const ValueBase &value_a, const ValueBase &value_b, const ValueBase &default_value)
	{
		if (value_a.get_type() != value_b.get_type()) return default_value;

		switch(value_a.get_type()) {
		case ValueBase::TYPE_REAL:
			return value_a.get(Real()) + value_b.get(Real());
		case ValueBase::TYPE_BLINEPOINT:
			{
				BLinePoint res(value_a.get(BLinePoint()));
				const BLinePoint &b = value_b.get(BLinePoint());
				res.set_vertex( res.get_vertex() + b.get_vertex() );
				res.set_tangent1( res.get_tangent1() + b.get_tangent1() );
				res.set_tangent2( res.get_tangent2() + b.get_tangent2() );
				return res;
			}
			break;
		case ValueBase::TYPE_MATRIX:
			return value_a.get(Matrix()) + value_b.get(Matrix());
		case ValueBase::TYPE_SEGMENT:
			{
				Segment res(value_a.get(Segment()));
				const Segment &b = value_b.get(Segment());
				res.p1 += b.p1;
				res.t1 += b.t1;
				res.p2 += b.p2;
				res.t2 += b.t2;
				return res;
			}
			break;
		case ValueBase::TYPE_TRANSFORMATION:
			return Transformation(
				value_a.get(Transformation()).get_matrix()
			  + value_b.get(Transformation()).get_matrix() );
		case ValueBase::TYPE_VECTOR:
			return value_a.get(Vector()) + value_b.get(Vector());
		case ValueBase::TYPE_WIDTHPOINT:
			{
				WidthPoint res(value_a.get(WidthPoint()));
				const WidthPoint &b = value_b.get(WidthPoint());
				res.set_width( res.get_width() + b.get_width() );
				return res;
			}
			break;
		default:
			break;
		}

		return default_value;
	}

	static ValueBase add(const ValueBase &value_a, const ValueBase &value_b)
		{ return add(value_a, value_b, value_a); }

	static ValueBase multiply(const ValueBase &value, Real amplifier)
	{
		switch(value.get_type()) {
		case ValueBase::TYPE_REAL:
			return value.get(Real()) * amplifier;
		case ValueBase::TYPE_BLINEPOINT:
			{
				BLinePoint res(value.get(BLinePoint()));
				res.set_vertex( res.get_vertex() * amplifier );
				res.set_tangent1( res.get_tangent1() * amplifier );
				res.set_tangent2( res.get_tangent2() * amplifier );
				return res;
			}
			break;
		case ValueBase::TYPE_MATRIX:
			return value.get(Matrix()) * amplifier;
		case ValueBase::TYPE_SEGMENT:
			{
				Segment res(value.get(Segment()));
				res.p1 *= amplifier;
				res.t1 *= amplifier;
				res.p2 *= amplifier;
				res.t2 *= amplifier;
				return res;
			}
			break;
		case ValueBase::TYPE_TRANSFORMATION:
			return Transformation( value.get(Transformation()).get_matrix() * amplifier );
		case ValueBase::TYPE_VECTOR:
			return value.get(Vector()) * amplifier;
		case ValueBase::TYPE_WIDTHPOINT:
			{
				WidthPoint res(value.get(WidthPoint()));
				res.set_width( res.get_width() * amplifier );
				return res;
			}
			break;
		default:
			break;
		}

		return value;
	}

	// iterators should provide following operations:
	// comparison i == j, increment ++i, indirection *i, copy constructor i(j)
	template<typename ConstIterator, typename ConstWeightIterator>
	static ValueBase average_generic(
		ConstIterator begin,
		ConstIterator end,
		ConstWeightIterator weight_begin,
		ConstWeightIterator weight_end,
		const ValueBase &default_value = ValueBase() )
	{
		if (begin == end) return ValueBase();

		// check values
		int count = 0;
		ValueBase::Type type = (*begin).get_type();
		if (!check_type(type)) return ValueBase();
		for(ConstIterator i(begin); !(i == end); ++i, ++count)
			if ((*i).get_type() != type) return ValueBase();

		// check weights
		bool weights = !(weight_begin == weight_end);
		Real summary_weight = 0.0;
		int weights_count = 0;
		if (weights)
			for(ConstWeightIterator i(weight_begin); !(i == weight_end) && weights_count < count; ++i, ++weights_count)
				summary_weight += *i;
		if (weights_count < count || summary_weight == 0.0) weights = false;
		if (!weights) summary_weight = (Real)count;
		Real amplifier = 1.0/summary_weight;

		// process
		ValueBase summary;
		if (weights)
		{
			// weighted
			ConstWeightIterator j(weight_begin);
			ConstIterator i(begin);
			summary = multiply(*i, (*j) * amplifier);
			for(++i, ++j; !(i == end); ++i, ++j)
				summary = add(summary, multiply(*i, (*j) * amplifier), ValueBase());
		}
		else
		{
			// simple
			ConstIterator i(begin);
			summary = multiply(*i, amplifier);
			for(++i; !(i == end); ++i)
				summary = add(summary, multiply(*i, amplifier), ValueBase());
		}

		return summary.get_type() == ValueBase::TYPE_NIL ? default_value : summary;
	}

	template<typename ConstIterator>
	static ValueBase average_generic(ConstIterator begin, ConstIterator end, const ValueBase &default_value = ValueBase())
		{ return average_generic(begin, end, (Real*)NULL, (Real*)NULL, default_value); }

	static ValueBase average(const ValueBase &list, const ValueBase &weights, const ValueBase &default_value)
	{
		if (list.get_type() != ValueBase::TYPE_LIST) return default_value;

		const std::vector<ValueBase> &list_vector = list.get_list();
		if (weights.get_type() == ValueBase::TYPE_LIST)
		{
			std::vector<Real> weights_vector_real;
			weights_vector_real.reserve(weights.get_list().size());
			const std::vector<ValueBase> &weights_vector = weights.get_list();
			for(std::vector<ValueBase>::const_iterator i = weights_vector.begin(); i != weights_vector.end(); ++i)
				if (i->get_type() == ValueBase::TYPE_REAL)
					weights_vector_real.push_back(*i); else break;
			if (weights_vector.size() >= list_vector.size())
				return average_generic(
					list_vector.begin(), list_vector.end(),
					weights_vector_real.begin(), weights_vector_real.end(),
					default_value );
		}
		return average_generic(list_vector.begin(), list_vector.end(), default_value);
	}

	static ValueBase average(const ValueBase &list, const ValueBase &weights)
		{ return average(list, weights, ValueBase()); }
	static ValueBase average(const ValueBase &list)
		{ return average(list, ValueBase()); }
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
