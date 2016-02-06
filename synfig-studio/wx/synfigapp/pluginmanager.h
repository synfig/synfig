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

#ifndef __SYNFIG_APP_PLUGINMANAGER_H
#define __SYNFIG_APP_PLUGINMANAGER_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <synfig/canvas.h>
#include <list>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {
	
class PluginLauncher
{

	/*
 -- ** -- P R I V A T E   D A T A ---------------------------------------------
	*/

private:

	std::string filename_original;		// location of original file
	std::string filename_processed; 	// processed file
	std::string filename_backup;		// backup copy
	std::string output;
	int exitcode;

protected:
	
	/*
 -- ** -- P U B L I C   M E T H O D S -----------------------------------------
	*/

public:
	PluginLauncher( synfig::Canvas::Handle );
	~PluginLauncher();

	bool execute( std::string script_path, const std::string& synfig_root );
	bool check_python_version( std::string path);
	std::string get_result_path();
	std::string get_original_path() { return filename_original; };
	std::string get_output() { return output; };

}; // END class Plugin
	
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

}; // END class PluginManager


}; // END namespace synfigapp

/* === E N D =============================================================== */

#endif
