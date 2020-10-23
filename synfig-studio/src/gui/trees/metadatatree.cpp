/* === S Y N F I G ========================================================= */
/*!	\file metadatatree.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2011 Carlos López
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

#include <gui/trees/metadatatree.h>

#include <gui/localization.h>

#include <synfig/general.h>

#endif

/* === U S I N G =========================================================== */

using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

MetaDataTree::MetaDataTree() : editable_(false)
{
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Key")) );

		cell_renderer_key = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*cell_renderer_key,true);
		column->add_attribute(cell_renderer_key->property_text(), model.key);
		cell_renderer_key->signal_edited().connect(sigc::mem_fun(*this,&studio::MetaDataTree::on_edited_key));
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();
		column->set_sort_column(model.key);
		append_column(*column);
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Data")) );

		cell_renderer_data = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*cell_renderer_data,true);
		column->add_attribute(cell_renderer_data->property_text(), model.data);
		cell_renderer_data->signal_edited().connect(sigc::mem_fun(*this,&studio::MetaDataTree::on_edited_data));
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable(false);
		append_column(*column);
	}
	set_rules_hint();
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
}

MetaDataTree::~MetaDataTree()
{
}

void
MetaDataTree::set_model(Glib::RefPtr<MetaDataTreeStore> metadata_tree_store)
{
	metadata_tree_store_=metadata_tree_store;
	Gtk::TreeView::set_model(metadata_tree_store);
}

void
MetaDataTree::set_editable(bool x)
{
	editable_=x;

	if(editable_)
		cell_renderer_data->property_editable()=true;
	else
		cell_renderer_data->property_editable()=false;
}

void
MetaDataTree::on_edited_key(const Glib::ustring&path_string,synfig::String key)
{
	Gtk::TreePath path(path_string);
	const Gtk::TreeRow row(*(get_model()->get_iter(path)));
	if(row) row[model.key]=key;
}

void
MetaDataTree::on_edited_data(const Glib::ustring&path_string,synfig::String data)
{
	Gtk::TreePath path(path_string);
	const Gtk::TreeRow row(*(get_model()->get_iter(path)));
	if(row)row[model.data]=data;
}

