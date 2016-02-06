/* === S Y N F I G ========================================================= */
/*!	\file trees/metadatatree.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2010 Carlos López
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_METADATATREE_H
#define __SYNFIG_STUDIO_METADATATREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <synfigapp/canvasinterface.h>
#include "trees/metadatatreestore.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class MetaDataTree : public Gtk::TreeView
{
public:
	MetaDataTree();
	virtual ~MetaDataTree();
	MetaDataTreeStore::Model model;

private:
	Glib::RefPtr<MetaDataTreeStore> metadata_tree_store_;
	Gtk::CellRendererText *cell_renderer_key;
	Gtk::CellRendererText *cell_renderer_data;
	sigc::signal<void,synfig::String> signal_edited_;
	sigc::signal<void,synfig::String,synfig::String> signal_edited_data_;
	bool editable_;

private:
	void on_edited_key(const Glib::ustring&path_string,synfig::String key);
	void on_edited_data(const Glib::ustring&path_string,synfig::String data);

public:
	void set_model(Glib::RefPtr<MetaDataTreeStore> metadata_tree_store_);
	void set_editable(bool x=true);
	bool get_editable()const { return editable_; }
	//! Signal called when a metadata has been edited in any way
	sigc::signal<void,synfig::String>& signal_edited() { return signal_edited_; }
	//! Signal called when data has been edited.
	sigc::signal<void,synfig::String,synfig::String>& signal_edited_data() { return signal_edited_data_; }
}; // END of MetaDataTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
