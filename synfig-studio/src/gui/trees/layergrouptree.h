/* === S Y N F I G ========================================================= */
/*!	\file trees/layergrouptree.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_LAYERGROUPTREE_H
#define __SYNFIG_STUDIO_LAYERGROUPTREE_H

/* === H E A D E R S ======================================================= */

#include <gtkmm/treeview.h>
#include <gui/trees/layergrouptreestore.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig { class Layer; }

namespace studio {

class LayerGroupTree : public Gtk::TreeView
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:

	typedef std::list<synfig::Layer::Handle> LayerList;

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	LayerGroupTreeStore::Model model;

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store_;

	sigc::signal<void,etl::handle<synfig::Layer> > signal_popup_layer_menu_;

	Glib::RefPtr<Gtk::TreeSelection> tree_selection;
	/*
 -- ** -- P R I V A T E   M E T H O D S ---------------------------------------
	*/

private:

	/*
 -- ** -- S I G N A L   T E R M I N A L S -------------------------------------
	*/

private:

	virtual bool on_button_press_event(GdkEventButton *button_event);
	void on_selection_changed();
	void on_toggle(const Glib::ustring& path_string);
	void on_layer_renamed(const Glib::ustring&path_string,const Glib::ustring& value);

	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:

	LayerGroupTree();
	~LayerGroupTree();

	Glib::RefPtr<LayerGroupTreeStore> get_model() { return layer_group_tree_store_; }

	sigc::signal<void,etl::handle<synfig::Layer> >& signal_popup_layer_menu() { return signal_popup_layer_menu_; }

	void set_model(Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store_);

	std::list<synfig::String> get_selected_groups()const;

	LayerList get_selected_layers()const;
}; // END of LayerGroupTree

}; // END of namespace studio

/* === E N D =============================================================== */

#endif
