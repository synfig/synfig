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
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/fontbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>
#include <gtkmm/notebook.h>
#include <gtkmm/scale.h>
#include <gtkmm/scalebutton.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/switch.h>
#include <gtkmm/volumebutton.h>

#include <synfig/general.h>
#include <synfig/os.h>

#include <synfigapp/main.h>

#include <gui/localization.h>
#include <gui/app.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

static void set_widget_value(Gtk::Widget* widget, const std::string& value) {
	if (!widget)
		return;
    
	if (GTK_IS_COMBO_BOX_TEXT(widget->gobj())) {
		auto* combo = static_cast<Gtk::ComboBoxText*>(widget);
		if (combo->get_has_entry()) {
			if (auto entry = combo->get_entry()) {
				entry->set_text(value);
			}
		} else {
			combo->set_active_id(value);
		}
	} else if (GTK_IS_COMBO_BOX(widget->gobj())) {
		auto* combo = static_cast<Gtk::ComboBox*>(widget);
		if (combo->get_has_entry()) {
			if (auto entry = combo->get_entry()) {
				entry->set_text(value);
				entry->set_editable(true);
			}
		} else {
			combo->set_active_id(value);
		}
	} else if (GTK_IS_SWITCH(widget->gobj())) {
		static_cast<Gtk::Switch*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_CHECK_BUTTON(widget->gobj())) {
		static_cast<Gtk::CheckButton*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_TOGGLE_BUTTON(widget->gobj())) {
		static_cast<Gtk::ToggleButton*>(widget)->set_active(value == "1" || value == "true");
	} else if (GTK_IS_FILE_CHOOSER_BUTTON(widget->gobj())) {
		static_cast<Gtk::FileChooserButton*>(widget)->set_filename(value);
	} else if (GTK_IS_COLOR_BUTTON(widget->gobj())) {
		Gdk::RGBA color;
		color.set(value);
		static_cast<Gtk::ColorButton*>(widget)->set_rgba(color);
	} else if (GTK_IS_FONT_BUTTON(widget->gobj())) {
		static_cast<Gtk::FontButton*>(widget)->set_font_name(value);
	} else if (GTK_IS_SCALE_BUTTON(widget->gobj())) {
		static_cast<Gtk::ScaleButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_VOLUME_BUTTON(widget->gobj())) {
		static_cast<Gtk::VolumeButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_SCALE(widget->gobj())) {
		static_cast<Gtk::Scale*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_SPIN_BUTTON(widget->gobj())) {
		static_cast<Gtk::SpinButton*>(widget)->set_value(std::stod(value));
	} else if (GTK_IS_ENTRY(widget->gobj())) {
		static_cast<Gtk::Entry*>(widget)->set_text(value);
	}
}

static void hydrate_config(Gtk::Container* container, const std::map<std::string, std::string>& values) {
	if (!container)
		return;

	// Process all children

	std::vector<Gtk::Widget*> children = container->get_children();
	for (Gtk::Widget* child : children) {
		if (!child->get_name().empty()) {
			auto it = values.find(child->get_name());
			if (it != values.end())
				set_widget_value(child, it->second);
		}

		// Recursively process child containers
		if (GTK_IS_CONTAINER(child->gobj()))
			hydrate_config(dynamic_cast<Gtk::Container*>(child), values);
	}
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
	auto config_data = PluginManager::parse_dialog(*config_widget);

	// Convert to JSON format
	std::string json_data;
	for (const auto& d : config_data) {
		if (!json_data.empty())
			json_data.push_back(',');
		json_data += synfig::strprintf("\"%s\":\"%s\"",
			JSON::escape_string(d.first).c_str(),
			JSON::escape_string(d.second).c_str());
	}
	json_data = "{" + json_data + "}";

	// Save to user_config.json using FileSystemNative
	try {
		auto file_system = FileSystemNative::instance();
		auto stream = file_system->get_write_stream(user_config_file.u8string());
		if (!stream) {
			throw std::runtime_error(_("Could not open file for writing"));
		}
		stream->write(json_data.c_str(), json_data.size());

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
Dialog_PluginManager::reset_plugin_config(const std::string& plugin_id, Gtk::Widget* config_widget)
{
	if (!config_widget)
		return;

	// Ask for confirmation
	confirmation_dialog.set_message(_("Are you sure you want to reset all settings to default?"));
	int response = confirmation_dialog.run();
	confirmation_dialog.hide();

	if (response == Gtk::RESPONSE_OK) {
		Plugin plugin = App::plugin_manager.get_plugin(plugin_id);
		if (!plugin.is_valid())
			return;

		const filesystem::Path default_config_file = plugin.default_config_filepath();
		const filesystem::Path user_config_file = plugin.user_config_filepath();

		try {
			auto file_system = FileSystemNative::instance();

			// Check if default config exists
			if (!file_system->is_file(default_config_file.u8string())) {
				message_dialog.set_message(_("Error: Default configuration file not found"));
				message_dialog.run();
				message_dialog.hide();
				return;
			}

			// Copy default_config.json to user_config.json using FileSystemNative
			if (!synfig::FileSystem::copy(file_system, default_config_file.u8string(), file_system, user_config_file.u8string())) {
				throw std::runtime_error(_("Could not restore default configuration file"));
			}

			message_dialog.set_message(_("Configuration reset to defaults successfully"));
			message_dialog.run();
			message_dialog.hide();

			// Reload the configuration UI
			build_notebook();
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
		const filesystem::Path default_config_file = plugin.default_config_filepath();
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
			// Configuration UI exists, check for default config
			if (!file_system->is_file(default_config_file.u8string())) {
				// Show error if default config is missing
				Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
					_("Configuration cannot be loaded - missing default_config.json")
				));
				error_label->set_margin_top(20);
				error_label->set_margin_bottom(20);
				error_label->set_margin_left(20);
				error_label->set_margin_right(20);
				plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
			} else {
				try {
					auto builder = Gtk::Builder::create();
					try {
						builder->add_from_file(config_ui_file.u8string());
					} catch (const Glib::FileError& ex) {
						throw std::runtime_error(ex.what());
					}
					Gtk::Widget* config_widget = nullptr;
					builder->get_widget("dialog_contents", config_widget);
					synfig::FileSystemNative::Handle native_fs = synfig::FileSystemNative::instance();
					if (!native_fs->is_file(user_config_file.u8string())) {
						// Copy default config to user config using native filesystem
						if (!synfig::FileSystem::copy(native_fs, default_config_file.u8string(), native_fs, user_config_file.u8string())) {
							throw std::runtime_error(_("Could not create user configuration file"));
						}
					}

					// Now try to read the user config file (which should always exist)
					auto stream = native_fs->get_read_stream(user_config_file.u8string());
					if (!stream) {
						throw std::runtime_error(_("Could not open user configuration file for reading"));
					}
					std::stringstream buffer;
					buffer << stream->rdbuf();
					std::string config_data = buffer.str();
					auto values = JSON::Parser::parse(config_data);
					if (config_widget && GTK_IS_CONTAINER(config_widget->gobj())) {
						auto container = static_cast<Gtk::Container*>(config_widget);
						hydrate_config(container, values);
						synfig::info("Hydrated");
						// If configuration UI was found, add it to the tab
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
							plugin.id, config_widget));

						button_box->pack_end(*save_button, Gtk::PACK_SHRINK);
						button_box->pack_end(*reset_button, Gtk::PACK_SHRINK);
						plugin_tab->pack_end(*button_box, Gtk::PACK_SHRINK);
					} else {
						// Show error if dialog_contents not found
						Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
							_("Error: Configuration UI file found but missing 'dialog_contents' element")));
						plugin_label->set_margin_bottom(10);
						plugin_label->set_margin_top(10);
						plugin_label->set_margin_left(20);
						plugin_label->set_margin_right(20);
						plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
					}
				}
				catch (const Glib::Error& ex) {
					// Show error if UI file couldn't be loaded
					Gtk::Label* error_label = Gtk::manage(new Gtk::Label(
						_("Error loading configuration UI: ") + ex.what()));
					error_label->set_margin_bottom(20);
					error_label->set_margin_top(20);
					error_label->set_margin_left(20);
					error_label->set_margin_right(20);
					error_label->set_selectable(true);
					plugin_tab->pack_start(*error_label, Gtk::PACK_SHRINK);
				}
				catch (const std::exception& ex) {
					synfig::error(_("Error loading configuration UI: %s"), ex.what());
				}
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
