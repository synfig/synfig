/* === S Y N F I G ========================================================= */
/*!	\file workspacehandler.h
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

#ifndef SYNFIG_STUDIO_WORKSPACEHANDLER_H
#define SYNFIG_STUDIO_WORKSPACEHANDLER_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace studio {

/// Deal with custom workspaces
class WorkspaceHandler
{
public:
	WorkspaceHandler(const char *filename);

	static bool is_valid_name(const std::string &name);

	bool has_workspace(const std::string &name) const;
	/// \param[in] tpl workspace template string
	bool add_workspace(const std::string &name, const std::string &tpl);
	void remove_workspace(const std::string &name);

	/// \param[in] tpl workspace template string
	bool set_workspace(const std::string &name, const std::string &tpl);
	/// \param[out] tpl workspace template string
	bool get_workspace(const std::string &name, std::string &tpl) const;

	void get_name_list(std::vector<std::string>& list);

	void save();

private:
	std::map<std::string, std::string> workspaces;

	std::string filename;
	void load();
};

}

#endif // SYNFIG_STUDIO_WORKSPACEHANDLER_H
