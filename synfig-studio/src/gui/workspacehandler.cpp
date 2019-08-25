/* === S Y N F I G ========================================================= */
/*!	\file workspacehandler.cpp
**	\brief Handle with custom workspaces
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2019 Rodolfo R Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif
# include "workspacehandler.h"
# include <algorithm>
# include <fstream>
# include "synfig/general.h"
# include "gui/localization.h"
#endif

using namespace studio;

WorkspaceHandler::WorkspaceHandler(const char *filename)
	: filename(filename)
{
	if (!this->filename.empty())
		load();
}

static void
trim_string(std::string &text)
{
	text.erase(text.begin(),
			   std::find_if(text.begin(), text.end(),
							[](int chr) { return !std::isspace(chr);})
			   );
	text.erase(std::find_if(text.rbegin(), text.rend(),
							[](int chr) { return !std::isspace(chr);}).base(),
			   text.end()
			   );
}

bool
WorkspaceHandler::is_valid_name(const std::string& name)
{
	std::string valid_name = name;
	trim_string(valid_name);
	return !valid_name.empty() && valid_name.find("=") == std::string::npos;
}

bool
WorkspaceHandler::has_workspace(const std::string& name) const
{
	return workspaces.find(name) != workspaces.end();
}

bool
WorkspaceHandler::add_workspace(const std::string& name, const std::string& tpl)
{
	if (!is_valid_name(name) || tpl.empty())
		return false;
	std::string valid_name(name);
	trim_string(valid_name);
	if (has_workspace(valid_name))
		return false;
	workspaces[valid_name] = tpl;
	return true;
}

void
WorkspaceHandler::remove_workspace(const std::string& name)
{
	workspaces.erase(name);
}

bool
WorkspaceHandler::set_workspace(const std::string& name, const std::string& tpl)
{
	if (!is_valid_name(name) || tpl.empty())
		return false;
	std::string valid_name(name);
	trim_string(valid_name);
	if (!has_workspace(valid_name))
		return false;
	workspaces[valid_name] = tpl;
	return true;
}

bool
WorkspaceHandler::get_workspace(const std::string& name, std::string& tpl) const
{
	auto it = workspaces.find(name);
	if (it == workspaces.end())
		return false;
	tpl = it->second;
	return true;
}

void
WorkspaceHandler::get_name_list(std::vector<std::string>& list)
{
	list.clear();
	for(auto it = workspaces.cbegin(); it != workspaces.cend(); ++it)
		list.push_back(it->first);
}

void
WorkspaceHandler::save()
{
	std::ofstream ofs(filename);
	if (!ofs) {
		synfig::error(_("Can't save custom workspaces"));
		return;
	}
	for (auto it = workspaces.begin(); it != workspaces.end(); ++it)
		ofs << it->first << "=" << it->second << std::endl;
	ofs.close();
}

void
WorkspaceHandler::load()
{
	workspaces.clear();

	std::ifstream ifs(filename);
	std::string line;
	while (ifs && !ifs.eof()) {
		getline(ifs, line);
		if (line.empty())
			continue;

		auto pos = line.find("=");
		if (pos == std::string::npos) {
			synfig::warning(_("ignoring malformed workspace line: %s"), line.c_str());
			continue;
		}

		std::string name = line.substr(0, pos);
		if (has_workspace(name)) {
			synfig::warning(_("ignoring duplicated workspace name: %s"), name.c_str());
			continue;
		}

		std::string tpl = line.substr(pos+1);
		workspaces[name] = tpl;
	}
}
