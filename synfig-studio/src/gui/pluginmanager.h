/* === S Y N F I G ========================================================= */
/*!	\file gui/pluginmanager.h
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_STUDIO_PLUGINMANAGER_H
#define SYNFIG_STUDIO_PLUGINMANAGER_H

/* === H E A D E R S ======================================================= */

#include <unordered_map>
#include <vector>

#include <sigc++/signal.h>

#include <synfig/filesystem_path.h>

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
	synfig::filesystem::Path script;
	synfig::filesystem::Path working_directory;

	// Behavior
	bool modify_document = true;

	enum class ArgNecessity {
		ARGUMENT_OPTIONAL,
		ARGUMENT_MANDATORY,
		ARGUMENT_UNUSED,
	};

	// Required arguments
	struct ScriptArgs
	{
		ArgNecessity current_time = ArgNecessity::ARGUMENT_UNUSED;
		ArgNecessity selected_layers = ArgNecessity::ARGUMENT_UNUSED;
	} extra_args;

	static PluginScript load(const xmlpp::Node& node, const synfig::filesystem::Path& working_directory);
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
	synfig::filesystem::Path dir;
	PluginString name;

	std::string author;
	PluginString release;
	int version;
	std::string url;

	PluginString description;

	bool is_valid() const;

	synfig::filesystem::Path user_config_filepath() const;
	synfig::filesystem::Path config_ui_filepath() const;

private:
	static const synfig::filesystem::Path user_config_filename;
	static const synfig::filesystem::Path config_ui_filename;
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
		const std::string& id, const synfig::filesystem::Path& plugindir, const xmlpp::Node* node,
		const std::string& name, std::vector<ImportExport>& output
	);

	static bool check_and_run_dialog(const PluginScript& script, std::string& dialog_args);

	sigc::signal<void> signal_list_changed_;

public:
	void load_dir(const std::string& pluginsprefix);
	void load_plugin(const synfig::filesystem::Path& file, const synfig::filesystem::Path& plugindir, bool notify = false);
	void remove_plugin(const std::string& id);
	bool run(const PluginScript& script, std::vector<std::string> args, const std::unordered_map<std::string,std::string>& view_state) const;
	bool run(const std::string& script_id, const std::vector<std::string>& args, const std::unordered_map<std::string,std::string>& view_state = {}) const;

	const std::vector<Plugin>& plugins() const { return plugins_; };

	Plugin get_plugin(const std::string& id) const;
	PluginScript::ScriptArgs get_script_args(const std::string& script_id) const;

	const std::vector<ImportExport>& exporters() { return exporters_; };

	const std::vector<ImportExport>& importers() { return importers_; };

	sigc::signal<void>& signal_list_changed();

}; // END class PluginManager


} // END namespace synfigapp

/* === E N D =============================================================== */

#endif
