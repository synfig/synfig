/* === S I N F G =========================================================== */
/*!	\file template.h
**	\brief Template Header
**
**	$Id: settings.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SINFG_SETTINGS_H
#define __SINFG_SETTINGS_H

/* === H E A D E R S ======================================================= */

#include <sinfg/string.h>
#include <map>
#include <list>
#include <ETL/stringf>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class Settings
{
public:
	
	typedef std::list<sinfg::String> KeyList;
	typedef std::map<sinfg::String,sinfg::String> ValueBaseMap;
	typedef std::map<sinfg::String,Settings*> DomainMap;
	
private:
	ValueBaseMap simple_value_map;

	DomainMap domain_map;

public:
	Settings();
	virtual ~Settings();

	virtual bool get_value(const sinfg::String& key, sinfg::String& value)const;
	virtual bool set_value(const sinfg::String& key,const sinfg::String& value);
	virtual KeyList get_key_list()const;

	sinfg::String get_value(const sinfg::String& key)const;
	void add_domain(Settings* domain, const sinfg::String& name);
	void remove_domain(const sinfg::String& name);

	bool load_from_string(const sinfg::String& data);
	bool save_to_string(sinfg::String& data);

	bool load_from_file(const sinfg::String& filename);
	bool save_to_file(const sinfg::String& filename)const;
}; // END of class Settings

}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
