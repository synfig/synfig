/* === S Y N F I G ========================================================= */
/*!	\file metadatatreestore.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include <gui/trees/metadatatreestore.h>

#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

static MetaDataTreeStore::Model& ModelHack()
{
	static MetaDataTreeStore::Model* model(0);
	if(!model)model=new MetaDataTreeStore::Model;
	return *model;
}

MetaDataTreeStore::MetaDataTreeStore(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore	(ModelHack()),
	canvas_interface_		(canvas_interface_)
{
	// Connect the signal
	get_canvas()->signal_meta_data_changed().connect(sigc::mem_fun(*this,&MetaDataTreeStore::meta_data_changed));

	rebuild();
}

MetaDataTreeStore::~MetaDataTreeStore()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("MetaDataTreeStore::~MetaDataTreeStore(): Deleted");
}

Glib::RefPtr<MetaDataTreeStore>
MetaDataTreeStore::create(etl::loose_handle<synfigapp::CanvasInterface> canvas_interface_)
{
	return Glib::RefPtr<MetaDataTreeStore>(new MetaDataTreeStore(canvas_interface_));
}

void
MetaDataTreeStore::meta_data_changed(synfig::String /*key*/)
{
	rebuild();
}

void
MetaDataTreeStore::rebuild()
{
	clear();

	std::list<String> keys(get_canvas()->get_meta_data_keys());

	for(;!keys.empty();keys.pop_front())
	{
		Gtk::TreeRow row(*append());
		row[model.key]=keys.front();
	}
}

void
MetaDataTreeStore::get_value_vfunc (const Gtk::TreeModel::iterator& iter, int column, Glib::ValueBase& value)const
{
	if(column>=get_n_columns_vfunc())
	{
		g_warning("MetaDataTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(column==model.data.index())
	{
		synfig::String key((Glib::ustring)(*iter)[model.key]);
		g_value_init(value.gobj(),G_TYPE_STRING);
		g_value_set_string(value.gobj(),get_canvas()->get_meta_data(key).c_str());
		return;
	}
	else
		Gtk::TreeStore::get_value_vfunc(iter,column,value);
}

void
MetaDataTreeStore::set_value_impl(const Gtk::TreeModel::iterator& iter, int column, const Glib::ValueBase& value)
{
	if(column>=get_n_columns_vfunc())
	{
		g_warning("MetaDataTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("MetaDataTreeStore::set_value_impl: Bad value type");
		return;
	}

	if(column==model.data.index())
	{
		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),model.data.type());
		g_value_copy(value.gobj(),x.gobj());

		synfig::String key((Glib::ustring)(*iter)[model.key]);
		synfig::String new_data(x.get());

		get_canvas_interface()->set_meta_data(key,new_data);
	}
	else
		Gtk::TreeStore::set_value_impl(iter,column, value);
}
