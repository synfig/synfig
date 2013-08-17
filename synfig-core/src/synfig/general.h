/* === S Y N F I G ========================================================= */
/*!	\file general.h
**	\brief General macros, classes, and procedure declarations
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**  Copyright (c) 2010 Carlos López
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

#ifndef __SYNFIG_GENERAL_H
#define __SYNFIG_GENERAL_H

/* === H E A D E R S ======================================================= */

#include <ETL/stringf>
#include "string.h"
#include "version.h"
#ifdef ENABLE_NLS
 #include <locale.h>
 #include <libintl.h>
#endif

/* === M A C R O S ========================================================= */

#ifdef ENABLE_NLS
#define _(x) dgettext("synfig",x)
#define gettext_noop(x) x
#define N_(x) gettext_noop(x)
#else
#define dgettext(a,x) (x)
#define _(x) (x)
#define N_(x) (x)
#endif

#define SYNFIG_COPYRIGHT "Copyright (c) 2001-2005 Robert B. Quattlebaum Jr., Adrian Bentley"


#ifdef _DEBUG
#ifdef __FUNC__
#define DEBUGPOINT()	synfig::warning(etl::strprintf(__FILE__":"__FUNC__":%d DEBUGPOINT",__LINE__))
#define DEBUGINFO(x)	synfig::warning(etl::strprintf(__FILE__":"__FUNC__":%d:DEBUGINFO:",__LINE__)+x)
#else
#define DEBUGPOINT()	synfig::warning(etl::strprintf(__FILE__":%d DEBUGPOINT",__LINE__))
#define DEBUGINFO(x)	synfig::warning(etl::strprintf(__FILE__":%d:DEBUGINFO:",__LINE__)+x)
#endif

#else
#define DEBUGPOINT()
#define DEBUGINFO(x)
#endif

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class ChangeLocale {
	const String previous;
	const int category;
public:
	ChangeLocale(int category, const char *locale):
	// This backups the old locale
	previous(setlocale(category,NULL)),category(category)
	{
		// This effectively changes the locale
		setlocale(category, locale);
	}
	~ChangeLocale() {
		// This restores the locale
		setlocale(category,previous.c_str());
	}
};

/*!	\class ProgressCallback
**	\todo writeme
*/
class ProgressCallback
{
public:

	virtual ~ProgressCallback() { }
	virtual bool task(const String &/*task*/) { return true; }
	virtual bool error(const String &/*task*/) { return true; }
	virtual bool warning(const String &/*task*/) { return true; }
	virtual bool amount_complete(int /*current*/, int /*total*/) { return true; }

	virtual bool valid() const { return true; }
};

typedef ProgressCallback ProgressManager;

/*!	\class SuperCallback
**	\todo writeme
*/
class SuperCallback : public ProgressCallback
{
	ProgressCallback *cb;
	int start,end,tot;
	int w;
public:

	SuperCallback() { cb=NULL; }
	SuperCallback(ProgressCallback *cb,int start, int end, int total):cb(cb),start(start),end(end),tot(total)
	{
		//make sure we don't "inherit" if our subcallback is invalid
		if(!cb || !cb->valid())
			cb = NULL;
		w=end-start;
	}
	virtual bool task(const String &task) { if(cb)return cb->task(task); return true; }
	virtual bool error(const String &task) { if(cb)return cb->error(task); return true; }
	virtual bool warning(const String &task) { if(cb)return cb->warning(task); return true; }
	virtual bool amount_complete(int cur, int total) { if(cb)return cb->amount_complete(start+cur*w/total,tot); return true; }

	virtual bool valid() const { return cb != 0; }
};


/*
extern bool add_to_module_search_path(const std:string &path);
extern bool add_to_config_search_path(const std:string &path);
*/

//! Shutdown the synfig environment
extern void shutdown();

//! Reports an error
/*! Call this when an error occurs, describing what happened */
extern void error(const char *format,...);
extern void error(const String &str);

//! Reports a warning
/*! Call this when something questionable occurs, describing what happened */
extern void warning(const char *format,...);
extern void warning(const String &str);

//! Reports some information
/*! Call this to report various information. Please be sparse... */
extern void info(const char *format,...);
extern void info(const String &str);

//! Returns absolute path to the binary
extern String get_binary_path(const String &fallback_path);

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
