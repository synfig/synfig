/* === S I N F G =========================================================== */
/*!	\file metadatatreestore.cpp
**	\brief Template File
**
**	$Id: metadatatreestore.cpp,v 1.1.1.1 2005/01/07 03:34:36 darco Exp $
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

#include "metadatatreestore.h"
#include <sinfgapp/canvasinterface.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
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

MetaDataTreeStore::MetaDataTreeStore(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_):
	Gtk::TreeStore	(ModelHack()),
	canvas_interface_		(canvas_interface_)
{
	// Connect the signal
	get_canvas()->signal_meta_data_changed().connect(sigc::mem_fun(*this,&MetaDataTreeStore::meta_data_changed));
	
	rebuild();
}

MetaDataTreeStore::~MetaDataTreeStore()
{
	sinfg::info("MetaDataTreeStore::~MetaDataTreeStore(): Deleted");

}

Glib::RefPtr<MetaDataTreeStore>
MetaDataTreeStore::create(etl::loose_handle<sinfgapp::CanvasInterface> canvas_interface_)
{
	return Glib::RefPtr<MetaDataTreeStore>(new MetaDataTreeStore(canvas_interface_));
}

void
MetaDataTreeStore::meta_data_changed(sinfg::String key)
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
		g_warning("KeyframeTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(column==model.data.index())
	{
		sinfg::String key((Glib::ustring)(*iter)[model.key]);
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
		g_warning("KeyframeTreeStore::set_value_impl: Bad column (%d)",column);
		return;
	}

	if(!g_value_type_compatible(G_VALUE_TYPE(value.gobj()),get_column_type_vfunc(column)))
	{
		g_warning("KeyframeTreeStore::set_value_impl: Bad value type");
		return;
	}

	if(column==model.data.index())
	{
		Glib::Value<Glib::ustring> x;
		g_value_init(x.gobj(),model.data.type());
		g_value_copy(value.gobj(),x.gobj());
		
		sinfg::String key((Glib::ustring)(*iter)[model.key]);
		sinfg::String new_data(x.get());
		
		get_canvas_interface()->set_meta_data(key,new_data);
	}
	else
		Gtk::TreeStore::set_value_impl(iter,column, value);
}
