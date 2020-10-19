/* === S Y N F I G ========================================================= */
/*!	\file valueoperations.cpp
**	\brief Implementation of common operations with ValueBase.
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "valueoperations.h"
#include "weightedvalue.h"
#include "blinepoint.h"
#include "segment.h"
#include "widthpoint.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Vector
ValueVector::get_vector(const ValueBase &value) {
	Type &type(value.get_type());
	if (type == type_bline_point)
		return value.get(BLinePoint()).get_vertex();
	else
	if (type == type_matrix)
		return value.get(Matrix()).get_transformed(Vector(0, 0));
	else
	if (type == type_transformation)
		return value.get(Transformation()).transform(Vector(0,0));
	else
	if (type == type_vector)
		return value.get(Vector());
	return Vector(0, 0);
}


ValueBase
ValueTransformation::transform(const Transformation &transformation, const ValueBase &value) {
	Type &type(value.get_type());
	if (type == type_angle)
		return value.get(Angle()) + transformation.angle;
	else
	if (type == type_bline_point)
	{
		BLinePoint bp(value.get(BLinePoint()));
		bp.set_vertex( transformation.transform(bp.get_vertex()) );
		bp.set_tangent1( transformation.transform(bp.get_tangent1(), false) );
		bp.set_tangent2( transformation.transform(bp.get_tangent2(), false) );
		return bp;
	}
	else
	if (type == type_matrix)
		return transformation.transform(value.get(Matrix()));
	else
	if (type == type_segment)
	{
		Segment s(value.get(Segment()));
		s.p1 = transformation.transform(s.p1);
		s.t1 = transformation.transform(s.t1, false);
		s.p2 = transformation.transform(s.p2);
		s.t2 = transformation.transform(s.t2, false);
		return s;
	}
	else
	if (type == type_transformation)
		return transformation.transform(value.get(Transformation()));
	else
	if (type == type_vector)
		return transformation.transform(value.get(Vector()));
	else
	if (type == type_width_point)
	{
		WidthPoint wp(value.get(WidthPoint()));
		wp.set_width( wp.get_width()*transformation.scale[1] );
		return wp;
	}
	return value;
}


ValueBase
ValueAverage::add(const ValueBase &value_a, const ValueBase &value_b, const ValueBase &default_value)
{
	if (value_a.get_type() != value_b.get_type()) return default_value;

	Type &type(value_a.get_type());
	if (type == type_real)
		return value_a.get(Real()) + value_b.get(Real());
	else
	if (type == type_bline_point)
	{
		BLinePoint res(value_a.get(BLinePoint()));
		const BLinePoint &b = value_b.get(BLinePoint());
		res.set_vertex( res.get_vertex() + b.get_vertex() );
		res.set_tangent1( res.get_tangent1() + b.get_tangent1() );
		res.set_tangent2( res.get_tangent2() + b.get_tangent2() );
		return res;
	}
	else
	if (type == type_matrix)
		return value_a.get(Matrix()) + value_b.get(Matrix());
	else
	if (type == type_segment)
	{
		Segment res(value_a.get(Segment()));
		const Segment &b = value_b.get(Segment());
		res.p1 += b.p1;
		res.t1 += b.t1;
		res.p2 += b.p2;
		res.t2 += b.t2;
		return res;
	}
	else
	if (type == type_transformation)
		return Transformation(
			value_a.get(Transformation()).get_matrix()
			+ value_b.get(Transformation()).get_matrix() );
	else
	if (type == type_vector)
		return value_a.get(Vector()) + value_b.get(Vector());
	else
	if (type == type_width_point)
	{
		WidthPoint res(value_a.get(WidthPoint()));
		const WidthPoint &b = value_b.get(WidthPoint());
		res.set_width( res.get_width() + b.get_width() );
		return res;
	}

	return default_value;
}

ValueBase
ValueAverage::multiply(const ValueBase &value, Real amplifier)
{
	Type &type(value.get_type());
	if (type == type_real)
		return value.get(Real()) * amplifier;
	else
	if (type == type_bline_point)
	{
		BLinePoint res(value.get(BLinePoint()));
		res.set_vertex( res.get_vertex() * amplifier );
		res.set_tangent1( res.get_tangent1() * amplifier );
		res.set_tangent2( res.get_tangent2() * amplifier );
		return res;
	}
	else
	if (type == type_matrix)
		return value.get(Matrix()) * amplifier;
	else
	if (type == type_segment)
	{
		Segment res(value.get(Segment()));
		res.p1 *= amplifier;
		res.t1 *= amplifier;
		res.p2 *= amplifier;
		res.t2 *= amplifier;
		return res;
	}
	else
	if (type == type_transformation)
		return Transformation( value.get(Transformation()).get_matrix() * amplifier );
	else
	if (type == type_vector)
		return value.get(Vector()) * amplifier;
	else
	if (type == type_width_point)
	{
		WidthPoint res(value.get(WidthPoint()));
		res.set_width( res.get_width() * amplifier );
		return res;
	}

	return value;
}

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

