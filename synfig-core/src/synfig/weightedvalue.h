/* === S Y N F I G ========================================================= */
/*!	\file weightedvalue.h
**	\brief A weighted value template
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

#ifndef __SYNFIG_WEIGHTEDVALUE_H
#define __SYNFIG_WEIGHTEDVALUE_H

/* === H E A D E R S ======================================================= */

#include "real.h"
#include "type.h"
#include "value.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

template<typename T>
class WeightedValue
{
public:
	typedef T ValueType;
	Real weight;
	ValueType value;
	WeightedValue(): weight(), value() { }
	explicit WeightedValue(Real weight): weight(weight), value() { }
	explicit WeightedValue(Real weight, const ValueType &value): weight(weight), value(value) { }
};

namespace types_namespace {
	class TypeWeightedValueBase: public Type
	{
	public:
		virtual Type& get_contained_type() = 0;
		virtual ValueBase create_weighted_value(Real weight, const ValueBase &value) = 0;
		virtual Real extract_weight(const ValueBase &value) = 0;
		virtual ValueBase extract_value(const ValueBase &value) = 0;
	};

	template<typename T>
	class TypeWeightedValue: public TypeWeightedValueBase
	{
		static String to_string(const WeightedValue<T> &x) {
			return etl::strprintf("Weight (%f) %s", x.weight, value_to_string(x.value).c_str());
		}
		void initialize_vfunc(Description &description)
		{
			Type &type = get_type_alias(T()).type;
			type.initialize();

			Type::initialize_vfunc(description);
			description.name = "weighted_" + type.description.name;
			description.local_name = local_n("weighted") + String(" ") + type.description.local_name;
			register_all_but_compare<WeightedValue<T>, TypeWeightedValue<T>::to_string>();
		}
		Type& get_contained_type() { return get_type_alias(T()).type; }
		ValueBase create_weighted_value(Real weight, const ValueBase &value)
			{ return WeightedValue<T>(weight, value.get(T())); };
		Real extract_weight(const ValueBase &value)
			{ return value.get(WeightedValue<T>()).weight; };
		ValueBase extract_value(const ValueBase &value)
			{ return value.get(WeightedValue<T>()).value; };
	public:
		static TypeWeightedValue instance;
	};

	template<typename T>
	TypeWeightedValue<T> TypeWeightedValue<T>::instance;

	template<typename T>
	TypeAlias< WeightedValue<T> > get_type_alias(WeightedValue<T> const&) { return TypeAlias< WeightedValue<T> >(TypeWeightedValue<T>::instance); }
}


}; // END of namespace synfig

#endif
