
#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "dialog_workspaces.h"

#include "gui/resourcehelper.h"
#include "gui/localization.h"
#include "gui/app.h"
#include "gui/workspacehandler.h"

#include <synfig/general.h>

#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/messagedialog.h>

#endif

using namespace studio;

static Glib::RefPtr<Gtk::Builder> load_interface() {
	auto refBuilder = Gtk::Builder::create();
	try
	{
		refBuilder->add_from_file(ResourceHelper::get_ui_path("dialog_workspaces.glade"));
	}
	catch(const Glib::FileError& ex)
	{
		synfig::error("FileError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Glib::MarkupError& ex)
	{
		synfig::error("MarkupError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	catch(const Gtk::BuilderError& ex)
	{
		synfig::error("BuilderError: " + ex.what());
		return Glib::RefPtr<Gtk::Builder>();
	}
	return refBuilder;
}

class WorkspaceCols: public Gtk::TreeModel::ColumnRecord {
    public:
        WorkspaceCols() {
            this->add(this->col_name);
        }

        Gtk::TreeModelColumn<Glib::ustring> col_name;
};

Dialog_Workspaces::Dialog_Workspaces(Gtk::Dialog::BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) :
	Gtk::Dialog(cobject),
	builder(refGlade),
	rename_button(nullptr),
	delete_button(nullptr)
{
	Gtk::Button *button = nullptr;

	refGlade->get_widget("workspaces_close_button", button);
	if (button)
		button->signal_clicked().connect(sigc::mem_fun(*this, &Gtk::Dialog::close));

	refGlade->get_widget("workspaces_delete_button", delete_button);
	if (delete_button)
		delete_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Workspaces::on_delete_clicked));

	refGlade->get_widget("workspaces_rename_button", rename_button);
	if (rename_button)
		rename_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_Workspaces::on_rename_clicked));

	Gtk::TreeView * workspace_treeview = nullptr;
	refGlade->get_widget("workspaces_treeview", workspace_treeview);
	if (workspace_treeview) {
		current_selection = workspace_treeview->get_selection();
		current_selection->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Workspaces::on_selection_changed));

		workspace_model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(
					refGlade->get_object("workspaces_liststore")
				);
//		WorkspaceCols ws_cols;
//		Gtk::TreeModel::Row row = *workspace_model->append();
//		row[ws_cols.col_name] = "ui";
//		workspace_model->append()->set_value(0, Glib::ustring("ui"));

		App::signal_custom_workspaces_changed().connect(sigc::mem_fun(*this, &Dialog_Workspaces::rebuild_list));

		rebuild_list();
	}
}

Dialog_Workspaces* Dialog_Workspaces::create(Gtk::Window& parent)
{
	auto refBuilder = load_interface();
	if (!refBuilder)
		return nullptr;
	Dialog_Workspaces * dialog = nullptr;
	refBuilder->get_widget_derived("dialog_workspaces", dialog);
	if (dialog) {
		dialog->set_transient_for(parent);
	}
	return dialog;
}

Dialog_Workspaces::~Dialog_Workspaces()
{

}

void Dialog_Workspaces::on_selection_changed()
{
	int count = current_selection->count_selected_rows();
	rename_button->set_sensitive(count == 1);
	delete_button->set_sensitive(count > 0);
}

void Dialog_Workspaces::on_delete_clicked()
{
	char msg[256];
	snprintf(msg, 255, _("Are you sure you want to delete %d workspaces?"), current_selection->count_selected_rows());
	Gtk::MessageDialog confirm_dlg(*this, msg, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
	if (confirm_dlg.run() == Gtk::RESPONSE_NO)
		return;

	// get_selected_rows() return TreePath not TreeIter
	// So, deleting an item, invalidates the other TreePaths (they point to wrong items)
	std::vector<std::string> names;
	for (auto selected_path : current_selection->get_selected_rows()) {
		std::string name;
		workspace_model->get_iter(selected_path)->get_value(0, name);
		names.push_back(name);
	}
	for (const std::string & name : names) {
		App::get_workspace_handler()->remove_workspace(name);
	}
}

void Dialog_Workspaces::on_rename_clicked()
{
	std::string old_name;
	auto selected_path = current_selection->get_selected_rows()[0];
	workspace_model->get_iter(selected_path)->get_value(0, old_name);

	Gtk::MessageDialog dialog(*this, _("Type a name for this custom workspace:"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE, true);
	dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	Gtk::Button * ok_button = dialog.add_button(_("Ok"), Gtk::RESPONSE_OK);
	ok_button->set_sensitive(false);

	Gtk::Entry * name_entry = Gtk::manage(new Gtk::Entry());
	name_entry->set_margin_left(16);
	name_entry->set_margin_right(16);
	name_entry->signal_changed().connect([&](){
		std::string name = name_entry->get_text();
		WorkspaceHandler::trim_string(name);
		bool has_equal_sign = name.find("=") != std::string::npos;
		ok_button->set_sensitive(!name.empty() && !has_equal_sign);
		if (ok_button->is_sensitive())
			ok_button->grab_default();
	});
	name_entry->signal_activate().connect(sigc::mem_fun(*ok_button, &Gtk::Button::clicked));
	name_entry->set_text(old_name);

	dialog.get_content_area()->set_spacing(12);
	dialog.get_content_area()->add(*name_entry);

	ok_button->set_can_default(true);

	dialog.show_all();

	int response = dialog.run();
	if (response == Gtk::RESPONSE_CANCEL)
		return;

	std::string name = name_entry->get_text();
	WorkspaceHandler::trim_string(name);

	if (old_name == name)
		return;

	if (App::get_workspace_handler()->has_workspace(name)) {
		Gtk::MessageDialog error_dlg(dialog, _("There is already a workspace with this name."), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
		error_dlg.run();
		return;
	}

	std::string tpl;
	App::get_workspace_handler()->get_workspace(old_name, tpl);
	App::get_workspace_handler()->remove_workspace(old_name);
	App::get_workspace_handler()->add_workspace(name, tpl);
}

void Dialog_Workspaces::rebuild_list()
{
	workspace_model->clear();

	WorkspaceHandler *workspaces = App::get_workspace_handler();
	std::vector<std::string> names;
	workspaces->get_name_list(names);
	for (const std::string & name : names)
		workspace_model->append()->set_value(0, name);
}
