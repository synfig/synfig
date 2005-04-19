/* === S Y N F I G ========================================================= */
/*!	\file template.cpp
**	\brief Template File
**
**	$Id: exception.cpp,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "exception.h"
#include "general.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Exception::BadLinkName::BadLinkName(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad link name: "+what);
}

Exception::BadType::BadType(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("Bad Type")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad type: "+what);
}

Exception::BadFrameRate::BadFrameRate(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad frame rate: "+what);
}

Exception::BadTime::BadTime(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad time: "+what);
}

Exception::NotFound::NotFound(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("Not Found")+what.empty()?"":(String(": ")+what))
{
//	synfig::error("EXCEPTION: not found: "+what);
}

Exception::IDNotFound::IDNotFound(const String &what):
	NotFound(what)
//	std::runtime_error(_("Not Found")+what.empty()?"":(String(": ")+what))
{
//	synfig::error("EXCEPTION: not found: "+what);
}

Exception::FileNotFound::FileNotFound(const String &what):
	NotFound(what)
//	std::runtime_error(_("Not Found")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: file not found: "+what);
}

Exception::IDAlreadyExists::IDAlreadyExists(const String &what):
	std::runtime_error(what)
//	std::runtime_error(_("ID Already Exists")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: id already exists: "+what);
}
