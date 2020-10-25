/* === S Y N F I G ========================================================= */
/*!	\file valueoperations.h
**	\brief Common operations with ValueBase
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

namespace types_namespace { class TypeWeightedValueBase; }

/*!	\class ValueVector
**	\todo writeme
*/
class ValueVector
{
private:
	//! it's static class
	ValueVector() { }

public:
	static bool check_type(Type &type) {
		return type == type_bline_point
			|| type == type_matrix
			|| type == type_transformation
			|| type == type_vector;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static Vector get_vector(const ValueBase &value);
};


/*!	\class ValueTransformation
**	\todo writeme
*/
class ValueTransformation
{
private:
	//! it's static class
	ValueTransformation() { }

public:
	static bool check_type(Type &type) {
		return type == type_angle
			|| type == type_bline_point
			|| type == type_matrix
			|| type == type_segment
			|| type == type_transformation
			|| type == type_vector
			|| type == type_width_point;
	}

	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase transform(const Transformation &transformation, const ValueBase &value);

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

	static types_namespace::TypeWeightedValueBase *allowed_types[];

public:
	static types_namespace::TypeWeightedValueBase* get_weighted_type_for(Type &type);
	static Type& convert_to_weighted_type(Type &type);
	static Type& get_type_from_weighted(Type& type);

	static bool check_weighted_type(Type& type);
	static bool check_type(Type& type)
		{ return get_weighted_type_for(type) != NULL; }
	static bool check_type(const ValueBase &value)
		{ return check_type(value.get_type()); }

	static ValueBase add(const ValueBase &value_a, const ValueBase &value_b, const ValueBase &default_value);

	static ValueBase add(const ValueBase &value_a, const ValueBase &value_b)
		{ return add(value_a, value_b, value_a); }

	static ValueBase multiply(const ValueBase &value, Real amplifier);

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
		Type &type = (*begin).get_type();
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

		return summary.get_type() == type_nil ? default_value : summary;
	}

	template<typename ConstIterator>
	static ValueBase average_generic(ConstIterator begin, ConstIterator end, const ValueBase &default_value = ValueBase())
		{ return average_generic(begin, end, (Real*)NULL, (Real*)NULL, default_value); }

	static ValueBase average(const ValueBase &list, const ValueBase &weights, const ValueBase &default_value)
	{
		if (list.get_type() != type_list) return default_value;

		const std::vector<ValueBase> &list_vector = list.get_list();
		if (weights.get_type() == type_list)
		{
			std::vector<Real> weights_vector_real;
			weights_vector_real.reserve(weights.get_list().size());
			const std::vector<ValueBase> &weights_vector = weights.get_list();
			for(std::vector<ValueBase>::const_iterator i = weights_vector.begin(); i != weights_vector.end(); ++i)
				if (i->get_type() == type_real)
					weights_vector_real.push_back(i->get(Real())); else break;
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

	static ValueBase average_weighted(const ValueBase &weighted_list, const ValueBase &default_value);
	static ValueBase average_weighted(const ValueBase &weighted_list)
		{ return average_weighted(weighted_list, ValueBase()); }


	// iterators should provide following operations:
	// comparison i == j, increment ++i, indirection *i, copy constructor i(j)
	template<typename Iterator, typename ConstWeightIterator>
	static void set_average_value_generic(
		Iterator begin,
		Iterator end,
		ConstWeightIterator weight_begin,
		ConstWeightIterator weight_end,
		const ValueBase &value )
	{
		if (begin == end) return;

		// check values
		int count = 0;
		Type &type = (*begin).get_type();
		if (!check_type(type)) return;
		for(Iterator i(begin); !(i == end); ++i, ++count)
			if ((*i).get_type() != type) return;

		// find difference
		ValueBase previous_value = average_generic(begin, end, weight_begin, weight_end);
		ValueBase difference = add(value, multiply(previous_value, -1.0));

		// simple
		for(Iterator i(begin); !(i == end); ++i)
			*i = add(*i, difference);
	}

	template<typename Iterator>
	static void set_average_value_generic(Iterator begin, Iterator end, const ValueBase &value)
		{ set_average_value_generic(begin, end, (Real*)NULL, (Real*)NULL, value); }

	static void set_average_value(ValueBase &list, const ValueBase &weights, const ValueBase &value)
	{
		if (list.get_type() != type_list) return;

		std::vector<ValueBase> list_vector = list.get_list();
		if (weights.get_type() == type_list)
		{
			std::vector<Real> weights_vector_real;
			weights_vector_real.reserve(weights.get_list().size());
			const std::vector<ValueBase> &weights_vector = weights.get_list();
			for(std::vector<ValueBase>::const_iterator i = weights_vector.begin(); i != weights_vector.end(); ++i)
				if (i->get_type() == type_real)
					weights_vector_real.push_back(i->get(Real())); else break;
			if (weights_vector.size() >= list_vector.size())
			{
				set_average_value_generic(
					list_vector.begin(), list_vector.end(),
					weights_vector_real.begin(), weights_vector_real.end(),
					value );
				return;
			}
		}
		set_average_value_generic(list_vector.begin(), list_vector.end(), value);
		list = list_vector;
	}

	static void set_average_value(ValueBase &list, const ValueBase &value)
		{ return set_average_value(list, ValueBase(), value); }

	static void set_average_value_weighted(ValueBase &weighted_list, const ValueBase &value);
};

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
