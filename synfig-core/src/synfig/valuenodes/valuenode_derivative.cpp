/* === S Y N F I G ========================================================= */
/*!	\file valuenode_derivative.cpp
**	\brief Implementation of the "Derivative" valuenode conversion.
**
**	\legal
**	Copyright (c) 2014 Carlos López
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

#include "valuenode_derivative.h"
#include "valuenode_const.h"
#include <synfig/general.h>
#include <synfig/localization.h>
#include <synfig/valuenode_registry.h>
#include <synfig/vector.h>

#include <ETL/misc>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */
// E= EVALUATE
#define E(x,t,y) ( (*x)(t).get(y) )
// D= DERIVATIVE
// See http://en.wikipedia.org/wiki/Finite_difference_coefficients#Central_finite_difference
// First Derivatives
#define D_ROUGH(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-h,y)*(-1.0/2.0)+      \
				E(x,t+h,y)*( 1.0/2.0)       \
				)/(h)                       \
				)
#define D_NORMAL(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-2*h,y)*( 1.0/12.0)+   \
				E(x,t-h  ,y)*( -2.0/3.0)+   \
				E(x,t+h  ,y)*(  2.0/3.0)+   \
				E(x,t+2*h,y)*(-1.0/12.0)    \
				)/(h)                       \
				)
#define D_FINE(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-3*h,y)*(-1.0/60.0)+   \
				E(x,t-2*h,y)*( 3.0/20.0)+   \
				E(x,t-h  ,y)*( -3.0/4.0)+   \
				E(x,t+h  ,y)*(  3.0/4.0)+   \
				E(x,t+2*h,y)*(-3.0/20.0)+   \
				E(x,t+3*h,y)*( 1.0/60.0)    \
				)/(h)                       \
				)
#define D_EXTREME(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-4*h,y)*( 1.0/280.0)+  \
				E(x,t-3*h,y)*(  -4.0/105)+  \
				E(x,t-2*h,y)*(   1.0/5.0)+  \
				E(x,t-h  ,y)*(  -4.0/5.0)+  \
				E(x,t+h  ,y)*(   4.0/5.0)+  \
				E(x,t+2*h,y)*(  -1.0/5.0)+  \
				E(x,t+3*h,y)*( 4.0/105.0)+  \
				E(x,t+4*h,y)*(-1.0/280.0)   \
				)/(h)                       \
				)

// Second Derivatives
#define DD_ROUGH(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-h,y)*( 1.0)+          \
				E(x,t,y  )*(-2.0)+          \
				E(x,t+h,y)*( 1.0)           \
				)/(h*h)                     \
				)
#define DD_NORMAL(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-2*h,y)*(-1.0/12.0)+   \
				E(x,t-h  ,y)*(  4.0/3.0)+   \
				E(x,t    ,y)*( -5.0/2.0)+   \
				E(x,t+h  ,y)*(  4.0/3.0)+   \
				E(x,t+2*h,y)*(-1.0/12.0)    \
				)/(h*h)                     \
				)
#define DD_FINE(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-3*h,y)*(  1.0/90.0)+  \
				E(x,t-2*h,y)*( -3.0/20.0)+  \
				E(x,t-h  ,y)*(   3.0/2.0)+  \
				E(x,t    ,y)*(-49.0/18.0)+  \
				E(x,t+h  ,y)*(   3.0/2.0)+  \
				E(x,t+2*h,y)*( -3.0/20.0)+  \
				E(x,t+3*h,y)*(  1.0/90.0)   \
				)/(h*h)                     \
				)
#define DD_EXTREME(x,t,h,y)\
				(                           \
				(                           \
				E(x,t-4*h,y)*( -1.0/560.0)+ \
				E(x,t-3*h,y)*(  8.0/315.0)+ \
				E(x,t-2*h,y)*(   -1.0/5.0)+ \
				E(x,t-h  ,y)*(    8.0/5.0)+ \
				E(x,t    ,y)*(-205.0/72.0)+ \
				E(x,t+h  ,y)*(    8.0/5.0)+ \
				E(x,t+2*h,y)*(   -1.0/5.0)+ \
				E(x,t+3*h,y)*(  8.0/315.0)+ \
				E(x,t+4*h,y)*( -1.0/560.0)  \
				)/(h*h)                     \
				)
/* === G L O B A L S ======================================================= */

REGISTER_VALUENODE(ValueNode_Derivative, RELEASE_VERSION_1_0, "derivative", "Derivative")

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ValueNode_Derivative::ValueNode_Derivative(const ValueBase &value):
	LinkableValueNode(value.get_type())
{
	Vocab ret(get_children_vocab());
	set_children_vocab(ret);
	set_link("interval",      ValueNode_Const::create(Real(0.01))); // Default interval
	set_link("accuracy",      ValueNode_Const::create((int)(NORMAL)));
	set_link("order",         ValueNode_Const::create((int)(FIRST)));

	Type &type(get_type());
	if (type == type_real)
		set_link("link",ValueNode_Const::create(value.get(Real())));
	else
	if (type == type_time)
		set_link("link",ValueNode_Const::create(value.get(Time())));
	else
	if (type == type_angle)
		set_link("link",ValueNode_Const::create(value.get(Angle())));
	else
	if (type == type_vector)
		set_link("link",ValueNode_Const::create(value.get(Vector())));
	else
		throw Exception::BadType(type.description.local_name);
}

LinkableValueNode*
ValueNode_Derivative::create_new()const
{
	return new ValueNode_Derivative(get_type());
}

ValueNode_Derivative*
ValueNode_Derivative::create(const ValueBase &x)
{
	return new ValueNode_Derivative(x);
}

ValueNode_Derivative::~ValueNode_Derivative()
{
	unlink_all();
}

ValueBase
ValueNode_Derivative::operator()(Time t)const
{
	if (getenv("SYNFIG_DEBUG_VALUENODE_OPERATORS"))
		printf("%s:%d operator()\n", __FILE__, __LINE__);

	Type &type(get_type());
	if (type == type_real)
	{
		switch((*accuracy_)(t).get(int()))
		{
		case ROUGH:
			return (*order_)(t).get(int())?
					DD_ROUGH(link_,t,(*interval_)(t).get(Real()),Real()):
					D_ROUGH(link_,t,(*interval_)(t).get(Real()),Real());
			break;
		case FINE:
			return (*order_)(t).get(int())?
					DD_FINE(link_,t,(*interval_)(t).get(Real()),Real()):
					D_FINE(link_,t,(*interval_)(t).get(Real()),Real());
			break;
		case EXTREME:
			return (*order_)(t).get(int())?
					DD_EXTREME(link_,t,(*interval_)(t).get(Real()),Real()):
					D_EXTREME(link_,t,(*interval_)(t).get(Real()),Real());
			break;
		case NORMAL:
		default:
			return (*order_)(t).get(int())?
					DD_NORMAL(link_,t,(*interval_)(t).get(Real()),Real()):
					D_NORMAL(link_,t,(*interval_)(t).get(Real()),Real());
		break;
		}
	}
	else
	if (type == type_time)
	{
		switch((*accuracy_)(t).get(int()))
		{
		case ROUGH:
			return (*order_)(t).get(int())?
					DD_ROUGH(link_,t,(*interval_)(t).get(Real()),Time()):
					D_ROUGH(link_,t,(*interval_)(t).get(Real()),Time());
			break;
		case FINE:
			return (*order_)(t).get(int())?
					DD_FINE(link_,t,(*interval_)(t).get(Real()),Time()):
					D_FINE(link_,t,(*interval_)(t).get(Real()),Time());
			break;
		case EXTREME:
			return (*order_)(t).get(int())?
					DD_EXTREME(link_,t,(*interval_)(t).get(Real()),Time()):
					D_EXTREME(link_,t,(*interval_)(t).get(Real()),Time());
			break;
		case NORMAL:
		default:
			return (*order_)(t).get(int())?
					DD_NORMAL(link_,t,(*interval_)(t).get(Real()),Time()):
					D_NORMAL(link_,t,(*interval_)(t).get(Real()),Time());
		break;
		}
	}
	else
	if (type == type_angle)
	{
		switch((*accuracy_)(t).get(int()))
		{
		case ROUGH:
			return (*order_)(t).get(int())?
					DD_ROUGH(link_,t,(*interval_)(t).get(Real()),Angle()):
					D_ROUGH(link_,t,(*interval_)(t).get(Real()),Angle());
			break;
		case FINE:
			return (*order_)(t).get(int())?
					DD_FINE(link_,t,(*interval_)(t).get(Real()),Angle()):
					D_FINE(link_,t,(*interval_)(t).get(Real()),Angle());
			break;
		case EXTREME:
			return (*order_)(t).get(int())?
					DD_EXTREME(link_,t,(*interval_)(t).get(Real()),Angle()):
					D_EXTREME(link_,t,(*interval_)(t).get(Real()),Angle());
			break;
		case NORMAL:
		default:
			return (*order_)(t).get(int())?
					DD_NORMAL(link_,t,(*interval_)(t).get(Real()),Angle()):
					D_NORMAL(link_,t,(*interval_)(t).get(Real()),Angle());
		break;
		}
	}
	else
	if (type == type_vector)
	{
		switch((*accuracy_)(t).get(int()))
		{
		case ROUGH:
			return (*order_)(t).get(int())?
					DD_ROUGH(link_,t,(*interval_)(t).get(Real()),Vector()):
					D_ROUGH(link_,t,(*interval_)(t).get(Real()),Vector());
			break;
		case FINE:
			return (*order_)(t).get(int())?
					DD_FINE(link_,t,(*interval_)(t).get(Real()),Vector()):
					D_FINE(link_,t,(*interval_)(t).get(Real()),Vector());
			break;
		case EXTREME:
			return (*order_)(t).get(int())?
					DD_EXTREME(link_,t,(*interval_)(t).get(Real()),Vector()):
					D_EXTREME(link_,t,(*interval_)(t).get(Real()),Vector());
			break;
		case NORMAL:
		default:
			return (*order_)(t).get(int())?
					DD_NORMAL(link_,t,(*interval_)(t).get(Real()),Vector()):
					D_NORMAL(link_,t,(*interval_)(t).get(Real()),Vector());
		break;
		}
	}
	return ValueBase();
}



bool
ValueNode_Derivative::check_type(Type &type)
{
	return
		type==type_real ||
		type==type_time ||
		type==type_angle ||
		type==type_vector;
}

bool
ValueNode_Derivative::set_link_vfunc(int i,ValueNode::Handle value)
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: CHECK_TYPE_AND_SET_VALUE(link_,    get_type());
	case 1: CHECK_TYPE_AND_SET_VALUE(interval_,type_real);
	case 2: CHECK_TYPE_AND_SET_VALUE(accuracy_,type_integer);
	case 3: CHECK_TYPE_AND_SET_VALUE(order_,type_integer);
	}
	return false;
}

ValueNode::LooseHandle
ValueNode_Derivative::get_link_vfunc(int i)const
{
	assert(i>=0 && i<link_count());

	switch(i)
	{
	case 0: return link_;
	case 1: return interval_;
	case 2: return accuracy_;
	case 3: return order_;
	default:
		return 0;
	}
}

LinkableValueNode::Vocab
ValueNode_Derivative::get_children_vocab_vfunc()const
{
	if(children_vocab.size())
		return children_vocab;

	LinkableValueNode::Vocab ret;
	ret.push_back(ParamDesc(ValueBase(),"link")
		.set_local_name(_("Link"))
		.set_description(_("Value to calculate the derivative"))
	);
	ret.push_back(ParamDesc(ValueBase(),"interval")
		.set_local_name(_("Interval"))
		.set_description(_("Interval of time to calculate the finite differences"))
	);
	ret.push_back(ParamDesc(ValueBase(),"accuracy")
		.set_local_name(_("Accuracy"))
		.set_description(_("Accuracy of the derivative"))
		.set_hint("enum")
		.add_enum_value(ROUGH,"rough",_("Rough"))
		.add_enum_value(NORMAL,"normal",_("Normal"))
		.add_enum_value(FINE,"fine",_("Fine"))
		.add_enum_value(EXTREME,"extreme",_("Extreme"))
	);
	ret.push_back(ParamDesc(ValueBase(),"order")
		.set_local_name(_("Order"))
		.set_description(_("Order of the derivative"))
		.set_hint("enum")
		.add_enum_value(FIRST,"first",_("First Derivative"))
		.add_enum_value(SECOND,"second",_("Second Derivative"))
	);
	return ret;
}
