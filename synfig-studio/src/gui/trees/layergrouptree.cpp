/* === S Y N F I G ========================================================= */
/*!	\file layergrouptree.cpp
**	\brief Layer set tree
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2017 caryoscelus
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

#include <gui/trees/layergrouptree.h>

#include <ETL/misc>

#include <gui/exception_guard.h>
#include <gui/localization.h>

#include <synfig/layer.h>

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

LayerGroupTree::LayerGroupTree()
{
	const LayerGroupTreeStore::Model model;


	{	// --- O N / O F F ----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_(" ")) );

		// Set up the on/off cell-renderer
		Gtk::CellRendererToggle* cellrenderer = Gtk::manage( new Gtk::CellRendererToggle() );
		cellrenderer->signal_toggled().connect(sigc::mem_fun(*this, &studio::LayerGroupTree::on_toggle));
		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_active(), model.active);
		append_column(*column);
	}
	{	// --- I C O N --------------------------------------------------------
		int index;
		index=append_column(_(" "),model.icon);
		Gtk::TreeView::Column* column = get_column(index-1);
		set_expander_column(*column);
	}
	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Name")) );
		Gtk::CellRendererText* cellrenderer = Gtk::manage( new Gtk::CellRendererText() );
		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_text(), model.label);
		cellrenderer->signal_edited().connect(sigc::mem_fun(*this, &studio::LayerGroupTree::on_layer_renamed));
		cellrenderer->property_editable()=true;
		column->set_resizable();
		column->set_clickable(false);
		append_column(*column);
	}
	{	// --- Z - D E P T H
		int index = append_column(_("Z Depth"), model.z_depth)-1;

		Gtk::TreeViewColumn* column_z_depth = get_column(index);
		column_z_depth->set_reorderable();
		column_z_depth->set_resizable();
		column_z_depth->set_clickable();

		column_z_depth->set_sort_column(model.z_depth);
	}

	set_enable_search(true);
	set_search_column(model.label);
	set_search_equal_func(sigc::ptr_fun(&studio::LayerGroupTreeStore::search_func));

	// This makes things easier to read.
	set_rules_hint();

	// Make us more sensitive to several events
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	set_reorderable(true);

	tree_selection = get_selection();
	tree_selection->set_mode(Gtk::SELECTION_MULTIPLE);
	tree_selection->signal_changed().connect(sigc::mem_fun(*this, &LayerGroupTree::on_selection_changed));
}

LayerGroupTree::~LayerGroupTree()
{
	if (getenv("SYNFIG_DEBUG_DESTRUCTORS"))
		synfig::info("LayerGroupTree::~LayerGroupTree(): Deleted");
}

void
LayerGroupTree::set_model(Glib::RefPtr<LayerGroupTreeStore> layer_group_tree_store)
{
	layer_group_tree_store_=layer_group_tree_store;
	Gtk::TreeView::set_model(layer_group_tree_store);

	Gtk::TreeView::Column* column = get_column(2);
	if (column)
	{
		column->set_sort_column(layer_group_tree_store_->model.label);
		column->clicked();
	}
}

bool
LayerGroupTree::on_button_press_event(GdkEventButton *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
    if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	{
		Gtk::TreeModel::Path path;
		Gtk::TreeViewColumn *column;
		int cell_x, cell_y;
		int wx(round_to_int(event->x)),wy(round_to_int(event->y));
		//tree_to_widget_coords (,, wx, wy);
		if(!get_path_at_pos(
			   wx,wy,	// x, y
			   path, // TreeModel::Path&
			   column, //TreeViewColumn*&
			   cell_x,cell_y //int&cell_x,int&cell_y
			   )
		   )
			return Gtk::TreeView::on_button_press_event(event);

		const Gtk::TreeRow row = *(get_model()->get_iter(path));

		if(row[model.is_layer])
		{
			signal_popup_layer_menu()((Layer::Handle)row[model.layer]);
			return true;
		}
	}
	return Gtk::TreeView::on_button_press_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
LayerGroupTree::on_selection_changed()
{
	layer_group_tree_store_->canvas_interface()->get_selection_manager()->clear_selected_layers();

	for (Gtk::TreePath path : tree_selection->get_selected_rows()) {
		Gtk::TreeRow row = *get_model()->get_iter(path);
		LayerList layer_list(row[model.all_layers]);
		layer_group_tree_store_->canvas_interface()->get_selection_manager()->set_selected_layers(layer_list);
	}
}


void
LayerGroupTree::on_toggle(const Glib::ustring& path_string)
{
	Gtk::TreePath path(path_string);
	const Gtk::TreeRow row = *(get_model()->get_iter(path));
	bool active=static_cast<bool>(row[model.active]);
	row[model.active]=!active;
}

void
LayerGroupTree::on_layer_renamed(const Glib::ustring& path_string, const Glib::ustring& value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_model()->get_iter(path));
	if(!row)
		return;
	row[model.label]=value;
	columns_autosize();
}


static inline void __group_grabber(const Gtk::TreeModel::iterator& iter, std::list<synfig::String>* ret)
{
	const LayerGroupTreeStore::Model model;
	if((bool)(*iter)[model.is_group])
		ret->push_back((Glib::ustring)(*iter)[model.group_name]);
}

std::list<synfig::String>
LayerGroupTree::get_selected_groups()const
{
	Glib::RefPtr<Gtk::TreeSelection> selection=const_cast<LayerGroupTree&>(*this).get_selection();

	if(!selection)
		return std::list<synfig::String>();

	std::list<synfig::String> ret;

	selection->selected_foreach_iter(
		sigc::bind(
			sigc::ptr_fun(
				&__group_grabber
			),
			&ret
		)
	);

	return ret;
}

static inline void __layer_grabber(const Gtk::TreeModel::iterator& iter, LayerGroupTree::LayerList* ret)
{
	const LayerGroupTreeStore::Model model;
	if((bool)(*iter)[model.is_layer])
		ret->push_back((Layer::Handle)(*iter)[model.layer]);
}

LayerGroupTree::LayerList
LayerGroupTree::get_selected_layers()const
{
	Glib::RefPtr<Gtk::TreeSelection> selection=const_cast<LayerGroupTree&>(*this).get_selection();

	if(!selection)
		return LayerList();

	LayerList ret;

	selection->selected_foreach_iter(
		sigc::bind(
			sigc::ptr_fun(
				&__layer_grabber
			),
			&ret
		)
	);

	return ret;
}
