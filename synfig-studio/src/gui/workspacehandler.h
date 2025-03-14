/* === S Y N F I G ========================================================= */
/*!	\file workspacehandler.h
**	\brief Handle with custom workspaces
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2019 Rodolfo R Gomes
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

#ifndef SYNFIG_STUDIO_WORKSPACEHANDLER_H
#define SYNFIG_STUDIO_WORKSPACEHANDLER_H

#include <map>
#include <string>
#include <vector>
#include <sigc++/signal.h>

#include <synfig/filesystem_path.h>

namespace studio {

/// Deal with custom workspaces
class WorkspaceHandler
{
public:
	WorkspaceHandler() = default;;

	static bool is_valid_name(const std::string &name);

	bool has_workspace(const std::string &name) const;
	/// \param[in] tpl workspace template string
	bool add_workspace(const std::string &name, const std::string &tpl);
	void remove_workspace(const std::string &name);
	/// remove all workspaces layout
	void clear();

	/// \param[in] tpl workspace template string
	bool set_workspace(const std::string &name, const std::string &tpl);
	/// \param[out] tpl workspace template string
	bool get_workspace(const std::string &name, std::string &tpl) const;

	/// \param[out] list List of workspace names
	void get_name_list(std::vector<std::string>& list);

	/// load custom workspace layouts from a config file
	void load(const synfig::filesystem::Path& filename);
	/// stores custom workspace layouts in a config file
	bool save(const synfig::filesystem::Path& filename);

	sigc::signal<void> & signal_list_changed();

private:
	std::map<std::string, std::string> workspaces;

	sigc::signal<void> signal_list_changed_;
};

}

#endif // SYNFIG_STUDIO_WORKSPACEHANDLER_H
