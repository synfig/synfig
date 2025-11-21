/* === S Y N F I G ========================================================= */
/*!	\file keyframetree.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2008 Chris Moore
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include <gui/trees/keyframetree.h>

#include <gtkmm/treemodelsort.h>

#include <gui/app.h>
#include <gui/cellrenderer/cellrenderer_time.h>
#include <gui/exception_guard.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

KeyframeTree::KeyframeTree() : editable_(false)
{
	const KeyframeTreeStore::Model model;

	{	// --- O N / O F F ----------------------------------------------------
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_(" ")) );

		// Set up the on/off cell-renderer
		Gtk::CellRendererToggle* cellrenderer = Gtk::manage( new Gtk::CellRendererToggle() );
		cellrenderer->signal_toggled().connect(sigc::mem_fun(*this, &studio::KeyframeTree::on_keyframe_toggle));
		column->pack_start(*cellrenderer,false);
		column->add_attribute(cellrenderer->property_active(), model.active);
		append_column(*column);
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Time")) );

		cell_renderer_time = Gtk::manage( new CellRenderer_Time() );
		column->pack_start(*cell_renderer_time,true);
		column->add_attribute(cell_renderer_time->property_time(), model.time);
		cell_renderer_time->signal_edited().connect(sigc::mem_fun(*this,&studio::KeyframeTree::on_edited_time));

		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();
		column->set_sort_column(model.time);

		append_column(*column);
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Length")) );

		cell_renderer_time_delta = Gtk::manage( new CellRenderer_Time() );
		column->pack_start(*cell_renderer_time_delta,true);
		column->add_attribute(cell_renderer_time_delta->property_time(), model.time_delta);
		cell_renderer_time_delta->signal_edited().connect(sigc::mem_fun(*this,&studio::KeyframeTree::on_edited_time_delta));

		column->set_reorderable();
		column->set_resizable();
		column->set_clickable(false);
		// column->set_sort_column(model.time_delta);

		append_column(*column);
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Jump")) );

		Gtk::CellRendererText* cell_renderer_jump=Gtk::manage(new Gtk::CellRendererText());
		column->pack_start(*cell_renderer_jump,true);
		cell_renderer_jump->property_text()=_("(JMP)");
		cell_renderer_jump->property_foreground()="#003a7f";

		column->set_reorderable();
		column->set_resizable();
		column->set_clickable(false);

		append_column(*column);
	}
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("Description")) );

		cell_renderer_description=Gtk::manage(new Gtk::CellRendererText());
		column->pack_start(*cell_renderer_description,true);
		column->add_attribute(cell_renderer_description->property_text(), model.description);
		cell_renderer_description->signal_edited().connect(sigc::mem_fun(*this,&studio::KeyframeTree::on_edited_description));

		column->set_reorderable();
		column->set_resizable();
		column->set_clickable();
		column->set_sort_column(model.description);

		append_column(*column);
	}

	set_enable_search(true);
	set_search_column(model.description);

	// This makes things easier to read.
	set_rules_hint();

	get_style_context()->add_class("keyframes");

	// Make us more sensitive to several events
	add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);

	// Listen to the changed selection signal to perform kf synchro thru canvas interface
	get_selection()->signal_changed().connect(sigc::mem_fun(*this, &studio::KeyframeTree::on_selection_changed));
	send_selection = false;
}

KeyframeTree::~KeyframeTree()
{
	keyframeselected.disconnect();
	DEBUG_LOG("SYNFIG_DEBUG_DESTRUCTORS",
		"KeyframeTree::~KeyframeTree(): Deleted");
}

void
KeyframeTree::on_rend_desc_changed()
{
	cell_renderer_time->property_fps().set_value(keyframe_tree_store_->canvas_interface()->get_canvas()->rend_desc().get_frame_rate());
	queue_draw();
}

void
KeyframeTree::set_model(Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store)
{
	keyframe_tree_store_ = keyframe_tree_store;
	KeyframeTreeStore::Model model;

	Glib::RefPtr<Gtk::TreeModelSort> sorted_store(Gtk::TreeModelSort::create(keyframe_tree_store_));
	sorted_store->set_default_sort_func(sigc::ptr_fun(&studio::KeyframeTreeStore::time_sorter));
	sorted_store->set_sort_func(model.time,			sigc::ptr_fun(&studio::KeyframeTreeStore::time_sorter));
	sorted_store->set_sort_func(model.description,	sigc::ptr_fun(&studio::KeyframeTreeStore::description_sorter));
	Gtk::TreeView::set_model(sorted_store);
	Gtk::TreeView::set_model(keyframe_tree_store);

	keyframe_tree_store_->canvas_interface()->signal_rend_desc_changed().connect(
		sigc::mem_fun( *this, &studio::KeyframeTree::on_rend_desc_changed ) );
	cell_renderer_time->property_fps().set_value(
		keyframe_tree_store_->canvas_interface()->get_canvas()->rend_desc().get_frame_rate() );
	cell_renderer_time_delta->property_fps().set_value(
		keyframe_tree_store_->canvas_interface()->get_canvas()->rend_desc().get_frame_rate() );

	//Listen to kf selection change from canvas interface
	keyframeselected = keyframe_tree_store_->canvas_interface()->signal_keyframe_selected().connect(
		sigc::mem_fun( *this, &studio::KeyframeTree::on_keyframe_selected ) );
}

void
KeyframeTree::set_editable(bool x)
{
	editable_=x;

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
}

void
KeyframeTree::on_keyframe_toggle(const Glib::ustring& path_string)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row = *(get_model()->get_iter(path));
	bool active=static_cast<bool>(row[model.active]);
	row[model.active]=!active;
}

void
KeyframeTree::on_edited_time(const Glib::ustring&path_string,synfig::Time time)
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
KeyframeTree::on_edited_time_delta(const Glib::ustring&path_string,synfig::Time time)
{
	Gtk::TreePath path(path_string);

	const Gtk::TreeRow row(*(get_model()->get_iter(path)));

	if(row)row[model.time_delta]=time;
}

void
KeyframeTree::on_edited_description(const Glib::ustring&path_string,const Glib::ustring &desc)
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

bool
KeyframeTree::on_event(GdkEvent *event)
{
	SYNFIG_EXCEPTION_GUARD_BEGIN()
    switch(event->type)
    {
	case GDK_KEY_PRESS:
		{
			send_selection = true;
		}
		break;
	case GDK_BUTTON_PRESS:
		{
			if (event->button.button == 1)
			{
				Gtk::TreeModel::Path path;
				Gtk::TreeViewColumn *column;
				int cell_x, cell_y;
				int wx(round_to_int(event->button.x)),wy(round_to_int(event->button.y));
				//tree_to_widget_coords (,, wx, wy);
				send_selection = true;

				if(!get_path_at_pos(
					wx,wy,	// x, y
					path, // TreeModel::Path&
					column, //TreeViewColumn*&
					cell_x,cell_y //int&cell_x,int&cell_y
					)
				) break;
				const Gtk::TreeRow row = *(get_model()->get_iter(path));

				signal_user_click()(event->button.button,row,(ColumnID)column->get_sort_column_id());
				if (synfig::String(column->get_title ()) == _("Jump"))
				{
					keyframe_tree_store_->canvas_interface()->set_time(row[model.time]);
				}
			} else if (event->button.button == 3)
			{
				if (App::menu_keyframe) {
					Gtk::Menu* menu = Gtk::manage(new Gtk::Menu(App::menu_keyframe));
					if (menu) {
						menu->attach_to_widget(*this);
						menu->popup(event->button.button, gtk_get_current_event_time());
					}
				}
			}

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

			{
				keyframe_tree_store_->canvas_interface()->set_time(row[model.time]);
				return true;
			}
		}
		break;

	case GDK_BUTTON_RELEASE:
		break;
	default:
		break;
	}
	return false;
	SYNFIG_EXCEPTION_GUARD_END_BOOL(true)
}

void
KeyframeTree::on_selection_changed()
{
	//Connected on treeview::selection::changed

	//if(send_selection && has_focus () && get_selection()->count_selected_rows()==1)
	if(send_selection && get_selection()->count_selected_rows()==1) {
		Keyframe keyframe((*get_selection()->get_selected())[model.keyframe]);
		if(keyframe && keyframe != selected_kf && keyframe_tree_store_) {
			selected_kf = keyframe;
			keyframe_tree_store_->canvas_interface()->signal_keyframe_selected()(keyframe);
		}
	}
}

void
KeyframeTree::on_keyframe_selected(synfig::Keyframe keyframe)
{
	Gtk::TreeModel::Path path;
	if (keyframe && keyframe != selected_kf) {
		selected_kf = keyframe;
		send_selection = false;
		if(keyframe_tree_store_ && keyframe_tree_store_->find_keyframe_path(keyframe,path))
			set_cursor (path);
	} else send_selection = true;
}
