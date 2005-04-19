/* === S I N F G =========================================================== */
/*!	\file exception.h
**	\brief Exceptions
**
**	$Id: exception.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
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

#ifndef __SINFG_EXCEPTION_H
#define __SINFG_EXCEPTION_H

/* === H E A D E R S ======================================================= */

#include <stdexcept>
#include "string_decl.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfg {
	
namespace Exception {

class BadLinkName : public std::runtime_error
{
public:
	BadLinkName(const String &what);
}; // END of class BadLinkName

class BadType : public std::runtime_error
{
public:
	BadType(const String &what);
}; // END of class BadType

class IDAlreadyExists : public std::runtime_error
{
public:
	IDAlreadyExists(const String &what);
};

class NotFound : public std::runtime_error
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

class BadTime : public std::runtime_error
{
public:
	BadTime(const String &what);
};

class BadFrameRate : public std::runtime_error
{
public:
	BadFrameRate(const String &what);
};

}; // END of namespace Exception

}; // END of namespace sinfg

/* === E N D =============================================================== */

#endif
