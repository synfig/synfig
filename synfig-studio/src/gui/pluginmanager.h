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
#include <synfig/canvas.h>
#include <list>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace studio {

class PluginManager
{
	/*
 -- ** -- P U B L I C   T Y P E S ---------------------------------------------
	*/

public:
	struct plugin{
		std::string id;
		std::string name;
		std::string path;
		std::string extension;
		std::string description;
	};

	/*
 -- ** -- P U B L I C  D A T A ------------------------------------------------
	*/

public:

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	std::list< plugin > list_;
	std::list< plugin > exporters_;

protected:
	
	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	PluginManager();
	~PluginManager();

	void load_dir( const std::string &pluginsprefix );
	void load_plugin( const std::string &path );

	std::list< plugin > get_list() { return list_; };

	std::list< plugin > get_exporters() { return exporters_; };

}; // END class PluginManager


}; // END namespace synfigapp

/* === E N D =============================================================== */

#endif
