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
	const Glib::RefPtr<Gtk::Builder>& builder;

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
