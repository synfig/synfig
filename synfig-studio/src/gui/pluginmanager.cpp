/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/pluginmanager.cpp
**	\brief  Plugin Manager responsible for loading plugins
**
**	$Id$
**
**	\legal
**	Copyright (c) 2012-2013 Konstantin Dmitriev
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

#include "pluginmanager.h"

#include <libxml++/libxml++.h>
#include <glibmm/fileutils.h>
#include <glibmm/spawn.h>

#include <ETL/handle>

#include <synfig/general.h>
#include <synfig/savecanvas.h>
#include <synfig/filesystemnative.h>
#include <synfigapp/main.h>

#include <synfigapp/localization.h>

#include "app.h"
#include "onemoment.h"

#endif


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
	std::string current_locale = setlocale(LC_ALL, NULL);
	auto it = translations_.find(current_locale);
	if ( it == translations_.end() )
		return fallback_;
	return it->second;
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

studio::PluginScript studio::PluginScript::load(const xmlpp::Node& node, const std::string& working_directory)
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
		script.script = text->get_content();

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
studio::PluginManager::load_dir( const std::string &pluginsprefix )
{
	
	synfig::info("Loading plugins from %s", pluginsprefix.c_str());

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
} // END of synfigapp::PluginManager::load_dir()

void
studio::PluginManager::load_plugin( const std::string &file, const std::string &plugindir )
{
	synfig::info("   Loading plugin: %s", etl::basename(plugindir).c_str());

	std::string id = file;

	// parse xml file
	try
	{
		xmlpp::DomParser parser;
		//parser.set_validate();
		parser.set_substitute_entities(); //We just want the text to be resolved/unescaped automatically.
		parser.parse_file(file);
		if ( !parser )
		{
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
		if ( !execlist.empty() )
		{
			Plugin plugin;
			plugin.name = PluginString::load(*pNode, "name");
			PluginScript script = PluginScript::load(*execlist[0], plugindir);
			plugin.id = id;

			if ( !plugin.is_valid() || !script.is_valid() )
			{
				synfig::warning("Invalid plugin description");
			}
			else
			{
				scripts_.emplace(plugin.id, std::move(script));
				plugins_.emplace_back(std::move(plugin));
			}
		}

		load_import_export(id, plugindir, pNode, "exporter", exporters_);
		load_import_export(id, plugindir, pNode, "importer", importers_);
	}
	catch(const std::exception& ex)
	{
		synfig::warning("Error while loading plugin.xml");
		std::cout << "Exception caught: " << ex.what() << std::endl;
	}
}

void studio::PluginManager::load_import_export(
	const std::string& id, const std::string& plugindir, const xmlpp::Node* node,
	const std::string& name, std::vector<ImportExport>& output
)
{
	auto nodelist = node->find("./exporter");
	output.reserve(output.size() + nodelist.size());
	int number = 0;
	for ( xmlpp::Node* exporter_node : nodelist )
	{
		auto execlist = exporter_node->find("./exec");
		if ( execlist.empty() )
			continue;

		ImportExport ie = ImportExport::load(*exporter_node);
		PluginScript script = PluginScript::load(*execlist[0], plugindir);
		if ( ie.is_valid() && script.is_valid() )
		{
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

	if ( interpreter == "python" )
	{
		const char* custom_python_binary = getenv("SYNFIG_PYTHON_BINARY");
		if(custom_python_binary) {
			command = custom_python_binary;
			if ( !studio::App::check_python_version(command) ) {
				command = "";
			}
		} else {
			// Set path to python binary depending on the os type.
			// For Windows case Python binary is expected
			// at INSTALL_PREFIX/python/python.exe
			for ( std::string iter : {"python", "python3"} )
			{
				std::string python_path;
#ifdef _WIN32
				python_path = App::get_base_path() + "/python/" + iter + ".exe";
#else
				python_path = iter;
#endif
				if ( studio::App::check_python_version(python_path) )
				{
					command = python_path;
					break;
				}
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
	}
	else
	{
		studio::App::dialog_message_1b(
			"Error",
			_("Error: Unsupported interpreter"),
			"details",
			_("Close")
		);

	}

	return command;
}

bool studio::PluginManager::run(const studio::PluginScript& script, std::vector<std::string> args) const
{
	std::string exec = interpreter_executable(script.interpreter);
	if ( exec.empty() )
	{
		return false;
	}

	args.insert(args.begin(), script.script);
	args.insert(args.begin(), exec);

	std::string stdout_str;
	std::string stderr_str;
	int exit_status;

	studio::OneMoment one_moment;
	try {
		Glib::spawn_sync(
			script.working_directory,
			args,
			Glib::SPAWN_SEARCH_PATH,
			Glib::SlotSpawnChildSetup(),
			&stdout_str,
			&stderr_str,
			&exit_status
		);
	} catch ( const Glib::SpawnError& err ) {
		studio::App::dialog_message_1b("Error", etl::strprintf(_("Plugin execution failed: %s"), err.what().c_str()), "details", _("Close"));
		return false;
	}

	one_moment.hide();
	handle_stream(script.stdout_behaviour, stdout_str);
	handle_stream(script.stderr_behaviour, stderr_str);

	if ( exit_status && (stderr_str.empty() || script.stderr_behaviour != PluginStream::Message) )
	{
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

bool studio::PluginManager::run(const std::string& script_id, const std::vector<std::string>& args) const
{
	auto it = scripts_.find(script_id);
	if ( it != scripts_.end() )
		return run(it->second, args);

	studio::App::dialog_message_1b("Error", _("Plugin not found"), "details", _("Close"));
	return false;
}
