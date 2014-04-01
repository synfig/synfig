/* === S Y N F I G ========================================================= */
/*!	\file valueoperations.cpp
**	\brief Implementation of common operations with ValueBase.
**
**	$Id$
**
**	\legal
**	......... ... 2014 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valueoperations.h"
#include "weightedvalue.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

types_namespace::TypeWeightedValueBase *ValueAverage::allowed_types[] =
{
	&types_namespace::TypeWeightedValue<Real>::instance,
	&types_namespace::TypeWeightedValue<BLinePoint>::instance,
	&types_namespace::TypeWeightedValue<Matrix>::instance,
	&types_namespace::TypeWeightedValue<Segment>::instance,
	&types_namespace::TypeWeightedValue<Transformation>::instance,
	&types_namespace::TypeWeightedValue<Vector>::instance,
	&types_namespace::TypeWeightedValue<WidthPoint>::instance
};

types_namespace::TypeWeightedValueBase* ValueAverage::get_weighted_type_for(Type &type)
{
	for(unsigned int i = 0; i < sizeof(allowed_types)/sizeof(allowed_types[0]); ++i)
		if (allowed_types[i]->get_contained_type() == type)
			return allowed_types[i];
	return NULL;
}

Type& ValueAverage::get_type_from_weighted(Type& type)
{
	for(unsigned int i = 0; i < sizeof(allowed_types)/sizeof(allowed_types[0]); ++i)
		if (allowed_types[i] == &type)
			return allowed_types[i]->get_contained_type();
	return type_nil;
}

Type& ValueAverage::convert_to_weighted_type(Type &type)
{
	Type* t = get_weighted_type_for(type);
	return t == NULL ? type_nil : *t;
}

bool ValueAverage::check_weighted_type(Type& type) {
	for(unsigned int i = 0; i < sizeof(allowed_types)/sizeof(allowed_types[0]); ++i)
		if (allowed_types[i] == &type)
			return true;
	return false;
}

ValueBase ValueAverage::average_weighted(const ValueBase &weighted_list, const ValueBase &default_value)
{
	if (weighted_list.get_type() != type_list) return default_value;

	const ValueBase::List &list = weighted_list.get_list();
	ValueBase::List values_list;
	values_list.reserve(list.size());
	std::vector<Real> weights_list;
	weights_list.reserve(list.size());
	for(ValueBase::List::const_iterator i = list.begin(); i != list.end(); ++i) {
		types_namespace::TypeWeightedValueBase *t =
			dynamic_cast<types_namespace::TypeWeightedValueBase *>(&(i->get_type()));
		if (t == NULL) continue;
		if (!check_weighted_type(*t)) continue;
		weights_list.push_back( t->extract_weight(*i) );
		values_list.push_back( t->extract_value(*i) );
	}
	return average_generic(
		values_list.begin(), values_list.end(),
		weights_list.begin(), weights_list.end(),
		default_value );
}

void ValueAverage::set_average_value_weighted(ValueBase &weighted_list, const ValueBase &value)
{
	if (weighted_list.get_type() != type_list) return;

	ValueBase::List list = weighted_list.get_list();
	if (list.empty()) return;
	types_namespace::TypeWeightedValueBase *t =
		dynamic_cast<types_namespace::TypeWeightedValueBase *>(&(list.front().get_type()));
	if (t == NULL) return;
	if (!check_weighted_type(*t)) return;

	ValueBase::List values_list;
	values_list.reserve(list.size());
	std::vector<Real> weights_list;
	weights_list.reserve(list.size());
	for(ValueBase::List::const_iterator i = list.begin(); i != list.end(); ++i) {
		if (i->get_type() != *t) return;
		weights_list.push_back( t->extract_weight(*i) );
		values_list.push_back( t->extract_value(*i) );
	}
	set_average_value_generic(
		values_list.begin(), values_list.end(),
		weights_list.begin(), weights_list.end(),
		value );

	std::vector<Real>::const_iterator j = weights_list.begin();
	for(ValueBase::List::const_iterator i = values_list.begin(); i != values_list.end(); ++i, ++j)
		list[i - values_list.begin()] = t->create_weighted_value(*j, *i);
	weighted_list = list;
}

