/* === S Y N F I G ========================================================= */
/*!	\file exception.h
**	\brief Exceptions
**
**	$Id$
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

#ifndef __SYNFIG_EXCEPTION_H
#define __SYNFIG_EXCEPTION_H

/* === H E A D E R S ======================================================= */

#include <stdexcept>
#include "string.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

namespace Exception {

class Exception : public std::runtime_error
{
public:
	Exception(const String &what);
};

class BadLinkName : public Exception
{
public:
	BadLinkName(const String &what);
}; // END of class BadLinkName

class BadType : public Exception
{
public:
	BadType(const String &what);
}; // END of class BadType

class IDAlreadyExists : public Exception
{
public:
	IDAlreadyExists(const String &what);
};

class NotFound : public Exception
{
public:
	NotFound(const String &what);
};

class IDNotFound : public NotFound
{
public:
	IDNotFound(const String &what);
};

class FileNotFound : public NotFound
{
public:
	FileNotFound(const String &what);
};

class BadTime : public Exception
{
public:
	BadTime(const String &what);
};

class BadFrameRate : public Exception
{
public:
	BadFrameRate(const String &what);
};

}; // END of namespace Exception

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
