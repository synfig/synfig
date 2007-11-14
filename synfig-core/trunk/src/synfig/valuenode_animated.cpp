/* === S Y N F I G ========================================================= */
/*!	\file valuenode_animated.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "canvas.h"
#include "general.h"
#include "valuenode_animated.h"
#include "valuenode_const.h"
#include "exception.h"
#include "gradient.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

// Fast binary search implementation
/*
template<typename I, typename T> inline I
binary_find(I begin, I end, const T& value)
{
	I iter(begin+(end-begin)/2);

	while(end-begin>1 && !(*iter==value))
	{
		((*iter<value)?begin:end) = iter;

		iter = begin+(end-begin)/2;
	}
	return iter;
}
*/

/*
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
	};
	typedef vector <
		PathSegment
		/*
		pair <
			hermite<Time,Time>,
			hermite<value_type,Time>
		>
		*/
	> curve_list_type;

	curve_list_type curve_list;

	// Bounds of this curve
	Time r,s;

public:
	ValueNode* clone(const GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_Hermite<T>* ret(new _Hermite<T>());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(deriv_guid));
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

							// TCB
							value_type vect(static_cast<value_type>
											(subtract_func(Pc,Pp) *
											           (((1.0-t) * (1.0+c) * (1.0+b)) / 2.0) +
											 (Pn-Pc) * (((1.0-t) * (1.0-c) * (1.0-b)) / 2.0)));

							// Tension Only
							//value_type vect=(value_type)((Pn-Pp)*(1.0-t));

							// Linear
							//value_type vect=(value_type)(Pn-Pc);

							// Debugging stuff
							//synfig::info("%d:t1: %s",i,tangent_info(Pp,Pn,vect).c_str());

							// Adjust for time
							//vect=value_type(vect*(curve.second.get_dt()*2.0)/(curve.second.get_dt()+curve_list.back().second.get_dt()));
							//vect=value_type(vect*(curve.second.get_dt())/(curve_list.back().second.get_dt()));

							curve.second.t1()=vect;
						}
					}
					else if(
						iter_get_after==INTERPOLATION_LINEAR || iter_get_after==INTERPOLATION_HALT ||
						(iter_get_after==INTERPOLATION_TCB && iter==waypoint_list_.begin()))
					{
						curve.second.t1()=subtract_func(curve.second.p2(),curve.second.p1());
					}

					if(iter_get_before==INTERPOLATION_TCB && iter->get_after()!=INTERPOLATION_TCB && !curve_list.empty())
					{
						curve_list.back().second.t2()=curve.second.t1();
						curve_list.back().second.sync();
					}


					if(next_get_before==INTERPOLATION_TCB && after_next!=waypoint_list_.end()  && !is_angle())
					{
						const Real &t(next->get_tension());		// Tension
						const Real &c(next->get_continuity());	// Continuity
						const Real &b(next->get_bias());			// Bias
						const value_type &Pp(curve.second.p1());	// P_{i-1}
						const value_type &Pc(curve.second.p2());	// P_i
						value_type Pn; Pn=after_next->get_value().get(T());	// P_{i+1}

						// TCB
						value_type vect(static_cast<value_type>(subtract_func(Pc,Pp) * (((1.0-t)*(1.0-c)*(1.0+b))/2.0) +
																			 (Pn-Pc) * (((1.0-t)*(1.0+c)*(1.0-b))/2.0)));

						// Tension Only
						//value_type vect((value_type)((Pn-Pp)*(1.0-t)));

						// Linear
						//value_type vect=(value_type)(Pc-Pp);

						// Debugging stuff
						//synfig::info("%d:t2: %s",i,tangent_info(Pp,Pn,vect).c_str());

						// Adjust for time
						//vect=value_type(vect*(curve.second.get_dt()*2.0)/(curve.second.get_dt()+(after_next->get_time()-next->get_time())));
						//vect=value_type(vect*(curve.second.get_dt()/((after_next->get_time()-next->get_time()))));

						curve.second.t2()=vect;
					}
					else if(
						next_get_before==INTERPOLATION_LINEAR || next_get_before==INTERPOLATION_HALT ||
						(next_get_before==INTERPOLATION_TCB && after_next==waypoint_list_.end()))
					{
						curve.second.t2()=subtract_func(curve.second.p2(),curve.second.p1());
					}

					// Adjust for time
					const float timeadjust(0.5);

					if(iter_get_after==INTERPOLATION_HALT)
						curve.second.t1()*=0;
					// if this isn't the first curve
					else if(iter_get_after != INTERPOLATION_LINEAR && !curve_list.empty())
						// adjust it for the curve that came before it
						curve.second.t1() = static_cast<T>(curve.second.t1() * // cast to prevent warning
							//                  (time span of this curve) * 1.5
							// -----------------------------------------------------------------
							// ((time span of this curve) * 0.5) + (time span of previous curve)
							  (curve.second.get_dt()*(timeadjust+1)) /
							  (curve.second.get_dt()*timeadjust + curve_list.back().second.get_dt()));

					if(next_get_before==INTERPOLATION_HALT)
						curve.second.t2()*=0;
					// if this isn't the last curve
					else if(next_get_before != INTERPOLATION_LINEAR && after_next!=waypoint_list_.end())
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
			curve.first.t1()=(curve.first.p2()-curve.first.p1())*(1.0f-iter->get_time_tension());
			curve.first.t2()=(curve.first.p2()-curve.first.p1())*(1.0f-next->get_time_tension());


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
};


template<typename T>
class _Constant : public synfig::ValueNode_Animated
{
public:
	typedef T value_type;

private:

	// Bounds of this curve
	Time r,s;

public:
	ValueNode* clone(const GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_Constant<T>* ret(new _Constant<T>());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(deriv_guid));
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
};

class _AnimBool : public synfig::ValueNode_Animated
{
public:
	typedef bool value_type;

private:

	// Bounds of this curve
	Time r,s;

public:
	ValueNode* clone(const GUID& deriv_guid)const
	{
		{ ValueNode* x(find_value_node(get_guid()^deriv_guid).get()); if(x)return x; }
		_AnimBool* ret(new _AnimBool());
		ret->set_guid(get_guid()^deriv_guid);
		for(WaypointList::const_iterator iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			ret->add(iter->clone(deriv_guid));
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
};

/* === M E T H O D S ======================================================= */

ValueNode_Animated::ValueNode_Animated()
{
	DCAST_HACK_ENABLE();
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

/*
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

			/*
			WaypointList::const_iterator closest;

			if(has_prev&&!has_next)
				closest=prev;
			else if(has_next&&!has_prev)
				closest=next;
			else if(time-prev->get_time()<next->get_time()-time)
				closest=prev;
			else
				closest=next;

			for(iter=waypoint_list().begin();iter!=waypoint_list().end();++iter)
			{
				const Real dist(abs(iter->get_time()-time));
				if(dist<abs(closest->get_time()-time))
					closest=iter;
			}
			*/

			if(has_prev && !prev->is_static())
				waypoint.set_value_node(prev->get_value_node());
			if(has_next && !next->is_static())
				waypoint.set_value_node(next->get_value_node());
			else
				waypoint.set_value((*this)(time));

			/*if(has_prev)
				waypoint.set_after(prev->get_before());
			if(has_next)
				waypoint.set_before(next->get_after());
			*/
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
	iter=std::find(waypoint_list().begin(),waypoint_list().end(),x);
	if(iter==waypoint_list().end() || iter->get_uid()!=x.get_uid())
		throw Exception::NotFound(strprintf("ValueNode_Animated::find(): Can't find UniqueID %d",x.get_uid()));
	return iter;
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find(const UniqueID &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find(x);
	/*
	ValueNode_Animated::WaypointList::const_iterator iter;
	iter=std::find(waypoint_list().begin(),waypoint_list().end(),x);
	if(iter!=waypoint_list().end() && iter->get_uid()!=x.get_uid())
		throw Exception::NotFound(strprintf("ValueNode_Animated::find()const: Can't find UniqueID %d",x.get_uid()));
	return iter;
	*/
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find(const Time &x)
{
	WaypointList::iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end() && x.is_equal(iter->get_time()))
		return iter;

	throw Exception::NotFound(strprintf("ValueNode_Animated::find(): Can't find Waypoint at %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find(x);
	/*
	WaypointList::const_iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end() && x.is_equal(iter->get_time()))
		return iter;

	throw Exception::NotFound(strprintf("ValueNode_Animated::find(): Can't find Waypoint at %s",x.get_string().c_str()));
	*/
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find_next(const Time &x)
{
	WaypointList::iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end())
	{
		if(iter->get_time().is_more_than(x))
			return iter;
		++iter;
		if(iter!=waypoint_list().end() && iter->get_time().is_more_than(x))
			return iter;
	}

	throw Exception::NotFound(strprintf("ValueNode_Animated::find_next(): Can't find Waypoint after %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find_next(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find_next(x);
	/*
	WaypointList::const_iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end())
	{
		if(iter->get_time()-Time::epsilon()>x)
			return iter;
		++iter;
		if(iter!=waypoint_list().end() && iter->get_time()-Time::epsilon()>x)
			return iter;
	}

	throw Exception::NotFound(strprintf("ValueNode_Animated::find_next(): Can't find Waypoint after %s",x.get_string().c_str()));
*/
}

ValueNode_Animated::WaypointList::iterator
ValueNode_Animated::find_prev(const Time &x)
{
	WaypointList::iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end())
	{
		if(iter->get_time().is_less_than(x))
			return iter;
		if(iter!=waypoint_list().begin() && (--iter)->get_time().is_less_than(x))
			return iter;
	}

	throw Exception::NotFound(strprintf("ValueNode_Animated::find_prev(): Can't find Waypoint after %s",x.get_string().c_str()));
}

ValueNode_Animated::WaypointList::const_iterator
ValueNode_Animated::find_prev(const Time &x)const
{
	return const_cast<ValueNode_Animated*>(this)->find_prev(x);
	/*
	WaypointList::const_iterator iter(binary_find(waypoint_list().begin(),waypoint_list().end(),x));

	if(iter!=waypoint_list().end())
	{
		if(iter->get_time()+Time::epsilon()<x)
			return iter;
		if(iter!=waypoint_list().begin() && (--iter)->get_time()+Time::epsilon()<x)
			return iter;
	}
	throw Exception::NotFound(strprintf("ValueNode_Animated::find_prev(): Can't find Waypoint after %s",x.get_string().c_str()));
	*/
}

void
ValueNode_Animated::erase(const UniqueID &x)
{
	waypoint_list().erase(find(x));
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
ValueNode_Animated::set_type(ValueBase::Type t)
{
	ValueNode::set_type(t);
}

ValueNode_Animated::Handle
synfig::ValueNode_Animated::create(ValueBase::Type type)
{
	switch(type)
	{
		case ValueBase::TYPE_TIME:
			return ValueNode_Animated::Handle(new _Hermite<Time>);
		case ValueBase::TYPE_REAL:
			return ValueNode_Animated::Handle(new _Hermite<Vector::value_type>);
		case ValueBase::TYPE_INTEGER:
			return ValueNode_Animated::Handle(new _Hermite<int>);
		case ValueBase::TYPE_ANGLE:
			return ValueNode_Animated::Handle(new _Hermite<Angle>);
		case ValueBase::TYPE_VECTOR:
			return ValueNode_Animated::Handle(new _Hermite<Vector>);
		case ValueBase::TYPE_COLOR:
			return ValueNode_Animated::Handle(new _Hermite<Color>);

		case ValueBase::TYPE_STRING:
			return ValueNode_Animated::Handle(new _Constant<String>);
		case ValueBase::TYPE_GRADIENT:
			return ValueNode_Animated::Handle(new _Hermite<Gradient>);
		case ValueBase::TYPE_BOOL:
			return ValueNode_Animated::Handle(new _AnimBool);
		case ValueBase::TYPE_CANVAS:
			return ValueNode_Animated::Handle(new _Constant<Canvas::LooseHandle>);
		default:
			throw
				Exception::BadType(strprintf(_("%s: You cannot use a %s in an animated ValueNode"),"synfig::ValueNode_Animated::create()",
					ValueBase::type_name(type).c_str())
				);
			break;
	}
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
