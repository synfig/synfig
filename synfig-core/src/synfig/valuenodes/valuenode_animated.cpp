/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.cpp
**	\brief Implementation of the "Animated" valuenode conversion.
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#include <vector>
#include <list>
#include <stdexcept>

#include <cmath>

#include <ETL/bezier>
#include <ETL/hermite>
#include <ETL/spline>
#include <ETL/handle>
#include <ETL/misc>

#include <algorithm>
#include <typeinfo>

#include <synfig/canvas.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include "valuenode_animated.h"
#include "valuenode_bone.h"
#include "valuenode_const.h"
#include <synfig/exception.h>
#include <synfig/gradient.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/*** DEBUG STUFF
template<typename T> String tangent_info(T a, T b, T v)
{
	return "...";
}

String tangent_info(Vector a, Vector b, Vector v)
{
	if(a==b)
		return strprintf("(should be zero) T=[%f,%f], Pp=[%f,%f], Pn=[%f,%f]",v[0],v[1],a[0],a[1],b[0],b[1]);
	else
		return strprintf("(should NOT be zero) T=[%f,%f], Pp=[%f,%f], Pn=[%f,%f]",v[0],v[1],a[0],a[1],b[0],b[1]);
}
*/

template <class T>
struct subtractor : public std::binary_function<T, T, T>
{
	T operator()(const T &a,const T &b)const
	{
		return a-b;
	}
};

template <>
struct subtractor<Angle> : public std::binary_function<Angle, Angle, Angle>
{
	Angle operator()(const Angle &a,const Angle &b)const
	{
		return a.dist(b);
	}
};

template <class T>
struct magnitude : public std::unary_function<float, T>
{
	float operator()(const T &a)const
	{
		return abs(a);
	}
};

template <>
struct magnitude<Angle> : public std::unary_function<float, Angle>
{
	float operator()(const Angle &a)const
	{
		return abs(Angle::rad(a).get());
	}
};

template <>
struct magnitude<Vector> : public std::unary_function<float, Vector>
{
	float operator()(const Vector &a)const
	{
		return a.mag();
	}
};

template <>
struct magnitude<Color> : public std::unary_function<float, Color>
{
	float operator()(const Color &a)const
	{
		return abs(a.get_y());
	}
};

template <>
struct magnitude<Gradient> : public std::unary_function<float, Gradient>
{
	float operator()(const Gradient &a)const
	{
		return a.mag();
	}
};

template <class T>
struct is_angle_type
{
	bool operator()()const
	{
		return false;
	}
};

#ifdef ANGLES_USE_LINEAR_INTERPOLATION
template <>
struct is_angle_type<Angle>
{
	bool operator()()const
	{
		return true;
	}
};
#endif	// ANGLES_USE_LINEAR_INTERPOLATION

template<class T>
T
clamped_tangent(T p1, T p2, T p3, Time t1, Time t2, Time t3)
{
	Real bias=0.0;
	T tangent(p3*0.0);
	T pm=p1+(p3-p1)*(t2-t1)/(t3-t1);
	if(p3 > p1)
	{
		if(p2 >= p3 || p2 <= p1)
			tangent = tangent*0.0;
		else
		{
			if(p2 > pm)
			{
				bias=(pm-p2)/(p3-pm);
			}
			else if (p2 < pm)
			{
				bias=(pm-p2)/(pm-p1);
			}
			else
				bias=0.0;
			tangent =( (p2-p1)*(1.0+bias)/2.0 + (p3-p2)*(1.0-bias)/2.0 );
		}
	}
	else if (p1 > p3)
	{
		if(p2 >= p1 || p2 <= p3)
			tangent = tangent*0.0;
		else
		{
			if(p2 > pm)
			{
				bias=(pm-p2)/(pm-p1);
			}
			else if (p2 < pm)
			{
				bias=(pm-p2)/(p3-pm);
			}
			else
				bias=0.0;
			tangent =( (p2-p1)*(1.0+bias)/2.0 + (p3-p2)*(1.0-bias)/2.0 );
		}
	}
	else
	{
		tangent= tangent * 0;
	}
	return tangent;
};

template<>
Vector
clamped_tangent<Vector>(Vector p1, Vector p2, Vector p3, Time t1, Time t2, Time t3)
{
	return Vector(clamped_tangent(p1[0],p2[0],p3[0],t1,t2,t3), clamped_tangent(p1[1],p2[1],p3[1],t1,t2,t3));
};

template<>
Angle
clamped_tangent<Angle>(Angle p1, Angle p2, Angle p3, Time t1, Time t2, Time t3)
{
	Real r1(Angle::rad(p1).get());
	Real r2(Angle::rad(p2).get());
	Real r3(Angle::rad(p3).get());
	return Angle::rad(clamped_tangent(r1, r2, r3, t1, t2, t3));
};

template<>
Time
clamped_tangent<Time>(Time p1, Time p2, Time p3, Time t1, Time t2, Time t3)
{
	return Time(clamped_tangent(double(p1), double(p2), double(p3), t1, t2, t3));
};

template<>
int
clamped_tangent<int>(int p1, int p2, int p3, Time t1, Time t2, Time t3)
{
	return int(clamped_tangent(Real(p1), Real(p2), Real(p3), t1, t2, t3));
};

template<>
Color
clamped_tangent<Color>(Color p1, Color p2, Color p3, Time t1, Time t2, Time t3)
{
	Color ret;
	ret.set_r(clamped_tangent(p1.get_r(), p2.get_r(), p3.get_r(), t1, t2, t3));
	ret.set_g(clamped_tangent(p1.get_g(), p2.get_g(), p3.get_g(), t1, t2, t3));
	ret.set_b(clamped_tangent(p1.get_b(), p2.get_b(), p3.get_b(), t1, t2, t3));
	ret.set_a(clamped_tangent(p1.get_a(), p2.get_a(), p3.get_a(), t1, t2, t3));
	return ret;
};

template<>
Gradient
clamped_tangent<Gradient>(Gradient p1, Gradient p2, Gradient p3, Time t1, Time t2, Time t3)
{
	Color c1, c2, c3;
	Gradient::CPoint cp;
	Gradient::const_iterator iter;
	Gradient ret;
	for(iter=p2.begin();iter!=p2.end();iter++)
	{
		cp=*iter;
		c1=p1(cp.pos);
		c2=cp.color;
		c3=p3(cp.pos);
		ret.push_back(Gradient::CPoint(cp.pos, clamped_tangent(c1, c2, c3, t1, t2, t3)));
	}
	return ret;
};

/* === G L O B A L S ======================================================= */

/* === C L A S S E S ======================================================= */

template<typename T>
class _Hermite : public synfig::ValueNode_Animated
{
public:
	typedef T value_type;
	affine_combo<value_type,Time> affine_combo_func;
	subtractor<value_type>	subtract_func;
	magnitude<value_type>	magnitude_func;
	is_angle_type<value_type>	is_angle;
private:
	struct PathSegment
	{
		is_angle_type<value_type>	is_angle;
		subtractor<value_type>	subtract_func;

		mutable hermite<Time,Time> first;
		mutable hermite<value_type,Time> second;
		WaypointList::iterator start;
		WaypointList::iterator end;

		value_type resolve(const Time &t)const
		{
			bool start_static(start->is_static());
			bool end_static(end->is_static());

			if(!start_static || !end_static)
			{
				//if(!start_static)
					second.p1()=start->get_value(t).get(value_type());
				if(start->get_after()==INTERPOLATION_CONSTANT || end->get_before()==INTERPOLATION_CONSTANT)
					return second.p1();
				//if(!end_static)
					second.p2()=end->get_value(t).get(value_type());

				// At the moment, the only type of non-constant interpolation
				// that we support is linear.
				second.t1()=
				second.t2()=subtract_func(second.p2(),second.p1());

				second.sync();
			}

			return second(first(t));
		}
	}; // END of struct PathSegment
	typedef vector<PathSegment> curve_list_type;

	curve_list_type curve_list;

	// Bounds of this curve
	Time r,s;

public:
	ValueNode::Handle clone(Canvas::LooseHandle canvas, const synfig::GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_Hermite<T>* ret(new _Hermite<T>());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(canvas, deriv_guid));
		ret->set_parent_canvas(canvas);
		return ret;
	}

	_Hermite()
	{
		set_type(ValueBase(value_type()).get_type());
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueBase value)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();

		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };
		Waypoint waypoint(value,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;

		if(is_angle())
		{
			ret->set_before(INTERPOLATION_LINEAR);
			ret->set_after(INTERPOLATION_LINEAR);
		}

		changed();

		return ret;
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();
		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };

		Waypoint waypoint(value_node,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;

		if(is_angle())
		{
			ret->set_before(INTERPOLATION_LINEAR);
			ret->set_after(INTERPOLATION_LINEAR);
		}

		changed();

		return ret;
	}

	virtual void on_changed()
	{
		if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
			printf("%s:%d _Hermite::on_changed()\n", __FILE__, __LINE__);

		ValueNode_Animated::on_changed();

		if(waypoint_list_.size()<=1)
			return;
		std::sort(waypoint_list_.begin(),waypoint_list_.end());
		//waypoint_list_.sort();

		r=waypoint_list_.front().get_time();
		s=waypoint_list_.back().get_time();

		curve_list.clear();

		WaypointList::iterator prev,iter,next=waypoint_list_.begin();
		int i=0;
		// The curve list must be calculated because we sorted the waypoints.
		for(iter=next++;iter!=waypoint_list_.end() && next!=waypoint_list_.end();prev=iter,iter=next++,i++)
		{
			typename curve_list_type::value_type curve;
			WaypointList::iterator after_next(next);
			++after_next;

			curve.start=iter;
			curve.end=next;

			// Set up the positions
			curve.first.set_rs(iter->get_time(), next->get_time());
			curve.second.set_rs(iter->get_time(), next->get_time());
			// Retrieve the interpolations
			Waypoint::Interpolation iter_get_after(iter->get_after());
			Waypoint::Interpolation next_get_after(next->get_after());
			Waypoint::Interpolation iter_get_before(iter->get_before());
			Waypoint::Interpolation next_get_before(next->get_before());

			if(is_angle())
			{
				if(iter_get_after==INTERPOLATION_TCB)
					iter_get_after=INTERPOLATION_LINEAR;
				if(next_get_after==INTERPOLATION_TCB)
					next_get_after=INTERPOLATION_LINEAR;
				if(iter_get_before==INTERPOLATION_TCB)
					iter_get_before=INTERPOLATION_LINEAR;
				if(next_get_before==INTERPOLATION_TCB)
					next_get_before=INTERPOLATION_LINEAR;
			}

			if(iter->is_static() && next->is_static())
			{
				curve.second.p1()=iter->get_value().get(T());
				curve.second.p2()=next->get_value().get(T());
				///
				/// ANY/CONSTANT ------ ANY/ANY
				///               or
				/// ANY/ANY-------------CONSTANT/ANY
				///
				if(iter_get_after==INTERPOLATION_CONSTANT || next_get_before==INTERPOLATION_CONSTANT)
				{
					// Sections must be constant on both sides.
					// NOTE: this is commented out because of some
					// user interface issues. Namely, if a section is
					// constant and the user turns off the constant on
					// one waypoint, this will end up turning it back on.
					// Confusing.
					//iter->get_after()=next->get_before()=INTERPOLATION_CONSTANT;
					curve.second.p1()=
					curve.second.p2()=iter->get_value().get(T());
					curve.second.t1()=
					curve.second.t2()=subtract_func(curve.second.p1(),curve.second.p2());
				}
				else
				{
					/// iter             next
					/// ANY/TCB -------- ANY/ANY and iter is middle waypoint
					///
				    if(iter_get_after==INTERPOLATION_TCB && iter!=waypoint_list_.begin() && !is_angle())
					{
						if(iter->get_before()!=INTERPOLATION_TCB && !curve_list.empty())
						{
							curve.second.t1()=curve_list.back().second.t2();
						}
						else
						{
							const Real& t(iter->get_tension());		// Tension
							const Real& c(iter->get_continuity());	// Continuity
							const Real& b(iter->get_bias());		// Bias
							// The following line works where the previous line fails.
							value_type Pp; Pp=curve_list.back().second.p1();	// P_{i-1}
							const value_type& Pc(curve.second.p1());	// P_i
							const value_type& Pn(curve.second.p2());	// P_{i+1}

							/// TCB calculation
							value_type vect(static_cast<value_type>
											(subtract_func(Pc,Pp) *
											           (((1.0-t) * (1.0+c) * (1.0+b)) / 2.0) +
											 (Pn-Pc) * (((1.0-t) * (1.0-c) * (1.0-b)) / 2.0)));
							curve.second.t1()=vect;
						}
					}
					else
					{
						///
						/// ANY/LINEAR ----- ANY/ANY
						///            or
						/// ANY/EASE ------- ANY/ANY
						///            or
						/// ANY/TCB -------- ANY/ANY and iter is first.
						///            or
						/// ANY/CLAMPED ---- ANT/ANY and iter is first
					    if(
						iter_get_after==INTERPOLATION_LINEAR || iter_get_after==INTERPOLATION_HALT ||
						(iter_get_after==INTERPOLATION_TCB && iter==waypoint_list_.begin()) ||
						(iter_get_after==INTERPOLATION_CLAMPED && iter==waypoint_list_.begin())
						)
						{
							/// t1 = p2 - p1
							curve.second.t1()=subtract_func(curve.second.p2(),curve.second.p1());
						}
					}
					/// iter             next
					/// ANY/CLAMPED ---- ANY/ANY and iter is middle waypoint
					if(iter_get_after == INTERPOLATION_CLAMPED && iter!=waypoint_list_.begin() && !is_angle())
					{
						value_type Pp; Pp=curve_list.back().second.p1(); // P_{i-1}
						const value_type& Pc(curve.second.p1());         // P_i
						const value_type& Pn(curve.second.p2());         // P_{i+1}
						Time T1(curve_list.back().first.p1());
						Time T2(iter->get_time());
						Time T3(next->get_time());
						value_type vect(clamped_tangent(Pp, Pc, Pn, T1, T2, T3));
						curve.second.t1()=vect;
					}
					///
					/// TCB/!TCB and list not empty
					///
					if(iter_get_before==INTERPOLATION_TCB && iter->get_after()!=INTERPOLATION_TCB && !curve_list.empty())
					{
						/// It means that there is one previous waypoint
						/// that is at cuerve_list.back()
						/// then its second tangent must be the same than
						/// our first one for continuity of the tangents.
						curve_list.back().second.t2()=curve.second.t1();
						curve_list.back().second.sync();
					}
					/// iter          next          after_next
					/// ANY/ANY ------TCB/ANY ----- ANY/ANY
					///
					if(next_get_before==INTERPOLATION_TCB && after_next!=waypoint_list_.end()  && !is_angle())
					{
						const Real &t(next->get_tension());       // Tension
						const Real &c(next->get_continuity());    // Continuity
						const Real &b(next->get_bias());          // Bias
						const value_type &Pp(curve.second.p1());  // P_{i-1}
						const value_type &Pc(curve.second.p2());  // P_i
						value_type Pn; Pn=after_next->get_value().get(T()); // P_{i+1}

						/// TCB calculation
						value_type vect(static_cast<value_type>(subtract_func(Pc,Pp) * (((1.0-t)*(1.0-c)*(1.0+b))/2.0) +
																			 (Pn-Pc) * (((1.0-t)*(1.0+c)*(1.0-b))/2.0)));
						curve.second.t2()=vect;
					}
					else
						/// iter          next
						/// ANY/ANY ----- LINEAR/ANY
						///           or
						/// ANY/ANY ----- EASE/ANY
						///           or
						/// ANY/ANY ----- TCB/ANY ---- END
						///           or
						/// ANY/ANY ----- CLAMPED/ANY ----END
					    if(
						next_get_before==INTERPOLATION_LINEAR || next_get_before==INTERPOLATION_HALT ||
						(next_get_before==INTERPOLATION_TCB && after_next==waypoint_list_.end()) ||
						(next_get_before==INTERPOLATION_CLAMPED && after_next==waypoint_list_.end())
						)
					{
						/// t2 = p2 - p1
						curve.second.t2()=subtract_func(curve.second.p2(),curve.second.p1());
					}
					/// iter             next         after_next
					/// ANY/ANY ---- CLAMPED/ANY      ANY/ANY
					if(next_get_before == INTERPOLATION_CLAMPED && after_next!=waypoint_list_.end()  && !is_angle())
					{
						const value_type &Pp(curve.second.p1());            // P_{i-1}
						const value_type &Pc(curve.second.p2());            // P_i
						value_type Pn; Pn=after_next->get_value().get(T()); // P_{i+1}
						Time T1(iter->get_time());
						Time T2(next->get_time());
						Time T3(after_next->get_time());
						value_type vect(clamped_tangent(Pp, Pc, Pn, T1, T2, T3));
						curve.second.t2()=vect;
					}
					// Adjust for time
					const float timeadjust(0.5);
					/// iter           next
					/// ANY/EASE ------ANY/ANY
					///
					if(iter_get_after==INTERPOLATION_HALT)
						curve.second.t1()*=0;
					// if this isn't the first curve
					else
					/// prev         iter              next
					/// ANY/ANY -----ANY/!LINEAR ----- ANY/ANY
					///
					if(iter_get_after != INTERPOLATION_LINEAR && !curve_list.empty())
						// adjust it for the curve that came before it
						curve.second.t1() = static_cast<T>(curve.second.t1() * // cast to prevent warning
							//                  (time span of this curve) * 1.5
							// -----------------------------------------------------------------
							// ((time span of this curve) * 0.5) + (time span of previous curve)
							  (curve.second.get_dt()*(timeadjust+1)) /
							  (curve.second.get_dt()*timeadjust + curve_list.back().second.get_dt()));
					/// iter           next
					/// ANY/ANY ------EASE/ANY
					///
					if(next_get_before==INTERPOLATION_HALT)
						curve.second.t2()*=0;
					// if this isn't the last curve
					else
					/// iter           next               after_next
					/// ANY/ANY ----- !LINEAR/ANY ------- ANY/ANY
					///
					if(next_get_before != INTERPOLATION_LINEAR && after_next!=waypoint_list_.end())
						// adjust it for the curve that came after it
						curve.second.t2() = static_cast<T>(curve.second.t2() * // cast to prevent warning
							//                (time span of this curve) * 1.5
							// -------------------------------------------------------------
							// ((time span of this curve) * 0.5) + (time span of next curve)
							  (curve.second.get_dt()*(timeadjust+1)) /
							  (curve.second.get_dt()*timeadjust+(after_next->get_time()-next->get_time())));
				} // not CONSTANT
			}

			// Set up the time to the default stuff
			curve.first.set_rs(iter->get_time(), next->get_time());
			curve.first.p1()=iter->get_time();
			curve.first.p2()=next->get_time();
			curve.first.t1()=(curve.first.p2()-curve.first.p1())*(1.0f-iter->get_temporal_tension());
			curve.first.t2()=(curve.first.p2()-curve.first.p1())*(1.0f-next->get_temporal_tension());


			curve.first.sync();
			curve.second.sync();

			curve_list.push_back(curve);
		}
	}

	virtual ValueBase operator()(Time t)const
	{
		if(waypoint_list_.empty())
			return value_type();	//! \todo Perhaps we should throw something here?
		if(waypoint_list_.size()==1)
			return waypoint_list_.front().get_value(t);
		if(t<=r)
			return waypoint_list_.front().get_value(t);
		if(t>=s)
			return waypoint_list_.back().get_value(t);

		typename curve_list_type::const_iterator iter;

		// This next line will set iter to the
		// correct iterator for the given time.
		for(iter=curve_list.begin();iter<curve_list.end() && t>=iter->first.get_s();++iter)
			continue;
		if(iter==curve_list.end())
			return waypoint_list_.back().get_value(t);
		return iter->resolve(t);
	}
}; // END of class _Hermite


template<typename T>
class _Constant : public synfig::ValueNode_Animated
{
public:
	typedef T value_type;

private:

	// Bounds of this curve
	Time r,s;

public:
	ValueNode::Handle clone(Canvas::LooseHandle canvas, const synfig::GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_Constant<T>* ret(new _Constant<T>());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(canvas, deriv_guid));
		ret->set_parent_canvas(canvas);
		return ret;
	}

	_Constant()
	{
		set_type(ValueBase(value_type()).get_type());
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueBase value)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();
		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };

		Waypoint waypoint(value,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;
		changed();

		return ret;
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();
		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };

		Waypoint waypoint(value_node,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;
		changed();

		return ret;
	}

	virtual void on_changed()
	{
		if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
			printf("%s:%d _Constant::on_changed()\n", __FILE__, __LINE__);

		ValueNode_Animated::on_changed();

		if(waypoint_list_.size()<=1)
			return;
		std::sort(waypoint_list_.begin(),waypoint_list_.end());
		//waypoint_list_.sort();
		r=waypoint_list_.front().get_time();
		s=waypoint_list_.back().get_time();

	}

	virtual ValueBase operator()(Time t)const
	{
		if(waypoint_list_.size()==1)
			return waypoint_list_.front().get_value(t);
		if(waypoint_list_.empty())
			return value_type();
		if(t<=r)
			return waypoint_list_.front().get_value(t);
		if(t>=s)
			return waypoint_list_.back().get_value(t);

		typename WaypointList::const_iterator iter;
		typename WaypointList::const_iterator next;

		// This next line will set iter to the
		// correct iterator for the given time.
		for(next=waypoint_list_.begin(),iter=next++;next!=waypoint_list_.end() && t>=next->get_time();iter=next++)
			continue;

		return iter->get_value(t);
	}
}; // END of class _Constant

class _AnimBool : public synfig::ValueNode_Animated
{
public:
	typedef bool value_type;

private:

	// Bounds of this curve
	Time r,s;

public:
	ValueNode::Handle clone(Canvas::LooseHandle canvas, const synfig::GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_AnimBool* ret(new _AnimBool());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(canvas, deriv_guid));
		ret->set_parent_canvas(canvas);
		return ret;
	}

	_AnimBool()
	{
		set_type(ValueBase(value_type()).get_type());
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueBase value)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();
		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };


		Waypoint waypoint(value,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;
		changed();

		return ret;
	}

	virtual WaypointList::iterator new_waypoint(Time t, ValueNode::Handle value_node)
	{
		// Make sure we are getting data of the correct type
		//if(data.type!=type)
		//	return waypoint_list_type::iterator();
		try { find(t); throw Exception::BadTime(_("A waypoint already exists at this point in time")); } catch(Exception::NotFound) { };

		Waypoint waypoint(value_node,t);
		waypoint.set_parent_value_node(this);

		waypoint_list_.push_back(waypoint);
		WaypointList::iterator ret=waypoint_list_.end();
		--ret;
		changed();

		return ret;
	}

	virtual void on_changed()
	{
		if (getenv("SYNFIG_DEBUG_ON_CHANGED"))
			printf("%s:%d _AnimBool::on_changed()\n", __FILE__, __LINE__);

		ValueNode_Animated::on_changed();

		if(waypoint_list_.size()<=1)
			return;
		std::sort(waypoint_list_.begin(),waypoint_list_.end());
		//waypoint_list_.sort();
		r=waypoint_list_.front().get_time();
		s=waypoint_list_.back().get_time();

	}

	virtual ValueBase operator()(Time t)const
	{
		if(waypoint_list_.size()==1)
			return waypoint_list_.front().get_value(t);
		if(waypoint_list_.empty())
			return false;
		if(t<r)
			return waypoint_list_.front().get_value(t);
		if(t>s)
			return waypoint_list_.back().get_value(t);

		WaypointList::const_iterator iter;
		WaypointList::const_iterator next;

		// This next line will set iter to the
		// correct iterator for the given time.
		for(next=waypoint_list_.begin(),iter=next++;next!=waypoint_list_.end() && t>=next->get_time();iter=next++)
			if(iter->get_time()==t)
				return iter->get_value(t);

		if(iter->get_time()==t)
			return iter->get_value(t);

		if(next!=waypoint_list_.end())
			return iter->get_value(t).get(bool()) || next->get_value(t).get(bool());
		return iter->get_value(t);
	}
}; // END of class _AnimBool

/* === M E T H O D S ======================================================= */

ValueNode_Animated::ValueNode_Animated()
{
	interpolation_=INTERPOLATION_UNDEFINED;
}

int
ValueNode_Animated::find(const Time& begin,const Time& end,std::vector<Waypoint*>& selected)
{
	Time curr_time(begin);
	int ret(0);

	// try to grab first waypoint
	try
	{
		WaypointList::iterator iter;
		iter=find(curr_time);
		selected.push_back(&*iter);
		ret++;
	}
	catch(...) { }

	try
	{
		WaypointList::iterator iter;
		while(true)
		{
			iter=find_next(curr_time);
			curr_time=iter->get_time();
			if(curr_time>=end)
				break;
			selected.push_back(&*iter);
			ret++;
		}
	}
	catch(...) { }

	return ret;
}

/** This code seems to be designed to move selected waypoints to other
 * place. Probably it was commented out because the conflict when other
 * waypoints are found in the target time. It was also defined and now commented
 * in valuenode_dynamiclist.h
void
ValueNode_Animated::manipulate_time(const Time& old_begin,const Time& old_end,const Time& new_begin,const Time& new_end)
{
#define old_2_new(x)	(((x)-old_begin)/(old_end-old_begin)*(new_end-new_begin)+new_begin)
	std::vector<Waypoint*> selected;
	std::vector<Waypoint*>::iterator iter;

	if(find(old_begin,old_end,selected))
	{
		// check to make sure this operation is OK
		for(iter=selected.begin();iter!=selected.end();++iter)
		{
			try
			{
				Time new_time(old_2_new((*iter)->get_time()));
				if(new_time>=old_begin && new_time<old_end)
					continue;
				find(new_time);
				// If we found a waypoint already at that time, then
				// we need to abort
				throw Exception::BadTime(_("Waypoint Conflict"));
			}
			catch(Exception::NotFound) { }

			selected.back()->set_time(old_2_new(selected.back()->get_time()));
			selected.pop_back();
		}


		while(!selected.empty())
		{
			selected.back()->set_time(old_2_new(selected.back()->get_time()));
			selected.pop_back();
		}

		changed();
	}
#undef old_2_new
}
*/

Waypoint
ValueNode_Animated::new_waypoint_at_time(const Time& time)const
{
	Waypoint waypoint;
	try
	{
		// Trivial case, we are sitting on a waypoint
		waypoint=*find(time);
		waypoint.make_unique();
	}
	catch(...)
	{
		if(waypoint_list().empty())
		{
			waypoint.set_value((*this)(time));
		}
		else
		{
			WaypointList::const_iterator prev;
			WaypointList::const_iterator next;

			bool has_prev(false), has_next(false);

			try { prev=find_prev(time); has_prev=true; } catch(...) { }
			try { next=find_next(time); has_next=true; } catch(...) { }

			if(has_prev && !prev->is_static())
				waypoint.set_value_node(prev->get_value_node());
			if(has_next && !next->is_static())
				waypoint.set_value_node(next->get_value_node());
			else
				waypoint.set_value((*this)(time));

		}
	}
	waypoint.set_time(time);
	waypoint.set_parent_value_node(const_cast<ValueNode_Animated*>(this));
//	synfig::info("waypoint.get_after()=set to %d",waypoint.get_after());
//	synfig::info("waypoint.get_before()=set to %d",waypoint.get_before());

	return waypoint;
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find(const UniqueID &x)
{
	ValueNode_Animated::WaypointList::iterator iter;
	iter=std::find(editable_waypoint_list().begin(),editable_waypoint_list().end(),x);
	if(iter==editable_waypoint_list().end() || iter->get_uid()!=x.get_uid())
		throw Exception::NotFound(strprintf("ValueNode_Animated::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find(const UniqueID &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find(x);
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find(const Time &x)
{
	WaypointList::iterator iter(binary_find(editable_waypoint_list().begin(),editable_waypoint_list().end(),x));

	if(iter!=editable_waypoint_list().end() && x.is_equal(iter->get_time()))
		return iter;

	throw Exception::NotFound(strprintf("ValueNode_Animated::find(): Can't find Waypoint at %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find(x);
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find_next(const Time &x)
{
	WaypointList::iterator iter(binary_find(editable_waypoint_list().begin(),editable_waypoint_list().end(),x));

	if(iter!=editable_waypoint_list().end())
	{
		if(iter->get_time().is_more_than(x))
			return iter;
		++iter;
		if(iter!=editable_waypoint_list().end() && iter->get_time().is_more_than(x))
			return iter;
	}

	throw Exception::NotFound(strprintf("ValueNode_Animated::find_next(): Can't find Waypoint after %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find_next(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find_next(x);
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find_prev(const Time &x)
{
	WaypointList::iterator iter(binary_find(editable_waypoint_list().begin(),editable_waypoint_list().end(),x));

	if(iter!=editable_waypoint_list().end())
	{
		if(iter->get_time().is_less_than(x))
			return iter;
		if(iter!=editable_waypoint_list().begin() && (--iter)->get_time().is_less_than(x))
			return iter;
	}

	throw Exception::NotFound(strprintf("ValueNode_Animated::find_prev(): Can't find Waypoint after %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find_prev(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find_prev(x);
}

bool
ValueNode_Animated::waypoint_is_only_use_of_valuenode(Waypoint &waypoint)
{
	ValueNode::Handle value_node(waypoint.get_value_node());
	assert(value_node);
	WaypointList wp_list(waypoint_list());
	WaypointList::iterator iter;
	for (iter = wp_list.begin(); iter != wp_list.end(); iter++)
		if (*iter == waypoint)
			continue;
		else if (iter->get_value_node() == value_node)
			return false;
	return true;
}

void
ValueNode_Animated::erase(const UniqueID &x)
{
	printf("%s:%d erasing waypoint from %lx\n", __FILE__, __LINE__, uintptr_t(this));
	WaypointList::iterator iter(find(x));
	Waypoint waypoint(*iter);
	assert(waypoint.get_value_node());
	editable_waypoint_list().erase(iter);
	if (waypoint_is_only_use_of_valuenode(waypoint))
		remove_child(waypoint.get_value_node().get());
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::add(const Waypoint &x)
{
	Waypoint waypoint(x);
	waypoint.set_parent_value_node(this);
	waypoint_list_.push_back(waypoint);
	//assert(waypoint_list_.back().get_parent_value_node()==this);
	WaypointList::iterator ret=waypoint_list_.end();
	--ret;
	changed();
	return ret;
}

void
ValueNode_Animated::set_type(Type &t)
{
	ValueNode::set_type(t);
}

ValueNode_Animated::Handle
synfig::ValueNode_Animated::create(Type &type)
{
	if (type == type_time)
		return ValueNode_Animated::Handle(new _Hermite<Time>);
	if (type == type_real)
		return ValueNode_Animated::Handle(new _Hermite<Vector::value_type>);
	if (type == type_integer)
		return ValueNode_Animated::Handle(new _Hermite<int>);
	if (type == type_angle)
		return ValueNode_Animated::Handle(new _Hermite<Angle>);
	if (type == type_vector)
		return ValueNode_Animated::Handle(new _Hermite<Vector>);
	if (type == type_color)
		return ValueNode_Animated::Handle(new _Hermite<Color>);
	if (type == type_string)
		return ValueNode_Animated::Handle(new _Constant<String>);
	if (type == type_bone_valuenode)
		return ValueNode_Animated::Handle(new _Constant<ValueNode_Bone::LooseHandle>);
	if (type == type_bone_object)
		return ValueNode_Animated::Handle(new _Constant<Bone>);
	if (type == type_gradient)
		return ValueNode_Animated::Handle(new _Hermite<Gradient>);
	if (type == type_bool)
		return ValueNode_Animated::Handle(new _AnimBool);
	if (type == type_canvas)
		return ValueNode_Animated::Handle(new _Constant<Canvas::LooseHandle>);

	throw
		Exception::BadType(strprintf(_("%s: You cannot use a %s in an animated ValueNode"),"synfig::ValueNode_Animated::create()",
			type.description.local_name.c_str())
		);
	return ValueNode_Animated::Handle();
}

ValueNode_Animated::Handle
ValueNode_Animated::create(const ValueBase& value, const Time& time)
{
	return create(ValueNode::Handle(ValueNode_Const::create(value)),time);
}

ValueNode_Animated::Handle
ValueNode_Animated::create(ValueNode::Handle value_node, const Time& time)
{
	ValueNode_Animated::Handle ret(create(value_node->get_type()));
	ret->new_waypoint(time,value_node);
	return ret;
}

ValueNode_Animated::~ValueNode_Animated()
{
}

String
ValueNode_Animated::get_name()const
{
	return "animated";
}

String
ValueNode_Animated::get_local_name()const
{
	return _("Animated");
}

void ValueNode_Animated::get_times_vfunc(Node::time_set &set) const
{
	//add all the way point times to the value node...

	WaypointList::const_iterator 	i = waypoint_list().begin(),
									end = waypoint_list().end();

	for(; i != end; ++i)
	{
		TimePoint t;
		t.set_time(i->get_time());
		t.set_before(i->get_before());
		t.set_after(i->get_after());
		t.set_guid(i->get_guid());
		set.insert(t);
	}
}
struct timecmp
 {
 	Time t;

 	timecmp(const Time &c) :t(c) {}

 	bool operator()(const Waypoint &rhs) const
 	{
 		return t.is_equal(rhs.get_time());
 	}
 };

 ValueNode_Animated::findresult
 ValueNode_Animated::find_uid(const UniqueID &x)
 {
 	findresult	f;
 	f.second = false;

 	//search for it... and set the bool part of the return value to true if we found it!
 	f.first = std::find(waypoint_list_.begin(), waypoint_list_.end(), x);
 	if(f.first != waypoint_list_.end())
 		f.second = true;

 	return f;
 }

 ValueNode_Animated::const_findresult
 ValueNode_Animated::find_uid(const UniqueID &x)const
 {
 	const_findresult	f;
 	f.second = false;

 	//search for it... and set the bool part of the return value to true if we found it!
 	f.first = std::find(waypoint_list_.begin(), waypoint_list_.end(), x);
 	if(f.first != waypoint_list_.end())
 		f.second = true;

 	return f;
 }

 ValueNode_Animated::findresult
 ValueNode_Animated::find_time(const Time &x)
 {
 	findresult	f;
 	f.second = false;

 	//search for it... and set the bool part of the return value to true if we found it!
 	f.first = std::find_if(waypoint_list_.begin(), waypoint_list_.end(), timecmp(x));
 	if(f.first != waypoint_list_.end())
 		f.second = true;

 	return f;
 }

ValueNode_Animated::const_findresult
ValueNode_Animated::find_time(const Time &x)const
{
 	const_findresult	f;
 	f.second = false;

 	//search for it... and set the bool part of the return value to true if we found it!
 	f.first = std::find_if(waypoint_list_.begin(), waypoint_list_.end(), timecmp(x));
 	if(f.first != waypoint_list_.end())
 		f.second = true;

 	return f;
}

String
ValueNode_Animated::get_string()const
{
	return "ValueNode_Animated";
}

void
ValueNode_Animated::insert_time(const Time& location, const Time& delta)
{
	if(!delta)
		return;
	try
	{
		WaypointList::iterator iter(find_next(location));
		for(;iter!=waypoint_list().end();++iter)
		{
			iter->set_time(iter->get_time()+delta);
		}
		changed();
	}
	catch(Exception::NotFound) { }
}
