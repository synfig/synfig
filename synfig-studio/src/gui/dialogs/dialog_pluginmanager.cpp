/* === S Y N F I G ========================================================= */
/*!	\file dialog_pluginmanager.cpp
**	\brief Plugin Manager Dialog
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

#include "dialog_pluginmanager.h"

#include <libxml++/libxml++.h>
#include <glibmm/fileutils.h>
#include <gtkmm/builder.h>
#include <gtkmm/scrolledwindow.h>

#include <synfig/general.h>
#include <synfig/os.h>

#include <synfigapp/main.h>

#include <gui/localization.h>
#include <gui/app.h>
#include <gui/json.h>
#include <gui/json_to_dialog_converter.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

enum class ValueType
{
	DEFAULT,
	SAVED,
};

struct MetaValues
{
	std::string default_value;
	std::string saved_value;
};

/* === P R O C E D U R E S ================================================= */

static void*
delete_meta_value(void* pdata)
{
	delete static_cast<MetaValues*>(pdata);
	return nullptr;
}

static void
set_meta_values_in_widget(Gtk::Widget& widget, const std::map<std::string, std::string>& values, ValueType value_type)
{
	auto it = values.find(widget.get_name());
	if (it != values.end()) {
		void* pdata = widget.get_data("meta_values");
		if (!pdata) {
			pdata = new MetaValues{};
			widget.set_data("meta_values", pdata);
			widget.add_destroy_notify_callback(pdata, delete_meta_value);
		}
		if (value_type == ValueType::DEFAULT)
			static_cast<MetaValues*>(pdata)->default_value = it->second;
		else if (value_type == ValueType::SAVED)
			static_cast<MetaValues*>(pdata)->saved_value = it->second;
	}

	if (GTK_IS_CONTAINER(widget.gobj())) {
		for (auto child : static_cast<Gtk::Container*>(&widget)->get_children()) {
			if (child)
				set_meta_values_in_widget(*child, values, value_type);
		}
	}
}

static void
get_meta_values_in_widget(Gtk::Widget& widget, std::map<std::string, std::string>& values, ValueType value_type)
{
	std::string name = widget.get_name();
	if (!name.empty()) {
		void* pdata = widget.get_data("meta_values"); // widget cannot be const o.O
		if (pdata) {
			if (value_type == ValueType::DEFAULT)
				values[name] = static_cast<MetaValues*>(pdata)->default_value;
			else if (value_type == ValueType::SAVED)
				values[name] = static_cast<MetaValues*>(pdata)->saved_value;
		}
	}

	if (GTK_IS_CONTAINER(widget.gobj())) {
		for (auto child : static_cast<Gtk::Container*>(&widget)->get_children()) {
			if (child)
				get_meta_values_in_widget(*child, values, value_type);
		}
	}
}

static Gtk::Container*
get_plugin_tab_content_widget(Gtk::Notebook& notebook, const std::string& plugin_name)
{
	const int n_pages = notebook.get_n_pages();
	for (int i = 0; i < n_pages; ++i) {
		auto widget = notebook.get_nth_page(i);
		if (notebook.get_tab_label_text(*widget) == plugin_name) {
			return dynamic_cast<Gtk::Container*>(widget);
		}
	}
	return nullptr;
}

/* === M E T H O D S ======================================================= */

Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    plugin_file_dialog(*this, "Select plugin zip file", Gtk::FILE_CHOOSER_ACTION_OPEN),
    message_dialog(*this, _(""), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true),
    confirmation_dialog(*this, _("Are you sure you want to delete this plugin?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true),
    plugin_list(App::plugin_manager.plugins())
{
	plugin_file_dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
	plugin_file_dialog.add_button(_("Open"), Gtk::RESPONSE_OK);

	// Add a filter to show only text files
	auto filter_zip = Gtk::FileFilter::create();
	filter_zip->set_name(_("Zip File (A synfig plugin zip file)"));
	filter_zip->add_mime_type("application/zip");
	plugin_file_dialog.add_filter(filter_zip);

	App::plugin_manager.signal_list_changed().connect(sigc::mem_fun(*this, &Dialog_PluginManager::refresh));
	// this->set_resizable(false);
	confirmation_dialog.set_transient_for(*this);
	set_resizable(false);
	set_default_size(800, 600);

	build_listbox();


	build_notebook();

	Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
	scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	scrolled_window->add(notebook);
	get_content_area()->pack_start(*scrolled_window);
	set_default_size(1200, 600);
	show_all_children();
}

Dialog_PluginManager::~Dialog_PluginManager()
{
}

void
Dialog_PluginManager::refresh()
{
	plugin_list = App::plugin_manager.plugins();
	auto children = plugin_list_box.get_children();
	for (Gtk::Widget* child : children) {
		plugin_list_box.remove(*child);
		delete child;
	}

	build_listbox();
	build_notebook();
}

void
Dialog_PluginManager::save_plugin_config(const std::string& plugin_id, Gtk::Widget* config_widget)
{
	if (!config_widget)
		return;

	Plugin plugin = App::plugin_manager.get_plugin(plugin_id);
	if (!plugin.is_valid())
		return;

	// Construct file paths
	const filesystem::Path user_config_file = plugin.user_config_filepath();

	// Get configuration data from widgets
	auto config_data = JSON::parse_dialog(*config_widget);

	// Convert to JSON format
	const std::string json_data = JSON::stringify(config_data);

	// Save to user_config.json using FileSystemNative
	try {
		auto file_system = FileSystemNative::instance();
		auto stream = file_system->get_write_stream(user_config_file.u8string());
		if (!stream) {
			throw std::runtime_error(_("Could not open file for writing"));
		}
		stream->write(json_data.c_str(), json_data.size());

		set_meta_values_in_widget(*config_widget, config_data, ValueType::SAVED);

		message_dialog.set_message(_("Configuration saved successfully"));
		message_dialog.run();
		message_dialog.hide();
	} catch (const std::exception& ex) {
		message_dialog.set_message(strprintf(_("Error saving configuration: %s"), ex.what()));
		message_dialog.run();
		message_dialog.hide();
	}
}

void
Dialog_PluginManager::reset_plugin_config(const std::string& plugin_id)
{
	// Ask for confirmation
	confirmation_dialog.set_message(_("Are you sure you want to reset all settings to default?"));
	int response = confirmation_dialog.run();
	confirmation_dialog.hide();

	if (response == Gtk::RESPONSE_OK) {
		Plugin plugin = App::plugin_manager.get_plugin(plugin_id);
		if (!plugin.is_valid())
			return;

		const filesystem::Path user_config_file = plugin.user_config_filepath();

		try {
			auto file_system = FileSystemNative::instance();

			if (file_system->is_file(user_config_file.u8string()))
				file_system->remove_recursive(user_config_file.u8string());

			// Reload the configuration UI
			Gtk::Container* container = get_plugin_tab_content_widget(notebook, plugin.name.get());
			if (container) {
				std::map<std::string, std::string> default_values;
				get_meta_values_in_widget(*container, default_values, ValueType::DEFAULT);

				JSON::hydrate_dialog(container, default_values);
			}

			message_dialog.set_message(_("Configuration reset to defaults successfully"));
			message_dialog.run();
			message_dialog.hide();

		} catch (const std::exception& ex) {
			message_dialog.set_message(strprintf(_("Error resetting configuration: %s"), ex.what()));
			message_dialog.run();
			message_dialog.hide();
		}
	}
}

void
Dialog_PluginManager::build_notebook()
{
	int activePage = notebook.get_current_page();
	while (notebook.get_n_pages() > 0) {
		notebook.remove_page(-1);
	}

	Gtk::ScrolledWindow* installed_plugins_scroll = Gtk::manage(new Gtk::ScrolledWindow());
	installed_plugins_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

	Gtk::VBox* installed_plugins_tab = Gtk::manage(new Gtk::VBox());
	installed_plugins_tab->property_expand().set_value(false);

	Gtk::Button* install_plugin_button = manage(new Gtk::Button(_("Install Plugin")));
	install_plugin_button->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
	install_plugin_button->set_always_show_image(true);
	install_plugin_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PluginManager::on_install_plugin_button_clicked));
	plugin_list_box.set_selection_mode(Gtk::SELECTION_NONE);

	installed_plugins_scroll->add(*installed_plugins_tab);

	Gtk::Label* installed_plugins_label = Gtk::manage(new Gtk::Label(_("Installed Plugins")));
	installed_plugins_label->set_margin_bottom(10);
	installed_plugins_label->set_margin_top(10);
	installed_plugins_label->set_margin_left(20);
	installed_plugins_label->set_margin_right(20);
	notebook.append_page(*installed_plugins_scroll, *installed_plugins_label);

	installed_plugins_tab->pack_start(plugin_list_box, Gtk::PACK_SHRINK);
	installed_plugins_tab->pack_end(*install_plugin_button, Gtk::PACK_SHRINK);

	auto file_system = FileSystemNative::instance();

	for (const auto& plugin : plugin_list) {

		Gtk::ScrolledWindow* plugin_scroll = Gtk::manage(new Gtk::ScrolledWindow());
		plugin_scroll->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

		// Create a new tab content for each plugin
		Gtk::VBox* plugin_tab = Gtk::manage(new Gtk::VBox());
		plugin_tab->property_expand().set_value(false);


		// Create the label for the tab
		Gtk::Label* plugin_label = Gtk::manage(new Gtk::Label(plugin.name.get()));
		plugin_label->set_margin_bottom(10);
		plugin_label->set_margin_top(10);
		plugin_label->set_margin_left(20);
		plugin_label->set_margin_right(20);

		const filesystem::Path config_ui_file = plugin.config_ui_filepath();
		const filesystem::Path user_config_file = plugin.user_config_filepath();

		if (!file_system->is_file(config_ui_file.u8string())) {
			// If configuration UI doesn't exist, show simple message that no configuration is available
			Gtk::Label* info_label = Gtk::manage(new Gtk::Label(
				_("This plugin doesn't have any configuration available (configuration file doesn't exist)")
			));
			info_label->set_margin_top(20);
			info_label->set_margin_bottom(20);
			info_label->set_margin_left(20);
			info_label->set_margin_right(20);
			plugin_tab->pack_start(*info_label, Gtk::PACK_SHRINK);
		} else {
			// Configuration UI exists

			std::string error_msg;

			try {
				auto builder = Gtk::Builder::create();
				builder->add_from_file(config_ui_file.u8string());

				Gtk::Widget* config_widget = nullptr;
				builder->get_widget("dialog_contents", config_widget);
				if (config_widget && GTK_IS_CONTAINER(config_widget->gobj())) {
					// Parse the default values set in configuration.ui file
					auto default_values = JSON::parse_dialog(*config_widget);

					// Fetch the stored user settings in user_config.json, if it exists
					auto user_saved_values = JSON::Parser::parse(user_config_file);

					set_meta_values_in_widget(*config_widget, default_values, ValueType::DEFAULT);
					set_meta_values_in_widget(*config_widget, user_saved_values, ValueType::SAVED);

					// Fill dialog with user settings
					if (!user_saved_values.empty()) {
						auto container = static_cast<Gtk::Container*>(config_widget);
						JSON::hydrate_dialog(container, user_saved_values);
					}

					plugin_tab->pack_start(*config_widget, Gtk::PACK_SHRINK);

					// Add Save and Reset buttons
					Gtk::HBox* button_box = Gtk::manage(new Gtk::HBox());
					button_box->set_spacing(10);
					button_box->set_margin_top(10);
					button_box->set_margin_bottom(10);
					button_box->set_margin_left(20);
					button_box->set_margin_right(20);

					Gtk::Button* save_button = Gtk::manage(new Gtk::Button(_("Save Configuration")));
					Gtk::Button* reset_button = Gtk::manage(new Gtk::Button(_("Reset to Defaults")));

					save_button->set_image_from_icon_name("document-save", Gtk::ICON_SIZE_BUTTON);
					reset_button->set_image_from_icon_name("edit-undo", Gtk::ICON_SIZE_BUTTON);

					save_button->set_always_show_image(true);
					reset_button->set_always_show_image(true);

					save_button->signal_clicked().connect(
						sigc::bind(sigc::mem_fun(*this, &Dialog_PluginManager::save_plugin_config),
						plugin.id, config_widget));
					reset_button->signal_clicked().connect(
						sigc::bind(sigc::mem_fun(*this, &Dialog_PluginManager::reset_plugin_config),
						plugin.id));

					button_box->pack_end(*save_button, Gtk::PACK_SHRINK);
					button_box->pack_end(*reset_button, Gtk::PACK_SHRINK);
					plugin_tab->pack_end(*button_box, Gtk::PACK_SHRINK);
				} else {
					// Show error if dialog_contents not found
					error_msg = _("Error: Configuration UI file found but missing 'dialog_contents' element");
				}
			} catch (const Glib::Error& ex) {
				// Show error if UI file couldn't be loaded
				error_msg = _("Error loading configuration UI: ") + ex.what();
			} catch (const std::exception& ex) {
				error_msg = strprintf(_("Error loading configuration UI: %s"), ex.what());
			}

			if (!error_msg.empty()) {
				Gtk::Label* error_label = Gtk::manage(new Gtk::Label(error_msg));
				error_label->set_margin_bottom(20);
				error_label->set_margin_top(20);
				error_label->set_margin_left(20);
				error_label->set_margin_right(20);
				error_label->set_selectable(true);
				plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
			}
		}
		// Add the tab to the notebook
		plugin_scroll->add(*plugin_tab);
		notebook.append_page(*plugin_scroll, *plugin_label);

	}

	notebook.show_all_children();
	notebook.set_current_page(activePage == -1 ? 0 : activePage);
	notebook.set_tab_pos(Gtk::POS_LEFT);
}
void
Dialog_PluginManager::build_listbox()
{
	for (const auto& plugin : plugin_list) {
		Gtk::HBox* plugin_list_item = manage(new Gtk::HBox());
		Gtk::Label* plugin_name = manage(new Gtk::Label());

		Gtk::HBox* plugin_option_box = manage(new Gtk::HBox());
		Gtk::Button* restore_settings = manage(new Gtk::Button());
		Gtk::Button* open_folder = manage(new Gtk::Button());
		Gtk::Button* delete_plugin = manage(new Gtk::Button());

		plugin_list_item->property_expand().set_value(false);
		plugin_list_item->set_margin_top(10);
		plugin_option_box->property_expand().set_value(false);

		restore_settings->set_image_from_icon_name("view-refresh", Gtk::ICON_SIZE_BUTTON);
		open_folder->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
		delete_plugin->set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);

		restore_settings->set_tooltip_text(_("Restore default plugin settings"));
		open_folder->set_tooltip_text(_("Open plugin folder"));
		delete_plugin->set_tooltip_text(_("Remove plugin"));

		delete_plugin->signal_clicked().connect([plugin, this ](){
			this->confirmation_dialog.set_message(strprintf(_("Do you want to delete the plugin: %s"), plugin.name.get().c_str()));
			int response = this->confirmation_dialog.run();
			if (response == Gtk::RESPONSE_OK) {
				App::plugin_manager.remove_plugin(plugin.id);
				this->confirmation_dialog.hide();
				this->message_dialog.set_message(_("Plugin uninstalled successfully"));
				this->message_dialog.run();
				this->message_dialog.close();
			} else {
				this->confirmation_dialog.close();
			}
		});

		open_folder->signal_clicked().connect([plugin]() {
			synfig::OS::launch_file_async(plugin.dir);
		});

		restore_settings->signal_clicked().connect(
			sigc::bind(
				sigc::mem_fun(*this, &Dialog_PluginManager::reset_plugin_config),
				plugin.id
			)
		);

		plugin_option_box->pack_start(*restore_settings, Gtk::PACK_SHRINK, 10);
		plugin_option_box->pack_start(*open_folder, Gtk::PACK_SHRINK, 10);
		plugin_option_box->pack_start(*delete_plugin, Gtk::PACK_SHRINK, 10);
		plugin_option_box->set_margin_top(10);
		plugin_option_box->set_margin_bottom(10);

		plugin_name->set_text(plugin.name.get());

		plugin_list_item->pack_start(*plugin_name, Gtk::PACK_SHRINK, 20);
		plugin_list_item->pack_end(*plugin_option_box, Gtk::PACK_SHRINK, 20);

		plugin_list_box.add(*plugin_list_item);

	}
	plugin_list_box.show_all_children();
}

std::string extract_plugin_name(std::istream& fileStream)
{
	xmlpp::DomParser parser;
	parser.set_substitute_entities();
	parser.parse_stream(fileStream);

	const xmlpp::Node* pNode = parser.get_document()->get_root_node();
	if (std::string(pNode->get_name()) != std::string("plugin") ) {
		synfig::warning(_("Invalid plugin.xml file (missing root <plugin>)"));
		return std::string("");
	}

	auto execlist = pNode->find("./exec");
	if (!execlist.empty()) {
		for (const xmlpp::Node* node : pNode->find("./name") ) {
			const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
			std::string lang = element->get_attribute_value("lang");
			if (lang.empty()) {
				return element->get_child_text()->get_content();
			}
		}
	}

	return "";
}

void
Dialog_PluginManager::on_install_plugin_button_clicked()
{
	// Show the dialog and wait for a user response
	const int result = plugin_file_dialog.run();
	if (result != Gtk::RESPONSE_OK) {
		plugin_file_dialog.close();
		return;
	}
	std::vector<std::string> files;
	synfig::FileSystemNative::Handle native_fs = FileSystemNative::instance();
	synfig::FileContainerZip::Handle zip_fs = new FileContainerZip();
	const filesystem::Path zip_filename = plugin_file_dialog.get_filename();
	const filesystem::Path path_to_user_plugins = synfigapp::Main::get_user_app_directory() / filesystem::Path("plugins");
	std::string plugin_metadata_file;

	if (!zip_fs->open(zip_filename.u8string())) {
		const std::string error_message = strprintf(_("Failed to open plugin zip file: %s"), zip_filename.u8_str());
		synfig::error(error_message);
		message_dialog.set_message(error_message);
		message_dialog.run();
		message_dialog.close();
		plugin_file_dialog.close();
		return;
	}
	// find plugin.xml in root dir or inside first-level directory
	std::string folder_name_in_zip;
	zip_fs->directory_scan("", files);
	for (const auto& file : files) {
		if (file == "plugin.xml") {
			plugin_metadata_file = file;
			break;
		} else if (zip_fs->is_directory(file)) {
			std::vector<std::string> child_files;
			if (zip_fs->directory_scan(file, child_files)) {
				auto it = std::find(child_files.begin(), child_files.end(), "plugin.xml");
				if (it != child_files.end()) {
					plugin_metadata_file = file + "/plugin.xml" ;
					folder_name_in_zip = file;
					break;
				}
			}
		}
	}
	if (plugin_metadata_file.empty() || !zip_fs->is_file(plugin_metadata_file)) {
		const std::string error_message = strprintf(_("Failed to find plugin.xml in zip file: %s"), zip_filename.u8_str());
		synfig::error(error_message);
		message_dialog.set_message(error_message);
		message_dialog.run();
		message_dialog.close();
		plugin_file_dialog.close();
		return;
	}
	// Get plugin name from metadata file (it will be the plugin installation folder name)
	const std::string plugin_name = extract_plugin_name(*zip_fs->get_read_stream(plugin_metadata_file));
	if (plugin_name.empty()) {
		const std::string error_message = strprintf(_("Plugin package is invalid: couldn't find \"name\" tag in metadata file: %s"), zip_filename.u8_str());
		synfig::error(error_message);
		message_dialog.set_message(error_message);
		message_dialog.run();
		message_dialog.close();
		plugin_file_dialog.close();
		return;
	}
	// TODO: Check if plugin is already installed:
	// - Compare versions
	// - Ask about the user settings file
	// Check if folder with same name already exists
	const filesystem::Path output_path = path_to_user_plugins / plugin_name;
	if (native_fs->is_directory(output_path.u8string())) {
		confirmation_dialog.set_message(_("Plugin already exists. Do you want to overwrite it?"));
		int response = confirmation_dialog.run();
		if (response != Gtk::RESPONSE_OK) {
			confirmation_dialog.close();
			plugin_file_dialog.close();
			return;
		}
		auto it = std::find_if(App::plugin_manager.plugins().begin(), App::plugin_manager.plugins().end(), [plugin_name](const Plugin& plugin) { return plugin.name.fallback() == plugin_name; });
		if (it != App::plugin_manager.plugins().end()) {
			App::plugin_manager.remove_plugin(it->id);
		}
		native_fs->remove_recursive(output_path);
		confirmation_dialog.close();
	} else if (native_fs->is_file(output_path.u8string())) {
		// There is a weird file with this name in the folder for installed plugins
		confirmation_dialog.set_message(_("There is a file with the same name in the folder for installed plugins.\nInstalling this plugin will remove this file.\nDo you want to continue the installation?"));
		int response = confirmation_dialog.run();
		if (response != Gtk::RESPONSE_OK) {
			confirmation_dialog.close();
			plugin_file_dialog.close();
			return;
		}
		if (!native_fs->file_remove(output_path.u8string())) {
			const std::string error_message = strprintf(_("Failed to remove file: %s"), output_path.u8_str());
			synfig::error(error_message);
			message_dialog.set_message(error_message);
			message_dialog.run();
			message_dialog.close();
			plugin_file_dialog.close();
			return;
		}
	}
	// Do install
	if (native_fs->directory_create(output_path.u8string())) {
		if (folder_name_in_zip.empty()) {
			for (const auto& file : files) {
				zip_fs->copy_recursive(zip_fs, file, native_fs, (output_path / file).u8string());
			}
		} else {
			zip_fs->copy_recursive(zip_fs, folder_name_in_zip, native_fs, output_path.u8string());
		}
	}
	App::plugin_manager.load_plugin(output_path / filesystem::Path{"plugin.xml"}, output_path, true);
	zip_fs->close();
	Gtk::MessageDialog msg_dialog = Gtk::MessageDialog(plugin_file_dialog, _("Plugin installed successfully."), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true);
	msg_dialog.run();
	plugin_file_dialog.close();
}
