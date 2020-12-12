/* === S Y N F I G ========================================================= */
/*!	\file childrentreestore.cpp
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

#include <gui/trees/childrentreestore.h>

#include <ETL/clock>
#include <glibmm/main.h>
#include <gtkmm/button.h>
#include <gui/localization.h>
#include <synfig/general.h>

class Profiler : private etl::clock
{
	const std::string name;
public:
	Profiler(const std::string& name):name(name) { reset(); }
	~Profiler() { float time(operator()()); synfig::info("%s: took %f msec",name.c_str(),time*1000); }
};

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

static ChildrenTreeStore::Model& ModelHack()
{
	static ChildrenTreeStore::Model* model(0);
	if(!model)model=new ChildrenTreeStore::Model;
	return *model;
}

ChildrenTreeStore::ChildrenTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore			(ModelHack()),
	CanvasTreeStore			(canvas_interface_)
{
	canvas_row=*append();
	canvas_row[model.label]=_("Canvases");
	canvas_row[model.is_canvas] = false;
	canvas_row[model.is_value_node] = false;

	value_node_row=*append();
	value_node_row[model.label]=_("ValueBase Nodes");
	value_node_row[model.is_canvas] = false;
	value_node_row[model.is_value_node] = false;

	// Connect all the signals
	canvas_interface()->signal_value_node_changed().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_value_node_changed));
	canvas_interface()->signal_value_node_renamed().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_value_node_renamed));
	canvas_interface()->signal_value_node_added().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_value_node_added));
	canvas_interface()->signal_value_node_deleted().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_value_node_deleted));
	canvas_interface()->signal_value_node_replaced().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_value_node_replaced));
	canvas_interface()->signal_canvas_added().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_canvas_added));
	canvas_interface()->signal_canvas_removed().connect(sigc::mem_fun(*this,&studio::ChildrenTreeStore::on_canvas_removed));

	rebuild();
}

ChildrenTreeStore::~ChildrenTreeStore()
{
}

Glib::RefPtr<ChildrenTreeStore>
ChildrenTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_)
{
	return Glib::RefPtr<ChildrenTreeStore>(new ChildrenTreeStore(canvas_interface_));
}

void
ChildrenTreeStore::rebuild()
{
	// Profiler profiler("ChildrenTreeStore::rebuild()");
	rebuild_value_nodes();
	rebuild_canvases();
}

void
ChildrenTreeStore::refresh()
{
	// Profiler profiler("ChildrenTreeStore::refresh()");
	refresh_value_nodes();
	refresh_canvases();
}

void
ChildrenTreeStore::rebuild_value_nodes()
{
	Gtk::TreeModel::Children children(value_node_row.children());

	while(!children.empty())erase(children.begin());

	clear_changed_queue();

	std::for_each(
		canvas_interface()->get_canvas()->value_node_list().rbegin(), canvas_interface()->get_canvas()->value_node_list().rend(),
		sigc::mem_fun(*this, &studio::ChildrenTreeStore::on_value_node_added)
	);
}

void
ChildrenTreeStore::refresh_value_nodes()
{
	Gtk::TreeModel::Children children(value_node_row.children());

	Gtk::TreeModel::Children::iterator iter;

	if(!children.empty())
		for(iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeRow row=*iter;
			refresh_row(row);
		}
}

void
ChildrenTreeStore::rebuild_canvases()
{
	Gtk::TreeModel::Children children(canvas_row.children());

	while(!children.empty())erase(children.begin());

	std::for_each(
		canvas_interface()->get_canvas()->children().rbegin(), canvas_interface()->get_canvas()->children().rend(),
		sigc::mem_fun(*this, &studio::ChildrenTreeStore::on_canvas_added)
	);
}

void
ChildrenTreeStore::refresh_canvases()
{
	rebuild_canvases();
}

void
ChildrenTreeStore::refresh_row(Gtk::TreeModel::Row &row, bool /*do_children*/)
{
	CanvasTreeStore::refresh_row(row,false);

	if((bool)row[model.is_value_node])
	{
		changed_set_.erase(row[model.value_node]);
	}

}

void
ChildrenTreeStore::on_canvas_added(synfig::Canvas::Handle canvas)
{
	Gtk::TreeRow row = *(prepend(canvas_row.children()));

	row[model.icon] = Gtk::Button().render_icon_pixbuf(Gtk::StockID("synfig-type_canvas"),Gtk::ICON_SIZE_SMALL_TOOLBAR);
	row[model.id] = canvas->get_id();
	row[model.name] = canvas->get_name();

	if(!canvas->get_id().empty())
		row[model.label] = canvas->get_id();
	else
	if(!canvas->get_name().empty())
		row[model.label] = canvas->get_name();
	else
		row[model.label] = _("[Unnamed]");

	row[model.canvas] = canvas;
	row[model.type] = _("Canvas");
	//row[model.is_canvas] = true;
	//row[model.is_value_node] = false;
}

void
ChildrenTreeStore::on_canvas_removed(synfig::Canvas::Handle /*canvas*/)
{
	rebuild_canvases();
}

void
ChildrenTreeStore::on_value_node_added(synfig::ValueNode::Handle value_node)
{
//	if(value_node->get_id().find("Unnamed")!=String::npos)
//		return;

	Gtk::TreeRow row = *prepend(value_node_row.children());

	set_row(row,synfigapp::ValueDesc(canvas_interface()->get_canvas(),value_node->get_id()),false);
}

void
ChildrenTreeStore::on_value_node_deleted(synfig::ValueNode::Handle value_node)
{
	Gtk::TreeIter iter;
	//int i(0);

	if(find_first_value_node(value_node,iter))
	{
		erase(iter);
	}
	//rebuild_value_nodes();
}

bool
ChildrenTreeStore::execute_changed_value_nodes()
{
	// Profiler profiler("ChildrenTreeStore::execute_changed_value_nodes()");
	if(!replaced_set_.empty())
		rebuild_value_nodes();

	etl::clock timer;
	timer.reset();

	while(!changed_set_.empty())
	{
		ValueNode::Handle value_node(*changed_set_.begin());
		changed_set_.erase(value_node);

		Gtk::TreeIter iter;

		try
		{
			Gtk::TreeIter iter;
			int i(0);

			if(!value_node->is_exported() && find_first_value_node(value_node,iter))
			{
				rebuild_value_nodes();
				continue;
			}

			if(value_node->is_exported() && find_first_value_node(value_node,iter)) do
			{
				Gtk::TreeRow row(*iter);
				i++;
				refresh_row(row);
			}while(find_next_value_node(value_node,iter));

			if(!i)
			{
				refresh_value_nodes();
				return false;
			}

		}
		catch(...)
		{
			rebuild_value_nodes();
			return false;
		}

		// If we are taking too long...
		if(timer()>4)
		{
			refresh_value_nodes();
			return false;
		}
	}

	return false;
}

void
ChildrenTreeStore::on_value_node_changed(synfig::ValueNode::Handle value_node)
{

	if(!value_node->is_exported())
		return;
	changed_connection.disconnect();
//	if(!execute_changed_queued())
//		changed_connection=Glib::signal_idle().connect(sigc::mem_fun(*this,&ChildrenTreeStore::execute_changed_value_nodes));
	changed_connection=Glib::signal_timeout().connect(sigc::mem_fun(*this,&ChildrenTreeStore::execute_changed_value_nodes),150);

	changed_set_.insert(value_node);
	/*
	try
	{
		Gtk::TreeIter iter;
		int i(0);
		while(find_next_value_node(value_node,iter))
		{
			Gtk::TreeRow row(*iter);
			i++;
			refresh_row(row);
		}
		if(!i)
		{
			refresh_value_nodes();
		}
	}
	catch(...)
	{
		rebuild_value_nodes();
	}
	*/
}

void
ChildrenTreeStore::on_value_node_renamed(synfig::ValueNode::Handle value_node)
{
	rebuild_value_nodes();
}

void
ChildrenTreeStore::on_value_node_replaced(synfig::ValueNode::Handle replaced_value_node,synfig::ValueNode::Handle /*new_value_node*/)
{
	changed_connection.disconnect();
	//if(!execute_changed_queued())
//		changed_connection=Glib::signal_idle().connect(sigc::mem_fun(*this,&ChildrenTreeStore::execute_changed_value_nodes));
		changed_connection=Glib::signal_timeout().connect(sigc::mem_fun(*this,&ChildrenTreeStore::execute_changed_value_nodes),150);

	replaced_set_.insert(replaced_value_node);
}

void
ChildrenTreeStore::set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value)
{
	if(column>=get_n_columns_vfunc())
	{
		g_warning("LayerTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("LayerTreeStore::set_value_impl: Bad value type");
		return;
	}

	try
	{
		if(column==model.value.index())
		{
			Glib::Value<synfig::ValueBase> x;
			g_value_init(x.gobj(),model.value.type());
			g_value_copy(value.gobj(),x.gobj());

			synfigapp::ValueDesc value_desc((*iter)[model.value_desc]);
			if(value_desc)
			{
				canvas_interface()->change_value(value_desc,x.get());
				row_changed(get_path(*iter),*iter);
			}

			return;
		}
		else
			CanvasTreeStore::set_value_impl(iter,column, value);
	}
	catch(std::exception& x)
	{
		g_warning("%s", x.what());
	}
}
