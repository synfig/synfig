/* === S I N F G =========================================================== */
/*!	\file instance.cpp
**	\brief Instance
**
**	$Id: instance.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
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

#include "instance.h"
#include "canvasinterface.h"
#include <iostream>
#include <sinfg/loadcanvas.h>
#include <sinfg/savecanvas.h>
#include <sinfg/valuenode_composite.h>
#include <sinfg/valuenode_radialcomposite.h>
#include <sinfg/valuenode_reference.h>
#include <map>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

static std::map<loose_handle<Canvas>, loose_handle<Instance> > instance_map_;

/* === P R O C E D U R E S ================================================= */

bool
sinfgapp::is_editable(sinfg::ValueNode::Handle value_node)
{
	if(ValueNode_Const::Handle::cast_dynamic(value_node)
		|| ValueNode_TimedSwap::Handle::cast_dynamic(value_node)
		|| ValueNode_Animated::Handle::cast_dynamic(value_node)
		|| ValueNode_Composite::Handle::cast_dynamic(value_node)
		|| ValueNode_RadialComposite::Handle::cast_dynamic(value_node)
		|| ValueNode_Reference::Handle::cast_dynamic(value_node)
	)
		return true;
	return false;
}

etl::handle<Instance>
sinfgapp::find_instance(etl::handle<sinfg::Canvas> canvas)
{
	if(instance_map_.count(canvas)==0)
		return 0;
	return instance_map_[canvas];
}

/* === M E T H O D S ======================================================= */

Instance::Instance(Canvas::Handle canvas):
	CVSInfo(canvas->get_file_name()),
	canvas_(canvas)
{
	assert(canvas->is_root());

	unset_selection_manager();
	
	instance_map_[canvas]=this;
} // END of sinfgapp::Instance::Instance()

handle<Instance>
Instance::create(Canvas::Handle canvas)
{
	// Construct a new instance
	handle<Instance> instance(new Instance(canvas));

	return instance;
} // END of sinfgapp::Instance::create()

sinfg::String
Instance::get_file_name()const
{
	return get_canvas()->get_file_name();
}

void
Instance::set_file_name(const sinfg::String &name)
{
	get_canvas()->set_file_name(name);
	CVSInfo::set_file_name(name);
}

Instance::~Instance()
{
	instance_map_.erase(canvas_);
	sinfg::info("studio::Instance::~Instance(): Deleted");
} // END of studio::Instance::~Instance()

handle<CanvasInterface>
Instance::find_canvas_interface(handle<Canvas> canvas)
{
	if(!canvas)
		return 0;
	while(canvas->is_inline())
		canvas=canvas->parent();
	
	CanvasInterfaceList::iterator iter;

	for(iter=canvas_interface_list().begin();iter!=canvas_interface_list().end();iter++)
		if((*iter)->get_canvas()==canvas)
			return *iter;

	return CanvasInterface::create(this,canvas);
}

bool
Instance::save()const
{
	bool ret=save_canvas(get_file_name(),canvas_);
	if(ret)
	{
		reset_action_count();
		const_cast<sigc::signal<void>& >(signal_saved_)();
	}
	return ret;
}

bool
Instance::save_as(const std::string &file_name)const
{
	bool ret=save_canvas(file_name,canvas_);
	if(ret)
	{
		reset_action_count();
		const_cast<sigc::signal<void>& >(signal_saved_)();
	}
	return ret;
}

bool
Instance::save_as(const std::string &file_name)
{
	bool ret;

	String old_file_name(get_file_name());
	
	set_file_name(file_name);

	ret=save_canvas(file_name,canvas_);

	if(ret)
	{
		reset_action_count();
		signal_saved_();
	}
	else
	{
		set_file_name(old_file_name);
	}

	signal_filename_changed_();

	return ret;
}
