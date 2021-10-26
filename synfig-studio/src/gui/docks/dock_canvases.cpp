/* === S Y N F I G ========================================================= */
/*!	\file dock_canvases.cpp
**	\brief Template File
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "docks/dock_canvases.h"

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dock_Canvases::Dock_Canvases():
	Dockable("canvases",_("Canvas Browser"),Gtk::StockID("synfig-canvas"))
{

	App::signal_instance_created().connect(sigc::mem_fun(*this,&studio::Dock_Canvases::new_instance));
	App::signal_instance_deleted().connect(sigc::mem_fun(*this,&studio::Dock_Canvases::delete_instance));
	App::signal_instance_selected().connect(sigc::mem_fun(*this,&studio::Dock_Canvases::set_selected_instance_signal));


	add(*create_canvas_tree());

/*  // \todo Implement canvas management in canvas browser
	add_button(
		Gtk::StockID("synfig-canvas_new"),
		_("Insert a new canvas")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Canvases::menu_new_canvas
		)
	);

	add_button(
		Gtk::StockID("gtk-delete"),
		_("Remove selected canvas")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Canvases::menu_delete
		)
	);

	add_button(
		Gtk::StockID("synfig-rename"),
		_("Rename selected canvas")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Canvases::menu_rename
		)
	);
*/
}

Dock_Canvases::~Dock_Canvases()
{
}

Gtk::Widget*
Dock_Canvases::create_canvas_tree()
{
	studio::Instance::CanvasTreeModel canvas_tree_model;
	canvas_tree=manage(new class Gtk::TreeView());
	{
		Gtk::TreeView::Column* column = Gtk::manage( new Gtk::TreeView::Column(_("ID")) );
//		Gtk::CellRendererPixbuf* icon_cellrenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

		//column->pack_start(*icon_cellrenderer,false);
		column->pack_start(canvas_tree_model.icon, false); //false = don't expand.
		column->pack_start(canvas_tree_model.label);

//#ifdef NDEBUG
//		column->add_attribute(icon_cellrenderer->property_pixbuf(), canvas_tree_model.icon);
//#endif

		canvas_tree->append_column(*column);
	}
	canvas_tree->set_rules_hint();
	canvas_tree->signal_row_activated().connect(sigc::mem_fun(*this,&Dock_Canvases::on_row_activate));
	//canvas_tree->signal_event().connect(sigc::mem_fun(*this,&Dock_Canvases::on_tree_event));
	canvas_tree->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	canvas_tree->add_events(Gdk::BUTTON1_MOTION_MASK);
	canvas_tree->show();
	canvas_tree->set_headers_visible(false);

	Gtk::ScrolledWindow *scrolledwindow = manage(new class Gtk::ScrolledWindow());
	scrolledwindow->set_can_focus(true);
	scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolledwindow->add(*canvas_tree);
	scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
	scrolledwindow->show_all();

	return scrolledwindow;
}

etl::loose_handle<studio::CanvasView>
Dock_Canvases::get_selected_canvas_view()
{
	return get_selected_instance()->find_canvas_view(get_selected_canvas());
}

etl::loose_handle<synfig::Canvas>
Dock_Canvases::get_selected_canvas()
{
	Glib::RefPtr<Gtk::TreeSelection> selection=canvas_tree->get_selection();

	if(!selection || !selection->get_selected())
		return 0;

	studio::Instance::CanvasTreeModel canvas_tree_model;

	return static_cast<etl::handle<synfig::Canvas> >((*selection->get_selected())[canvas_tree_model.canvas]);
}



void
Dock_Canvases::set_selected_instance_signal(etl::handle<studio::Instance> x)
{
	set_selected_instance(x);
}

void
Dock_Canvases::set_selected_instance_(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	selected_instance=instance;
	if(instance)
	{
		canvas_tree->set_model(instance->canvas_tree_store());
		canvas_tree->show();
	}
	else
	{
		canvas_tree->set_model(Glib::RefPtr< Gtk::TreeModel >());
		canvas_tree->hide();
	}
}

void
Dock_Canvases::set_selected_instance(etl::loose_handle<studio::Instance> x)
{
	if(studio::App::shutdown_in_progress)
		return;

	// if it's already selected, don't select it again
	if (x==selected_instance)
		return;

	set_selected_instance_(x);
}

void
Dock_Canvases::new_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	assert(instance);

	etl::loose_handle<studio::Instance> loose_instance(instance);

	instance->synfigapp::Instance::signal_filename_changed().connect(sigc::mem_fun(*this,&Dock_Canvases::refresh_instances));
	instance->synfigapp::Instance::signal_filename_changed().connect(
		sigc::bind<etl::loose_handle<studio::Instance> >(
			sigc::mem_fun(*this,&Dock_Canvases::set_selected_instance),
			loose_instance
		)
	);
}

void
Dock_Canvases::delete_instance(etl::handle<studio::Instance> instance)
{
	if(studio::App::shutdown_in_progress)
		return;

	refresh_instances();

	if(selected_instance==instance)
	{
		set_selected_instance(0);
	}
}

void
Dock_Canvases::refresh_instances()
{
	if(studio::App::shutdown_in_progress)
		return;
}

void
Dock_Canvases::on_row_activate(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *)
{
	assert(get_selected_instance());
	studio::Instance::CanvasTreeModel canvas_tree_model;
	const Gtk::TreeRow row = *(get_selected_instance()->canvas_tree_store()->get_iter(path));
	if(row[canvas_tree_model.is_canvas])
		get_selected_instance()->focus(row[canvas_tree_model.canvas]);
	else
		studio::App::dialog_not_implemented();
}
