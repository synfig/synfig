/* === S Y N F I G ========================================================= */
/*!	\file dock_metadata.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include "docks/dock_metadata.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include "trees/metadatatreestore.h"
#include "trees/metadatatree.h"
#include "canvasview.h"

#include "general.h"

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

Dock_MetaData::Dock_MetaData():
	Dock_CanvasSpecific("meta_data",_("Canvas MetaData"),Gtk::StockID("synfig-meta_data"))
{

	add_button(
		Gtk::StockID("gtk-add"),
		_("Add new MetaData entry")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_MetaData::on_add_pressed
		)
	);

	add_button(
		Gtk::StockID("gtk-delete"),
		_("Remove selected MetaData entry")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_MetaData::on_delete_pressed
		)
	);
}

Dock_MetaData::~Dock_MetaData()
{
}

void
Dock_MetaData::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Glib::RefPtr<MetaDataTreeStore> metadata_tree_store;
	metadata_tree_store=MetaDataTreeStore::create(canvas_view->canvas_interface());
	MetaDataTree* metadata_tree(new MetaDataTree());
	metadata_tree->set_model(metadata_tree_store);
	metadata_tree->set_editable(true);
	canvas_view->set_tree_model(get_name(),metadata_tree_store);
	canvas_view->set_ext_widget(get_name(),metadata_tree);
}

void
Dock_MetaData::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));
		add(*tree_view);
		tree_view->show();
	}
	else
		clear_previous();
}

void
Dock_MetaData::on_add_pressed()
{
	if(get_canvas_interface())
	{
		synfig::String key;
		if(App::dialog_entry(_("New MetaData Entry"), _("Please enter the name of the key"),key) && !key.empty())
		{
			get_canvas_interface()->set_meta_data(key," ");
		}
	}
}

void
Dock_MetaData::on_delete_pressed()
{
	Gtk::TreeView* tree_view(static_cast<Gtk::TreeView*>(get_canvas_view()->get_ext_widget(get_name())));
	if(tree_view)
	{
		Gtk::TreeModel::iterator iter(tree_view->get_selection()->get_selected());
		if(tree_view->get_selection()->count_selected_rows())
		{
			Gtk::TreeRow row(*iter);
			Glib::RefPtr<Gtk::TreeModel> treemodel(get_canvas_view()->get_tree_model(get_name()));
			Glib::RefPtr<studio::MetaDataTreeStore> meta_data_tree_store(Glib::RefPtr<studio::MetaDataTreeStore>::cast_dynamic(treemodel));
			Glib::ustring key(row[meta_data_tree_store->model.key]);
			if(get_canvas_interface())
				get_canvas_interface()->erase_meta_data(key);
		}
	}
}
