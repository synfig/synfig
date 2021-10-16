/* === S Y N F I G ========================================================= */
/*!	\file settings.h
**	\brief Template Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_SETTINGS_H
#define __SYNFIG_SETTINGS_H

/* === H E A D E R S ======================================================= */

#include <list>
#include <map>

#include <ETL/stringf>
#include <synfig/general.h>
#include <synfig/string.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class Settings
{
public:

	typedef std::list<synfig::String> KeyList;
	typedef std::map<synfig::String,synfig::String> ValueBaseMap;
	typedef std::map<synfig::String,Settings*> DomainMap;

private:
	ValueBaseMap simple_value_map;

	DomainMap domain_map;

public:
	Settings();
	virtual ~Settings();

	virtual bool get_raw_value(const synfig::String& key, synfig::String& value)const;
	virtual bool set_value(const synfig::String& key,const synfig::String& value);
	virtual KeyList get_key_list()const;

	synfig::String get_value(const synfig::String& key)const;
	double get_value(const synfig::String& key, double default_value) const;
	int get_value(const synfig::String& key, int default_value) const;
	bool get_value(const synfig::String& key, bool default_value) const;
	synfig::String get_value(const synfig::String& key, const synfig::String& default_value) const;
	synfig::String get_value(const synfig::String& key, const char* default_value) const;

	void add_domain(Settings* domain, const synfig::String& name);
	void remove_domain(const synfig::String& name);

	bool load_from_string(const synfig::String& data);
	bool save_to_string(synfig::String& data);

	//! \brief Load optionally filtered settings from given synfig settings format filename
	//! \return false if file open failed, else true. If key_filter is set, return false if value could not be loaded.
	//! \sa		set_value
	//! \Param[in] filename, the synfig settings format filename. Should be absolute path.
	//! \Param[in] key_filter, optional, string use to filter the settings key. No wildcard only full equal string test.
	bool load_from_file(const synfig::String& filename, const synfig::String& key_filter = "" );
	bool save_to_file(const synfig::String& filename)const;
}; // END of class Settings

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
