/* === S Y N F I G ========================================================= */
/*!	\file exception.cpp
**	\brief Template File
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "exception.h"
#include "general.h"
#include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Exception::Exception::Exception(const String& what):
	std::runtime_error(what)
{
}

Exception::BadLinkName::BadLinkName(const String &what):
	Exception(what)
//	Exception(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad link name: "+what);
}

Exception::BadType::BadType(const String &what):
	Exception(what)
//	Exception(_("Bad Type")+what.empty()?"":(String(": ")+what))
{
//	synfig::error("EXCEPTION: bad type: "+what);
}

Exception::BadFrameRate::BadFrameRate(const String &what):
	Exception(what)
//	Exception(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad frame rate: "+what);
}

Exception::BadTime::BadTime(const String &what):
	Exception(what)
//	Exception(_("Bad Link Name")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: bad time: "+what);
}

Exception::NotFound::NotFound(const String &what):
	Exception(what)
//	Exception(_("Not Found")+what.empty()?"":(String(": ")+what))
{
//	synfig::error("EXCEPTION: not found: "+what);
}

Exception::IDNotFound::IDNotFound(const String &what):
	NotFound(what)
//	Exception(_("Not Found")+what.empty()?"":(String(": ")+what))
{
//	synfig::error("EXCEPTION: not found: "+what);
}

Exception::FileNotFound::FileNotFound(const String &what):
	NotFound(what)
//	Exception(_("Not Found")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: file not found: "+what);
}

Exception::IDAlreadyExists::IDAlreadyExists(const String &what):
	Exception(what)
//	Exception(_("ID Already Exists")+what.empty()?"":(String(": ")+what))
{
	synfig::error("EXCEPTION: id already exists: "+what);
}
