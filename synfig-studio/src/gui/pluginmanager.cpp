/* === S Y N F I G ========================================================= */
/*!	\file gui/pluginmanager.cpp
**	\brief Plugin Manager responsible for loading plugins
**
**	\legal
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

#include <gui/pluginmanager.h>

#include <libxml++/libxml++.h>
#include <glibmm/fileutils.h>
#include <glibmm/markup.h>
#include <glibmm/miscutils.h>
#include <glibmm/spawn.h>

#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>

#include <gui/app.h>
#include <gui/localization.h>
#include <gui/onemoment.h>

#include <synfig/general.h>
#include <synfig/os.h>

#include <gui/json.h>
#include <gui/json_to_dialog_converter.h>

#endif

// Autodelete the file
struct TmpFile
{
	synfig::filesystem::Path filename;

	TmpFile(const void* random_ptr, const std::string& tag, const std::string& extension)
	{
		std::string file_tag = synfig::strprintf("plugin-%p-%s", random_ptr, tag.c_str());
		filename = synfig::FileSystemTemporary::generate_system_temporary_filename(file_tag, extension);
	}
	~TmpFile()
	{
		synfig::FileSystemNative::instance()->file_remove(filename.u8string());
	}
};

static bool
parse_boolean_string(const std::string& str)
{
	auto s = synfig::trim(str);
	synfig::strtolower(s);
	return s[0] != '0' && s != "false" && s != "off";
}

static bool
parse_boolean_attribute(const xmlpp::Element& element, const std::string& attribute_name, bool default_value)
{
	std::string attr = element.get_attribute_value(attribute_name);
	if ( !attr.empty() )
		return parse_boolean_string(attr);
	return default_value;
}

static studio::PluginScript::ArgNecessity
parse_argument_necessity(const xmlpp::Element& element, const std::string& attribute_name, studio::PluginScript::ArgNecessity default_value)
{
	std::string attr = element.get_attribute_value(attribute_name);
	if ( !attr.empty() ) {
		auto s = synfig::trim(attr);
		synfig::strtolower(s);
		if (s == "optional")
			return studio::PluginScript::ArgNecessity::ARGUMENT_OPTIONAL;
		if (s == "mandatory")
			return studio::PluginScript::ArgNecessity::ARGUMENT_MANDATORY;
		if (s != "unused")
			synfig::error("Invalid value for attribute '%s' : '%s'. Valid values are 'optional', 'mandatory', 'unused'.", attribute_name.c_str(), attr.c_str());
		return studio::PluginScript::ArgNecessity::ARGUMENT_UNUSED;
	}
	return default_value;
}

const synfig::filesystem::Path studio::Plugin::user_config_filename{"user_config.json"};
const synfig::filesystem::Path studio::Plugin::config_ui_filename{"configuration.ui"};

studio::PluginString::PluginString(std::string fallback)
	: fallback_(std::move(fallback))
{
}

studio::PluginString studio::PluginString::load(const xmlpp::Node& parent, const std::string& tag_name)
{
	PluginString string;
	for ( const xmlpp::Node* node : parent.find("./" + tag_name) )
	{
		const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
		std::string lang = element->get_attribute_value("lang");
		if ( lang.empty() )
			lang = element->get_attribute_value("lang", "xml");
		if ( const xmlpp::TextNode* text = element->get_child_text() )
			string.add_translation(lang, text->get_content());
	}
	return string;
}

void studio::PluginString::add_translation(const std::string& locale, const std::string& translated)
{
	if ( locale.empty() )
		fallback_ = translated;
	else
		translations_.emplace(locale, translated);
}

std::string studio::PluginString::get() const
{
	std::vector<std::string> langs;
	if (studio::App::ui_language == "os_LANG")
		langs = synfig::OS::get_user_lang();
	else
		langs.push_back(studio::App::ui_language);

	for (const auto& lang : langs) {
		auto it = translations_.find(lang);
		if ( it != translations_.end() )
			return it->second;
	}
	return fallback_;
}

studio::PluginStream studio::PluginScript::stream_from_name(const std::string& name, PluginStream default_value)
{
	if ( name == "log" )
		return PluginStream::Log;
	if ( name == "message" )
		return PluginStream::Message;
	if ( name == "ignore" )
		return PluginStream::Ignore;
	return default_value;
}

studio::PluginScript studio::PluginScript::load(const xmlpp::Node& node, const synfig::filesystem::Path& working_directory)
{
	const xmlpp::Element& element = dynamic_cast<const xmlpp::Element&>(node);
	PluginScript script;
	script.interpreter = element.get_attribute_value("type");
	if ( script.interpreter.empty() )
		script.interpreter = "python";

	script.stdout_behaviour = stream_from_name(element.get_attribute_value("stdout"), script.stdout_behaviour);
	script.stderr_behaviour = stream_from_name(element.get_attribute_value("stderr"), script.stderr_behaviour);

	script.working_directory = working_directory;

	if ( const xmlpp::TextNode* text = element.get_child_text() )
		script.script = synfig::trim(text->get_content());

	script.modify_document = parse_boolean_attribute(element, "modify_doc", true);

	script.extra_args.current_time = parse_argument_necessity(element, "current_time", ArgNecessity::ARGUMENT_UNUSED);
	script.extra_args.selected_layers = parse_argument_necessity(element, "selected_layers", ArgNecessity::ARGUMENT_UNUSED);

	return script;
}

bool studio::PluginScript::is_valid() const
{
	return !interpreter.empty() && !script.empty();
}


bool studio::Plugin::is_valid() const
{
	return !name.fallback().empty();
}

synfig::filesystem::Path studio::Plugin::user_config_filepath() const
{
	return dir / user_config_filename;
}

synfig::filesystem::Path studio::Plugin::config_ui_filepath() const
{
	return dir / config_ui_filename;
}

studio::ImportExport studio::ImportExport::load(const xmlpp::Node& node)
{
	ImportExport ie;
	for ( const xmlpp::Node* ext : node.find("./extension/text()") )
	{
		const xmlpp::TextNode* text = dynamic_cast<const xmlpp::TextNode*>(ext);
		if (  text && !text->get_content().empty() )
			ie.extensions.push_back("." + text->get_content());
	}

	ie.description = PluginString::load(node, "description");

	return ie;
}

bool studio::ImportExport::is_valid() const
{
	return !description.fallback().empty() && !extensions.empty();
}

bool studio::ImportExport::has_extension(const std::string& ext) const
{
	for ( const auto& e : extensions )
		if ( e == ext )
			return true;
	return false;
}


void
studio::PluginManager::load_dir( const std::string& pluginsprefix )
{
	synfig::info(_("Loading plugins from %s"), pluginsprefix.c_str());

	try {
		Glib::Dir dir(pluginsprefix);
		for ( const std::string& entry : dir ) {
			std::string pluginpath = pluginsprefix + "/" + entry;
			std::string pluginfilepath = pluginpath + "/plugin.xml";
			if ( Glib::file_test(pluginpath, Glib::FILE_TEST_IS_DIR) && Glib::file_test(pluginfilepath, Glib::FILE_TEST_IS_REGULAR) ) {
				load_plugin(pluginfilepath, pluginpath);
			}
		}
	} catch ( const Glib::FileError& e ) {
		synfig::warning("Can't read plugin directory: %s", e.what().c_str());
	}

	signal_list_changed_.emit();
} // END of synfigapp::PluginManager::load_dir()

void
studio::PluginManager::load_plugin( const synfig::filesystem::Path& file, const synfig::filesystem::Path& plugindir, bool notify )
{
	synfig::info(_("   Loading plugin: %s"), plugindir.filename().u8_str());

	static int plugin_count = 0;
	const std::string id = "plugin" + std::to_string(++plugin_count);

	// parse xml file
	try
	{
		xmlpp::DomParser parser;
		//parser.set_validate();
		parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
		parser.parse_file(file.u8string());
		if ( !parser ) {
			synfig::warning("Invalid plugin.xml file!");
			return;
		}

		//Walk the tree:
		const xmlpp::Node* pNode = parser.get_document()->get_root_node(); //deleted by DomParser.
		if ( std::string(pNode->get_name()) != std::string("plugin") ) {
			synfig::warning("Invalid plugin.xml file (missing root <plugin>)");
			return;
		}

		auto execlist = pNode->find("./exec");
		if ( !execlist.empty() ) {
			Plugin plugin;
			plugin.name = PluginString::load(*pNode, "name");
			PluginScript script = PluginScript::load(*execlist[0], plugindir);
			plugin.id = id;
			plugin.dir = plugindir;

			plugin.release = PluginString::load(*pNode, "release");
			for ( const xmlpp::Node* node : pNode->find("./author") )
			{
				if ( node ) {
					const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
					if ( const xmlpp::TextNode* text = element->get_child_text() )
						plugin.author = text->get_content();
				}
			}
			for ( const xmlpp::Node* node : pNode->find("./version") )
			{
				if ( node ) {
					const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
					if ( const xmlpp::TextNode* text = element->get_child_text() )
						plugin.version = std::atoi(text->get_content().c_str());
				}
			}
			for ( const xmlpp::Node* node : pNode->find("./url") )
			{
				if ( node ) {
					const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
					if ( const xmlpp::TextNode* text = element->get_child_text() )
						plugin.url = text->get_content();
				}
			}
			plugin.description = PluginString::load(*pNode, "description");

			if ( !plugin.is_valid() || !script.is_valid() ) {
				synfig::warning(_("Invalid plugin metadata description"));
			} else {
				scripts_.emplace(plugin.id, std::move(script));
				plugins_.emplace_back(std::move(plugin));
			}
		}

		load_import_export(id, plugindir, pNode, "exporter", exporters_);
		load_import_export(id, plugindir, pNode, "importer", importers_);
	}
	catch(const std::exception& ex)
	{
		synfig::warning(_("Error while loading plugin.xml"));
		std::cout << "Exception caught: " << ex.what() << std::endl;
	}

	if (notify)
		signal_list_changed_.emit();
}

void studio::PluginManager::load_import_export(
	const std::string& id, const synfig::filesystem::Path& plugindir, const xmlpp::Node* node,
	const std::string& name, std::vector<ImportExport>& output
)
{
	auto nodelist = node->find("./" + name);
	output.reserve(output.size() + nodelist.size());
	int number = 0;
	for ( xmlpp::Node* exporter_node : nodelist )
	{
		auto execlist = exporter_node->find("./exec");
		if ( execlist.empty() )
			continue;

		ImportExport ie = ImportExport::load(*exporter_node);
		PluginScript script = PluginScript::load(*execlist[0], plugindir);
		if ( ie.is_valid() && script.is_valid() ) {
			ie.id  = id + "/" + name + std::to_string(number++);
			scripts_.emplace(ie.id, std::move(script));
			output.emplace_back(std::move(ie));
		}
	}

}


std::string studio::PluginManager::interpreter_executable(const std::string& interpreter) const
{
	// Path to python binary can be overridden
	// with SYNFIG_PYTHON_BINARY env variable:
	std::string command;

	if ( interpreter == "python" ) {
		std::vector<std::string> search_paths;
		std::string custom_python_binary = Glib::getenv("SYNFIG_PYTHON_BINARY");
		if (!custom_python_binary.empty()) {
			search_paths.emplace_back(custom_python_binary);
		} else {
			// Set path to python binary depending on the os type.
			// For Windows case Python binary is expected
			// at INSTALL_PREFIX/python/python.exe
			for (std::string python_bin : {"python3", "python"} ) {
#ifdef _WIN32
				search_paths.emplace_back(App::get_base_path() + "/python/" + python_bin + ".exe");
#endif
				search_paths.emplace_back(python_bin);
			}
		}
		for (const auto& path : search_paths) {
			if (studio::App::check_python_version(path)) {
				command = path;
				break;
			}
		}

		if ( command.empty() ) {
			studio::App::dialog_message_1b(
				"Error",
				_("Error: No Python 3 binary found.\n\nHint: You can set SYNFIG_PYTHON_BINARY environment variable pointing at your custom python installation."),
				"details",
				_("Close")
			);
		} else {
			synfig::info("Python 3 binary found: "+command);
		}
	} else {
		studio::App::dialog_message_1b(
			"Error",
			_("Error: Unsupported interpreter"),
			"details",
			_("Close")
		);

	}

	return command;
}

bool studio::PluginManager::check_and_run_dialog(const PluginScript& script, std::string& dialog_args)
{
	if (!script.script.empty()) {
		auto ui_file = (script.working_directory / script.script).replace_extension({".ui"});
		if (Glib::file_test(ui_file.u8string(), Glib::FILE_TEST_EXISTS | Glib::FILE_TEST_IS_REGULAR)) {
			std::string error_msg;
			try {
				auto builder = Gtk::Builder::create_from_file(ui_file.u8string());
				Gtk::Widget* contents;
				builder->get_widget("dialog_contents", contents);
				if (!contents) {
					error_msg = _("Missing main dialog");
				} else {
					Gtk::Dialog dialog;
					auto ok_btn = Gtk::Button(_("OK"));
					auto cancel_btn = Gtk::Button(_("Cancel"));
					dialog.get_content_area()->add(*contents);
					dialog.add_action_widget(cancel_btn, Gtk::RESPONSE_CANCEL);
					dialog.add_action_widget(ok_btn, Gtk::RESPONSE_ACCEPT);
					dialog.get_action_area()->show_all();
					int result = dialog.run();
					if (result != Gtk::RESPONSE_ACCEPT)
						return false;
					auto dialog_data = JSON::parse_dialog(*contents);
//					delete dialog;
					dialog_args = JSON::stringify(dialog_data);
				}
			} catch (const Glib::FileError& ex) {
				error_msg = ex.what();
			} catch (const Glib::MarkupError& ex) {
				error_msg = ex.what();
			} catch (const Gtk::BuilderError& ex) {
				error_msg = ex.what();
			} catch(...) {
				error_msg = _("Unknown exception");
			}

			if (!error_msg.empty()) {
				studio::App::dialog_message_1b("Error", synfig::strprintf(_("Plugin execution failed: %s"), error_msg.c_str()), _("User Interface failed to be load"), _("Close"));
				return false;
			}
		}
	}
	return true;
}

bool studio::PluginManager::run(const studio::PluginScript& script, std::vector<std::string> args, const std::unordered_map<std::string,std::string>& view_state) const
{
	std::string exec = interpreter_executable(script.interpreter);
	if ( exec.empty() ) {
		return false;
	}

	std::string dialog_data;
	if (!check_and_run_dialog(script, dialog_data))
		return false;

	args.insert(args.begin(), script.script.u8string());
	args.insert(args.begin(), exec);

	std::string canvas_state;

	if (script.extra_args.current_time != PluginScript::ArgNecessity::ARGUMENT_UNUSED) {
//		if (!canvas_state.empty())
//			canvas_state.append(",");
		canvas_state += "\"current_time\":" + view_state.at("current_time");
	}
	if (script.extra_args.selected_layers != PluginScript::ArgNecessity::ARGUMENT_UNUSED) {
		if (!canvas_state.empty())
			canvas_state.append(",");
		auto iter = view_state.find("selected_layers");
		canvas_state += "\"selected_layers\":[" + (iter != view_state.end() ? iter->second : "") + "]";
	}
	if (!canvas_state.empty())
		canvas_state = "{" + canvas_state + "}";

	if (dialog_data == "{}") {
		dialog_data.clear();
	}

	TmpFile tmp_file(&script, "extra-data", "json");
	if (!canvas_state.empty() || !dialog_data.empty()) {
		auto stream2 = synfig::FileSystemNative::instance()->get_write_stream(tmp_file.filename.u8string());
		auto stream = stream2;
		*stream << "{";
		if (!canvas_state.empty()) {
			*stream << "\"canvas_state\":" << canvas_state;
			if (!dialog_data.empty())
				*stream << ",";
		}
		if (!dialog_data.empty()) {
			*stream << "\"dialog\":" << dialog_data;
		}
		*stream << "}";

		args.push_back(tmp_file.filename.u8string());
	}

	std::string stdout_str;
	std::string stderr_str;
	int exit_status;

	studio::OneMoment one_moment;
	try {
		Glib::spawn_sync(
			script.working_directory.u8string(),
			args,
			Glib::SPAWN_SEARCH_PATH,
			Glib::SlotSpawnChildSetup(),
			&stdout_str,
			&stderr_str,
			&exit_status
		);
	} catch ( const Glib::SpawnError& err ) {
		studio::App::dialog_message_1b("Error", synfig::strprintf(_("Plugin execution failed: %s"), err.what().c_str()), "details", _("Close"));
		return false;
	}

	one_moment.hide();
	handle_stream(script.stdout_behaviour, stdout_str);
	handle_stream(script.stderr_behaviour, stderr_str);

	if ( exit_status && (stderr_str.empty() || script.stderr_behaviour != PluginStream::Message) ) {
		studio::App::dialog_message_1b("Error", _("Plugin execution failed"), "details", _("Close"));
	}

	return true;
}

void studio::PluginManager::handle_stream(studio::PluginStream behaviour, const std::string& output) const
{
	if ( output.empty() )
		return;

	switch ( behaviour )
	{
		case PluginStream::Ignore:
			break;
		case PluginStream::Log:
			synfig::info(output);
			break;
		case PluginStream::Message:
			studio::App::dialog_message_1b("Error", output, "details", _("Close"));
			break;
	}
}

bool studio::PluginManager::run(const std::string& script_id, const std::vector<std::string>& args, const std::unordered_map<std::string,std::string>& view_state) const
{
	auto it = scripts_.find(script_id);
	if ( it != scripts_.end() )
		return run(it->second, args, view_state);

	studio::App::dialog_message_1b("Error", _("Plugin not found"), "details", _("Close"));
	return false;
}

studio::Plugin studio::PluginManager::get_plugin(const std::string& id) const
{
	if (!id.empty()) {
		for (const auto& plugin : plugins_) {
			if (plugin.id == id)
				return plugin;
		}
	}
	return Plugin();
}

void studio::PluginManager::remove_plugin(const std::string& id)
{
	try {
		auto plugin_it = std::find_if(plugins_.begin(), plugins_.end(), [&id](const Plugin& plugin) { return plugin.id == id; });
		if (plugin_it == plugins_.end())
			return;
		auto fileSystem = synfig::FileSystemNative::instance();
		if (fileSystem->remove_recursive(plugin_it->dir)) {
			plugins_.erase(plugin_it);
			signal_list_changed_.emit();
		}
	} catch(const std::exception& e) {
		studio::App::dialog_message_1b("Error", synfig::strprintf(_("Plugin execution failed: %s"), e.what()), _("Plugin Deletion Failed"), _("Close"));
	}
}

sigc::signal<void>& studio::PluginManager::signal_list_changed()
{
	return signal_list_changed_;
}

studio::PluginScript::ScriptArgs studio::PluginManager::get_script_args(const std::string& script_id) const
{
	auto it = scripts_.find(script_id);
	if ( it != scripts_.end() )
		return it->second.extra_args;
	return {};
}
