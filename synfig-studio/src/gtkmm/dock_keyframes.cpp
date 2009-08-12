/* === S Y N F I G ========================================================= */
/*!	\file dock_keyframes.cpp
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

#include "dock_keyframes.h"
#include "app.h"

#include <gtkmm/scrolledwindow.h>
#include <cassert>
#include "instance.h"
#include <sigc++/signal.h>
#include <sigc++/hide.h>
#include <sigc++/slot.h>
#include "keyframetreestore.h"
#include "keyframetree.h"
#include "canvasview.h"
#include "keyframeactionmanager.h"

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

Dock_Keyframes::Dock_Keyframes():
	Dock_CanvasSpecific("keyframes",_("Keyframes"),Gtk::StockID("synfig-keyframes")),
	action_group(Gtk::ActionGroup::create()),
	keyframe_action_manager(new KeyframeActionManager)
{
	keyframe_action_manager->set_ui_manager(App::ui_manager());
	keyframe_action_manager->signal_show_keyframe_properties().connect(
		sigc::mem_fun(*this,&Dock_Keyframes::show_keyframe_properties)
	);
/*	add_button(
		Gtk::StockID("gtk-add"),
		_("Inserts a Keyframe at the current time")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Keyframes::add_keyframe_pressed
		)
	);

	add_button(
		Gtk::StockID("synfig-duplicate"),
		_("Duplicates the selected keyframe at the current time")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Keyframes::duplicate_keyframe_pressed
		)
	);

	add_button(
		Gtk::StockID("gtk-delete"),
		_("Deletes the selected Keyframe")
	)->signal_clicked().connect(
		sigc::mem_fun(
			*this,
			&Dock_Keyframes::delete_keyframe_pressed
		)
	);
*/
    Glib::ustring ui_info =
	"<ui>"
	"	<toolbar action='toolbar-keyframe'>"
	"	<toolitem action='action-KeyframeAdd' />"
	"	<toolitem action='action-KeyframeDuplicate' />"
	"	<toolitem action='action-KeyframeRemove' />"
	"	<toolitem action='keyframe-properties' />"
	"	</toolbar>"
	"</ui>"
	;

	App::ui_manager()->add_ui_from_string(ui_info);

	set_toolbar(*dynamic_cast<Gtk::Toolbar*>(App::ui_manager()->get_widget("/toolbar-keyframe")));
}

Dock_Keyframes::~Dock_Keyframes()
{
}

void
Dock_Keyframes::show_keyframe_properties()
{
	if(get_canvas_view())
		get_canvas_view()->show_keyframe_dialog();
}

/*
void
Dock_Keyframes::add_keyframe_pressed()
{
	if(get_canvas_view())
		get_canvas_view()->on_keyframe_add_pressed();
}

void
Dock_Keyframes::duplicate_keyframe_pressed()
{
	if(get_canvas_view())
		get_canvas_view()->on_keyframe_duplicate_pressed();
}

void
Dock_Keyframes::delete_keyframe_pressed()
{
	if(get_canvas_view())
		get_canvas_view()->on_keyframe_remove_pressed();
}
*/

void
Dock_Keyframes::init_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	Glib::RefPtr<KeyframeTreeStore> keyframe_tree_store;
	keyframe_tree_store=KeyframeTreeStore::create(canvas_view->canvas_interface());

	KeyframeTree* keyframe_tree(new KeyframeTree());
	keyframe_tree->set_model(keyframe_tree_store);
	keyframe_tree->set_editable(true);

	canvas_view->set_tree_model(get_name(),keyframe_tree_store);
	canvas_view->set_ext_widget(get_name(),keyframe_tree);
}

void
Dock_Keyframes::changed_canvas_view_vfunc(etl::loose_handle<CanvasView> canvas_view)
{
	if(canvas_view)
	{
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));

		add(*tree_view);
		tree_view->show();

		keyframe_action_manager->set_keyframe_tree(dynamic_cast<KeyframeTree*>(canvas_view->get_ext_widget(get_name())));
		keyframe_action_manager->set_canvas_interface(canvas_view->canvas_interface());
		keyframe_action_manager->refresh();
	}
	else
	{
		clear_previous();

		keyframe_action_manager->set_keyframe_tree(0);
		keyframe_action_manager->set_canvas_interface(0);
		keyframe_action_manager->refresh();
	}
}
