/* === S Y N F I G ========================================================= */
/*!	\file settings.h
**	\brief Template Header
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#ifndef __SYNFIG_SETTINGS_H
#define __SYNFIG_SETTINGS_H

/* === H E A D E R S ======================================================= */

#include <synfig/string.h>
#include <map>
#include <list>
#include <ETL/stringf>
#include <string.h>
#include <sstream>

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

	//! new get_value
	template <typename T>
	T tget_value(const synfig::String& key, const T default_value) const
	{
		synfig::String str;
		if (!get_value(key, str))
			return default_value;
		T result;
		std::istringstream stream { str };
		if ((stream >> result) && stream.eof())
		{
			return result;
		}
		return default_value;
	}
	//! new set_value
	template <typename T>
	void tset_value(const synfig::String& key, const T value)
	{
		std::ostringstream stream;
		stream << value;
		set_value(key, stream.str());
	}

	//! old set&get value
	//! TODO: get rid or virtuality and make private
	virtual bool get_value(const synfig::String& key, synfig::String& value)const;
	virtual bool set_value(const synfig::String& key,const synfig::String& value);

	synfig::String get_value(const synfig::String& key)const;
	virtual KeyList get_key_list()const;

	void add_domain(Settings* domain, const synfig::String& name);
	void remove_domain(const synfig::String& name);

	bool load_from_string(const synfig::String& data);
	bool save_to_string(synfig::String& data);

	//! \brief Load optionally filtered settings from given synfig settings format filename
	//! \return false if file open failed, else true.
	//! \sa		set_value
	//! \Param[in] filename, the synfig settings format filename. Should be aboslute path.
	//! \Param[in] key_filter, optional, string use to filter the settings key. No wildcard only full equal string test.
	bool load_from_file(const synfig::String& filename, const synfig::String& key_filter = "" );
	bool save_to_file(const synfig::String& filename)const;
}; // END of class Settings

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
