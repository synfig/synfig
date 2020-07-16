/* === S Y N F I G ========================================================= */
/*!	\file layergrouptree.cpp
**	\brief Layer set tree
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2017 caryoscelus
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

#include <synfig/layer.h>
#include "trees/layergrouptree.h"
#include <gtkmm/treemodelsort.h>
#include <ETL/misc>

#include <gui/localization.h>

#include <gui/exception_guard.h>

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

LayerGroupTree::LayerGroupTree() : editable_(false)
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

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
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

void
LayerGroupTree::set_editable(bool x)
{
	editable_=x;
/*
	if(editable_)
	{
		cell_renderer_time->property_editable()=true;
		cell_renderer_time_delta->property_editable()=true;
		cell_renderer_description->property_editable()=true;
	}
	else
	{
		cell_renderer_time->property_editable()=false;
		cell_renderer_time_delta->property_editable()=false;
		cell_renderer_description->property_editable()=false;
	}
*/
}
/*
void
LayerGroupTree::on_edited_time(const Glib::ustring&path_string,synfig::Time time)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row(*(get_model()->get_iter(path)));

	synfig::Keyframe keyframe(row[model.keyframe]);
	if(time!=keyframe.get_time())
	{
		row[model.time]=time;
		//keyframe.set_time(time);
		//signal_edited_time()(keyframe,time);
		//signal_edited()(keyframe);
	}
}

void
LayerGroupTree::on_edited_time_delta(const Glib::ustring&path_string,synfig::Time time)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row(*(get_model()->get_iter(path)));

	if(row)row[model.time_delta]=time;
}

void
LayerGroupTree::on_edited_description(const Glib::ustring&path_string,const Glib::ustring &desc)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_model()->get_iter(path));

	const synfig::String description(desc);
	synfig::Keyframe keyframe(row[model.keyframe]);
	if(description!=keyframe.get_description())
	{
		row[model.description]=desc;
		keyframe.set_description(description);
		signal_edited_description()(keyframe,description);
		signal_edited()(keyframe);
	}
}
*/

bool
LayerGroupTree::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			int wx(round_to_int(event->button.x)),wy(round_to_int(event->button.y));
			//tree_to_widget_coords (,, wx, wy);
			if(!get_path_at_pos(
				wx,wy,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(get_model()->get_iter(path));

			if(row[model.is_layer] && event->button.button==3)
			{
				signal_popup_layer_menu()((Layer::Handle)row[model.layer]);
				return true;
			}

			/*signal_user_click()(event->button.button,row,(ColumnID)column->get_sort_column_id());
			if((ColumnID)column->get_sort_column_id()==COLUMNID_JUMP)
			{
				layer_group_tree_store_->canvas_interface()->set_time(row[model.time]);
			}*/
		}
		break;
	case GDK_2BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(get_model()->get_iter(path));

			LayerList layer_list(row[model.all_layers]);
			if(!layer_list.empty())
			{
				if(!(event->button.state&GDK_CONTROL_MASK))
				{
					layer_group_tree_store_->canvas_interface()->get_selection_manager()->clear_selected_layers();
				}
				layer_group_tree_store_->canvas_interface()->get_selection_manager()->set_selected_layers(layer_list);
				return true;
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return Gtk::TreeView::on_event(event);
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
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
