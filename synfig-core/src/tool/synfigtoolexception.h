/* === S Y N F I G ========================================================= */
/*!	\file tool/synfigtoolexception.h
**	\brief Exception class for Synfig Tool
**
**	$Id$
**
**	\legal
**	Copyright (c) 2012 Diego Barrios Romero
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

#ifndef __SYNFIG_TOOLEXCEPTION_H
#define __SYNFIG_TOOLEXCEPTION_H

#include <exception>

class SynfigToolException : public std::exception
{
public:
	SynfigToolException(exit_code code, std::string msg = "")
		: _code(code), _msg(msg)
	{ }
	exit_code get_exit_code() const { return _code; }
	std::string get_message() const { return _msg; }

	virtual const char* what() const throw() { return _msg.c_str(); }
	virtual ~SynfigToolException() throw() {}
private:
	exit_code _code;
	std::string _msg;
};

#endif
