/*!	\file gui/dialogs/dialog_workspaces.h
**	\brief Dialog for handling custom workspace list
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

#ifndef SYNFIG_STUDIO_DIALOG_WORKSPACES_H
#define SYNFIG_STUDIO_DIALOG_WORKSPACES_H

#include <gtkmm/dialog.h>
#include <gtkmm/builder.h>

namespace Gtk {
class Button;
class TreeSelection;
class ListStore;
}

namespace studio
{

class Dialog_Workspaces : public Gtk::Dialog
{
	const Glib::RefPtr<Gtk::Builder> builder;

	Gtk::Button * rename_button;
	Gtk::Button * delete_button;

	Glib::RefPtr<Gtk::TreeSelection> current_selection;
	Glib::RefPtr<Gtk::ListStore> workspace_model;

public:
	Dialog_Workspaces(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	static Dialog_Workspaces * create(Gtk::Window& parent);
	~Dialog_Workspaces();

private:
	void on_selection_changed();
	void on_delete_clicked();
	void on_rename_clicked();

	void rebuild_list();
};

}

#endif // SYNFIG_STUDIO_DIALOG_WORKSPACES_H
