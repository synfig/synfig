/* === S Y N F I G ========================================================= */
/*!	\file synfigapp/pluginmanager.h
**	\brief Plugin Manager responsible for loading plugins
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_GTKMM_PLUGINMANAGER_H
#define __SYNFIG_GTKMM_PLUGINMANAGER_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <unordered_map>
#include <vector>

namespace xmlpp {
	class Node;
} // namespace xmlpp

namespace studio {


class PluginString
{
private:
	std::unordered_map<std::string, std::string> translations_;
	std::string fallback_;

public:
	PluginString(std::string fallback={});

	static PluginString load(const xmlpp::Node& parent, const std::string& tag_name);

	void add_translation(const std::string& locale, const std::string& translated);

	std::string get() const;

	const std::string& fallback() const { return fallback_; }
};

enum class PluginStream
{
	Ignore,
	Log,
	Message
};

struct PluginScript
{
	PluginStream stdout_behaviour = PluginStream::Ignore;
	PluginStream stderr_behaviour = PluginStream::Message;
	std::string interpreter;
	std::string script;
	std::string working_directory;

	static PluginScript load(const xmlpp::Node& node, const std::string& working_directory);
	static PluginStream stream_from_name(const std::string& name, PluginStream default_value);

	bool is_valid() const;
};

class ImportExport
{
public:
	std::string id;
	std::vector<std::string> extensions;
	PluginString description;

	static ImportExport load(const xmlpp::Node& node);

	bool is_valid() const;

	bool has_extension(const std::string& ext) const;
};

class Plugin
{
public:
	std::string id;
	PluginString name;

	bool is_valid() const;
};

class PluginManager
{
private:
	std::vector<Plugin> plugins_;
	std::vector<ImportExport> exporters_;
	std::vector<ImportExport> importers_;
	std::unordered_map<std::string, PluginScript> scripts_;

	std::string interpreter_executable(const std::string& interpreter) const;
	void handle_stream(PluginStream behaviour, const std::string& output) const;
	void load_import_export(
		const std::string& id, const std::string& plugindir, const xmlpp::Node* node,
		const std::string& name, std::vector<ImportExport>& output
	);

public:
	void load_dir( const std::string &pluginsprefix );
	void load_plugin( const std::string &file, const std::string &plugindir );

	bool run(const PluginScript& script, std::vector<std::string> args) const;
	bool run(const std::string& script_id, const std::vector<std::string>& args) const;

	const std::vector<Plugin>& plugins() const { return plugins_; };

	const std::vector<ImportExport>& exporters() { return exporters_; };

	const std::vector<ImportExport>& importers() { return importers_; };

}; // END class PluginManager


} // END namespace synfigapp

/* === E N D =============================================================== */

#endif
