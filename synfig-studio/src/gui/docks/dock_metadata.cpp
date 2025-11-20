/* === S Y N F I G ========================================================= */
/*!	\file dock_metadata.cpp
**	\brief Template File
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

#include "docks/dock_metadata.h"

#include <gtkmm/stylecontext.h>

#include <gui/app.h>
#include <gui/canvasview.h>
#include <gui/localization.h>
#include <gui/trees/metadatatreestore.h>
#include <gui/trees/metadatatree.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static Gtk::ToolButton*
create_action_toolbutton(const std::string& action_name, const std::string& icon_name, const std::string& tooltip)
{
	Gtk::ToolButton* button = Gtk::manage(new Gtk::ToolButton());
	gtk_actionable_set_action_name(GTK_ACTIONABLE(button->gobj()), action_name.c_str());
	button->set_icon_name(icon_name);
	button->set_tooltip_text(tooltip);
	button->show();
	return button;
}

/* === M E T H O D S ======================================================= */

Dock_MetaData::Dock_MetaData():
	Dock_CanvasSpecific("meta_data",_("Canvas MetaData"),"meta_data_icon")
{
	// Make Canvas MetaData toolbar small for space efficiency
	get_style_context()->add_class("synfigstudio-efficient-workspace");

	auto toolbar = Gtk::manage(new Gtk::Toolbar());
	toolbar->show_all();
	toolbar->append(*create_action_toolbutton("doc.add-metadata", "list-add", _("Add a new MetaData entry to the canvas")));
	toolbar->append(*create_action_toolbutton("doc.remove-metadata", "edit-delete", _("Remove the selected MetaData entry")));
	set_toolbar(*toolbar);
}

Dock_MetaData::~Dock_MetaData()
{
}

void
Dock_MetaData::init_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	Glib::RefPtr<MetaDataTreeStore> metadata_tree_store;
	metadata_tree_store=MetaDataTreeStore::create(canvas_view->canvas_interface());
	MetaDataTree* metadata_tree(new MetaDataTree());
	metadata_tree->set_model(metadata_tree_store);
	metadata_tree->set_editable(true);
	canvas_view->set_tree_model(get_name(),metadata_tree_store);
	canvas_view->set_ext_widget(get_name(),metadata_tree);
	// canvas_view->get_canvas()->signal_meta_data_changed().connect(sigc::mem_fun(*this, &Dock_MetaData::on_metadata_changed));

	auto action_group = Glib::RefPtr<Gio::SimpleActionGroup>::cast_dynamic(canvas_view->get_action_group("doc"));
	if (action_group) {
		auto action = action_group->add_action("add-metadata", sigc::mem_fun(*this, &Dock_MetaData::on_add_pressed));
		action->set_enabled(true);
		action = action_group->add_action("remove-metadata", sigc::mem_fun(*this, &Dock_MetaData::on_delete_pressed));
		action->set_enabled(true);
	}
}

void
Dock_MetaData::changed_canvas_view_vfunc(CanvasView::LooseHandle canvas_view)
{
	if (canvas_view) {
		Gtk::Widget* tree_view(canvas_view->get_ext_widget(get_name()));
		add(*tree_view);
		tree_view->show();

		// action_group->set_sensitive(true);
	} else {
		// action_group->set_sensitive(false);
	}
}

void
Dock_MetaData::on_add_pressed()
{
	if(get_canvas_interface())
	{
		synfig::String key;
		if(App::dialog_entry(_("New Metadata entry"), _("Key Name: "), key, _("Cancel"), _("Ok")) && !key.empty())
		{
			get_canvas_interface()->set_meta_data(key," ");
		}
	}
}

void
Dock_MetaData::on_delete_pressed()
{
	CanvasView::LooseHandle canvas_view(get_canvas_view());
	if(!canvas_view) return;
	Gtk::TreeView* tree_view(static_cast<Gtk::TreeView*>(canvas_view->get_ext_widget(get_name())));
	if(tree_view)
	{
		Gtk::TreeModel::iterator iter(tree_view->get_selection()->get_selected());
		if(tree_view->get_selection()->count_selected_rows())
		{
			Gtk::TreeRow row(*iter);
			Glib::RefPtr<Gtk::TreeModel> treemodel(canvas_view->get_tree_model(get_name()));
			Glib::RefPtr<studio::MetaDataTreeStore> meta_data_tree_store(Glib::RefPtr<studio::MetaDataTreeStore>::cast_dynamic(treemodel));
			Glib::ustring key(row[meta_data_tree_store->model.key]);
			if(get_canvas_interface())
				get_canvas_interface()->erase_meta_data(key);
		}
	}
}
