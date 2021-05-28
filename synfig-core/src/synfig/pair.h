/* === S Y N F I G ========================================================= */
/*!	\file pair.h
**	\brief A pair value template
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

#ifndef __SYNFIG_PAIR_H
#define __SYNFIG_PAIR_H

/* === H E A D E R S ======================================================= */

#include <ETL/stringf>
#include <utility>
#include "type.h"
#include "value.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace types_namespace {
	class TypePairBase: public Type
	{
	public:
		virtual Type& get_first_type() = 0;
		virtual Type& get_second_type() = 0;
		virtual ValueBase create_value(const ValueBase &first, const ValueBase &second) = 0;
		virtual ValueBase extract_first(const ValueBase &value) = 0;
		virtual ValueBase extract_second(const ValueBase &value) = 0;
	};

	template<typename T1, typename T2>
	class TypePair: public TypePairBase
	{
		typedef std::pair<T1, T2> ValueType;

		static String to_string(const ValueType &x) {
			return etl::strprintf("Pair (%s, %s)", value_to_string(x.first).c_str(), value_to_string(x.second).c_str());
		}
		void initialize_vfunc(Description &description)
		{
			Type &type_first = get_type_alias(T1()).type;
			type_first.initialize();
			Type &type_second = get_type_alias(T2()).type;
			type_second.initialize();

			Type::initialize_vfunc(description);
			description.name = "pair_" + type_first.description.name + "_" + type_second.description.name;
			description.local_name = local_n("Pair") + String(" (")
					               + type_first.description.local_name + String(", ")
					               + type_second.description.local_name + String(")");
			register_all_but_compare<ValueType, TypePair::to_string>();
		}
		virtual Type& get_first_type() { return get_type_alias(T1()).type; }
		virtual Type& get_second_type() { return get_type_alias(T2()).type; }
		virtual ValueBase create_value(const ValueBase &first, const ValueBase &second)
			{ return ValueType(first.get(T1()), second.get(T2())); };
		virtual ValueBase extract_first(const ValueBase &value)
			{ return value.get(ValueType()).first; };
		virtual ValueBase extract_second(const ValueBase &value)
			{ return value.get(ValueType()).second; };
	public:
		static TypePair instance;
	};

	template<typename T1, typename T2>
	TypePair<T1, T2> TypePair<T1, T2>::instance;

	template<typename T1, typename T2>
	TypeAlias< std::pair<T1, T2> > get_type_alias(typename std::pair<T1, T2> const&)
		{ return TypeAlias< std::pair<T1, T2> >(TypePair<T1, T2>::instance); }
}


}; // END of namespace synfig

#endif
