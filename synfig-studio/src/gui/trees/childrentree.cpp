/* === S Y N F I G ========================================================= */
/*!	\file childrentree.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
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

#include "trees/childrentree.h"
#include "cellrenderer/cellrenderer_value.h"
#include "cellrenderer/cellrenderer_timetrack.h"
#include <synfigapp/action.h>
#include <synfigapp/instance.h>
#include <gtkmm/scrolledwindow.h>
#include <synfig/timepointcollect.h>

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
		column->add_attribute(cellrenderer_value->property_value_desc(), model.value_desc);

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
		// ice0: already added in constructor.
		// if we need to change it, then we need to rewrite this code
		//column->add_attribute(cellrenderer_time_track->property_canvas(), model.canvas);

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
	tree_view.signal_query_tooltip().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_tree_view_query_tooltip));

	// Create a scrolled window for that tree
	Gtk::ScrolledWindow *scroll_children_tree = manage(new class Gtk::ScrolledWindow());
	scroll_children_tree->set_can_focus(true);
	scroll_children_tree->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scroll_children_tree->add(tree_view);
	scroll_children_tree->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scroll_children_tree->show();

	attach(*scroll_children_tree, 0, 3, 0, 1, Gtk::EXPAND|Gtk::FILL,Gtk::EXPAND|Gtk::FILL, 0, 0);

	hbox=manage(new Gtk::HBox());

	attach(*hbox, 0, 1, 1, 2, Gtk::FILL|Gtk::SHRINK, Gtk::SHRINK, 0, 0);

	tree_view.set_enable_search(true);
	tree_view.set_search_column(model.label);

	get_selection()->signal_changed().connect(sigc::mem_fun(*this, &studio::ChildrenTree::on_selection_changed));

	tree_view.set_reorderable(true);

	hbox->show();
	tree_view.show();

	tree_view.set_has_tooltip();

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
	cellrenderer_time_track->set_canvas_interface(children_tree_store_->canvas_interface());
	children_tree_store_->canvas_interface()->signal_dirty_preview().connect(sigc::mem_fun(*this,&studio::ChildrenTree::on_dirty_preview));
}

void
ChildrenTree::set_time_model(const etl::handle<TimeModel> &x)
	{ cellrenderer_time_track->set_time_model(x); }

void
ChildrenTree::on_dirty_preview()
{
}

void
ChildrenTree::on_selection_changed()
{
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
											   const synfig::Time& time_dilation __attribute__ ((unused)),
											   int button __attribute__ ((unused)))
{
	std::set<synfig::Waypoint, std::less<UniqueID> > waypoint_set;
	synfig::waypoint_collect(waypoint_set,time,node);

	synfigapp::ValueDesc value_desc;

	if (waypoint_set.size() == 1)
	{
		ValueNode::Handle value_node(waypoint_set.begin()->get_parent_value_node());
		assert(value_node);

		Gtk::TreeRow row;
		if (children_tree_store_->find_first_value_node(value_node, row) && row)
			value_desc = static_cast<synfigapp::ValueDesc>(row[model.value_desc]);
	}

	if (!waypoint_set.empty())
		signal_waypoint_clicked_childrentree()(value_desc,waypoint_set,button);
}

bool
ChildrenTree::on_tree_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
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

			if(column->get_first_cell()==cellrenderer_time_track)
			{
				Gdk::Rectangle rect;
				tree_view.get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[model.value_desc];
				cellrenderer_time_track->property_canvas()=row[model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
			}
			else if(column->get_first_cell()==cellrenderer_value)
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

			if(cellrenderer_time_track==column->get_first_cell())
			{
				// Movement on TimeLine
				Gdk::Rectangle rect;
				tree_view.get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[model.value_desc];
				cellrenderer_time_track->property_canvas()=row[model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				queue_draw();
				//queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
			}
		}
		break;
	case GDK_BUTTON_RELEASE:
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

			if(column && cellrenderer_time_track == column->get_first_cell())
			{
				Gdk::Rectangle rect;
				tree_view.get_cell_area(path,*column,rect);
				cellrenderer_time_track->property_value_desc()=row[model.value_desc];
				cellrenderer_time_track->property_canvas()=row[model.canvas];
				cellrenderer_time_track->activate(event,*this,path.to_string(),rect,rect,Gtk::CellRendererState());
				queue_draw();
				queue_draw_area(rect.get_x(),rect.get_y(),rect.get_width(),rect.get_height());
				return true;
			}
		}
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

bool
ChildrenTree::on_tree_view_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip)
{
	if(keyboard_tooltip)
		return false;
	Gtk::TreeModel::Path path;
	Gtk::TreeViewColumn *column;
	int cell_x, cell_y;
	int bx, by;
	get_tree_view().convert_widget_to_bin_window_coords(x, y, bx, by);
	if(!get_tree_view().get_path_at_pos(bx, by, path, column, cell_x,cell_y))
		return false;
	Gtk::TreeIter iter(get_tree_view().get_model()->get_iter(path));
	if(!iter)
		return false;
	Gtk::TreeRow row = *(iter);
	Glib::ustring tooltip_string(row[model.tooltip]);
	if(tooltip_string.empty())
		return false;
	tooltip->set_text(tooltip_string);
	get_tree_view().set_tooltip_row(tooltip, path);
	return true;
}
