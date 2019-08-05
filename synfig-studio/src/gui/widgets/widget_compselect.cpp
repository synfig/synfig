/* === S Y N F I G ========================================================= */
/*!	\file widget_compselect.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include <synfig/general.h>

#include <gtkmm/menu.h>
#include "widgets/widget_compselect.h"
#include <ETL/stringf>
#include <synfig/valuenode.h>
#include "instance.h"

#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Widget_CompSelect::Widget_CompSelect()
{
	App::signal_instance_created().connect(sigc::mem_fun(*this,&studio::Widget_CompSelect::new_instance));
	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::Widget_CompSelect::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::Widget_CompSelect::set_selected_instance_signal));

	refresh();
}

Widget_CompSelect::~Widget_CompSelect()
{
}

void
Widget_CompSelect::set_selected_instance_signal(etl::handle<studio::Instance> x)
{
	set_selected_instance(x);
}

void
Widget_CompSelect::set_selected_instance_(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	selected_instance=instance;
}

void
Widget_CompSelect::on_changed()
{
	int i = get_active_row_number();
	if (i < 0 || i >= (int)instances.size()) return;
	if (selected_instance == instances[i]) return;
	studio::App::set_selected_instance(instances[i]);
}

void
Widget_CompSelect::set_selected_instance(etl::loose_handle<studio::Instance> x)
{
	if(studio::App::shutdown_in_progress)
		return;

	// if it's already selected, don't select it again
	if (x==selected_instance)
		return;

	std::list<etl::handle<studio::Instance> >::iterator iter;

	if(x)
	{
		int i;
		for(i=0,iter=studio::App::instance_list.begin();iter!=studio::App::instance_list.end() && ((*iter)!=x);iter++,i++)
			;

		if (iter != studio::App::instance_list.end())
		{
			set_active(i);
		} else {
			synfig::warning("Can't set selected instance! (already closed?)");
			x.reset();
		}
	}
	else
		set_active(0);

	set_selected_instance_(x);
}

void
Widget_CompSelect::new_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	assert(instance);

	etl::loose_handle<studio::Instance> loose_instance(instance);

	instance->synfigapp::Instance::signal_filename_changed().connect(sigc::mem_fun(*this,&Widget_CompSelect::refresh));
	instance->synfigapp::Instance::signal_filename_changed().connect(
		sigc::bind<etl::loose_handle<studio::Instance> >(
			sigc::mem_fun(*this,&Widget_CompSelect::set_selected_instance),
			loose_instance
		)
	);

	{
		std::string name=basename(instance->get_file_name());
		instances.push_back(loose_instance);
		append(name);
	}

}

void
Widget_CompSelect::delete_instance(etl::handle<studio::Instance> instance)
{
	refresh();

	if(selected_instance==instance)
	{
		set_selected_instance(0);
		set_active(0);
	}
}

void
Widget_CompSelect::refresh()
{
	set_active(-1);
	remove_all();
	instances.clear();

	if(studio::App::shutdown_in_progress)
		return;

	std::list<etl::handle<studio::Instance> >::iterator iter;
	for(iter=studio::App::instance_list.begin();iter!=studio::App::instance_list.end();iter++)
	{
		std::string name=basename((*iter)->get_file_name());
		instances.push_back( etl::loose_handle<studio::Instance>(*iter) );
		append(name);
	}
}
