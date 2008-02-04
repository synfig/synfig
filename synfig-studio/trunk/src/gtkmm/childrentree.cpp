/* === S Y N F I G ========================================================= */
/*!	\file childrentree.cpp
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

#include "childrentree.h"
#include "cellrenderer_value.h"
#include "cellrenderer_timetrack.h"
#include <synfigapp/action.h>
#include <synfigapp/instance.h>
#include <gtkmm/scrolledwindow.h>

#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

#ifndef SMALL_BUTTON
#define SMALL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),iconsize));	\
	button->add(*icon);	\
	tooltips_.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	button->set_relief(Gtk::RELIEF_NONE); \
	button->show()
#endif

#ifndef NORMAL_BUTTON
#define NORMAL_BUTTON(button,stockid,tooltip)	\
	button = manage(new class Gtk::Button());	\
	icon=manage(new Gtk::Image(Gtk::StockID(stockid),Gtk::ICON_SIZE_BUTTON));	\
	button->add(*icon);	\
	tooltips_.set_tip(*button,tooltip);	\
	icon->set_padding(0,0);\
	icon->show();	\
	/*button->set_relief(Gtk::RELIEF_NONE);*/ \
	button->show()
#endif

#define NEW_SMALL_BUTTON(x,y,z)	Gtk::Button *SMALL_BUTTON(x,y,z)

#define NOT_IMPLEMENTED_SLOT sigc::mem_fun(*reinterpret_cast<studio::CanvasViewUIInterface*>(get_ui_interface().get()),&studio::CanvasViewUIInterface::not_implemented)

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

ChildrenTree::ChildrenTree()
{
	const ChildrenTreeStore::Model model;

	{	// --- N A M E --------------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("ID")) );

		// Set up the icon cell-renderer
		Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );
		column->pack_start(*icon_cellrenderer,false);
		column->add_attribute(icon_cellrenderer->property_pixbuf(), model.icon);

		// Pack the label into the column
		column->pack_start(model.label,true);

		// Finish setting up the column
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();
		column->set_min_width(150);
		column->set_sort_column(model.label);
		tree_view.append_column(*column);

	}
	{	// --- T Y P E --------------------------------------------------------
		int cols_count = tree_view.append_column(_("Type"),model.type);
		Gtk::TreeViewColumn* column = tree_view.get_column(cols_count-1);
		if(column)
		{
			column->set_reorderable();
			column->set_resizable();
			column->set_clickable();
			column->set_sort_column(model.type);
		}
	}
	{	// --- V A L U E  -----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("ValueBase")) );

		// Set up the value cell-renderer
		cellrenderer_value=ChildrenTreeStore::add_cell_renderer_value(column);
		cellrenderer_value->signal_edited().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_edited_value));
		cellrenderer_value->property_value()=synfig::ValueBase();

		// Finish setting up the column
		tree_view.append_column(*column);
		column->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);
		column->set_min_width(150);
		column->set_reorderable();
		column->set_resizable();
		column->set_clickable(false);
	}
	{	// --- T I M E   T R A C K --------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Time Track")) );
		column_time_track=column;

		// Set up the value-node cell-renderer
		cellrenderer_time_track=ChildrenTreeStore::add_cell_renderer_value_node(column);
		cellrenderer_time_track->property_mode()=Gtk::CELL_RENDERER_MODE_ACTIVATABLE;
		cellrenderer_time_track->signal_waypoint_clicked_cellrenderer().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_waypoint_clicked_childrentree) );
		column->add_attribute(cellrenderer_time_track->property_value_desc(), model.value_desc);
		column->add_attribute(cellrenderer_time_track->property_canvas(), model.canvas);

		//column->pack_start(*cellrenderer_time_track);

		// Finish setting up the column
		column->set_reorderable();
		column->set_resizable();
		tree_view.append_column(*column);
	}

	// This makes things easier to read.
	tree_view.set_rules_hint();

	// Make us more sensitive to several events
	tree_view.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON1_MOTION_MASK | Gdk::BUTTON2_MOTION_MASK|Gdk::POINTER_MOTION_MASK);

	tree_view.signal_event().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_tree_event));

	// Create a scrolled window for that tree
	Gtk::ScrolledWindow *scroll_children_tree = manage(new class Gtk::ScrolledWindow());
	scroll_children_tree->set_flags(Gtk::CAN_FOCUS);
	scroll_children_tree->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll_children_tree->add(tree_view);
	scroll_children_tree->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scroll_children_tree->show();

	attach(*scroll_children_tree, 0, 3, 0, 1, Gtk::EXPAND|Gtk::FILL,Gtk::EXPAND|Gtk::FILL, 0, 0);

	hbox=manage(new Gtk::HBox());

	attach(*hbox, 0, 1, 1, 2, Gtk::FILL|Gtk::SHRINK, Gtk::SHRINK, 0, 0);



	tree_view.set_enable_search(true);
	tree_view.set_search_column(model.label);


/*
	Gtk::Image *icon;
	//Gtk::IconSize iconsize(Gtk::IconSize::from_name("synfig-small_icon"));
	Gtk::IconSize iconsize(Gtk::ICON_SIZE_SMALL_TOOLBAR);

	SMALL_BUTTON(button_raise,"gtk-go-up",_("Raise"));
	SMALL_BUTTON(button_lower,"gtk-go-down",_("Lower"));
	SMALL_BUTTON(button_duplicate,"synfig-duplicate",_("Duplicate"));
	SMALL_BUTTON(button_delete,"gtk-delete",_("Delete"));

	hbox->pack_start(*button_raise,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_lower,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_duplicate,Gtk::PACK_SHRINK);
	hbox->pack_start(*button_delete,Gtk::PACK_SHRINK);

	button_raise->signal_clicked().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_raise_pressed));
	button_lower->signal_clicked().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_lower_pressed));
	button_duplicate->signal_clicked().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_duplicate_pressed));
	button_delete->signal_clicked().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_delete_pressed));

	button_raise->set_sensitive(false);
	button_lower->set_sensitive(false);
	button_duplicate->set_sensitive(false);
	button_delete->set_sensitive(false);
*/



	get_selection()->signal_changed().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_selection_changed));


	tree_view.set_reorderable(true);

	hbox->show();
	tree_view.show();

	tooltips_.enable();

	//get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
}


ChildrenTree::~ChildrenTree()
{
}

void
ChildrenTree::set_show_timetrack(bool x)
{
	column_time_track->set_visible(x);
}

void
ChildrenTree::set_model(Glib::RefPtr<ChildrenTreeStore> children_tree_store)
{
	children_tree_store_=children_tree_store;
	tree_view.set_model(children_tree_store_);
	children_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::ChildrenTree::on_dirty_preview));
}

void
ChildrenTree::set_time_adjustment(Gtk::Adjustment &adjustment)
{
	cellrenderer_time_track->set_adjustment(adjustment);
}

void
ChildrenTree::on_dirty_preview()
{
}

void
ChildrenTree::on_selection_changed()
{
	if(0)
		{
		button_raise->set_sensitive(false);
		button_lower->set_sensitive(false);
		button_duplicate->set_sensitive(false);
		button_delete->set_sensitive(false);
		return;
	}
}


void
ChildrenTree::on_edited_value(const Glib::ustring&path_string,synfig::ValueBase value)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(tree_view.get_model()->get_iter(path));

	row[model.value]=value;
//	signal_edited_value()(row[model.value_desc],value);
}

void
ChildrenTree::on_waypoint_clicked_childrentree(const etl::handle<synfig::Node>& node __attribute__ ((unused)),
											   const synfig::Time& time __attribute__ ((unused)),
											   const synfig::Time& time_offset __attribute__ ((unused)),
											   int button __attribute__ ((unused)),
											   synfig::Waypoint::Side side __attribute__ ((unused)))
{
	//! \todo writeme

	// std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
	// signal_waypoint_clicked_childrentree()(waypoint_set,button,side);
}

bool
ChildrenTree::on_tree_event(GdkEvent *event)
{
    switch(event->type)
    {
	case GDK_BUTTON_PRESS:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!tree_view.get_path_at_pos(
				int(event->button.x),int(event->button.y),	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;
			const Gtk::TreeRow row = *(tree_view.get_model()->get_iter(path));

			if(column->get_first_cell_renderer()==cellrenderer_time_track)
			{
				return signal_user_click()(event->button.button,row,COLUMNID_TIME_TRACK);
			}
			else if(column->get_first_cell_renderer()==cellrenderer_value)
				return signal_user_click()(event->button.button,row,COLUMNID_VALUE);
			else
				return signal_user_click()(event->button.button,row,COLUMNID_ID);

		}
		break;

	case GDK_MOTION_NOTIFY:
		{
			Gtk::TreeModel::Path path;
			Gtk::TreeViewColumn *column;
			int cell_x, cell_y;
			if(!tree_view.get_path_at_pos(
				(int)event->button.x,(int)event->button.y,	// x, y
				path, // TreeModel::Path&
				column, //TreeViewColumn*&
				cell_x,cell_y //int&cell_x,int&cell_y
				)
			) break;

			if(!tree_view.get_model()->get_iter(path))
				break;

			Gtk::TreeRow row = *(tree_view.get_model()->get_iter(path));

			if(cellrenderer_time_track==column->get_first_cell_renderer())
			{
				// Movement on TimeLine
				return true;
			}
			else
			if(last_tooltip_path.get_depth()<=0 || path!=last_tooltip_path)
			{
				tooltips_.unset_tip(*this);
				Glib::ustring tooltips_string(row[model.tooltip]);
				last_tooltip_path=path;
				if(!tooltips_string.empty())
				{
					tooltips_.set_tip(*this,tooltips_string);
					tooltips_.force_window();
				}
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return false;
}

void
ChildrenTree::on_raise_pressed()
{
}

void
ChildrenTree::on_lower_pressed()
{
}

void
ChildrenTree::on_duplicate_pressed()
{
}

void
ChildrenTree::on_delete_pressed()
{
}
