/* === S Y N F I G ========================================================= */
/*!	\file tool/progress.h
**	\brief Progress class
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

#ifndef __SYNFIG_PROGRESS_H
#define __SYNFIG_PROGRESS_H

#include <synfig/string.h>
#include "definitions.h"

class Progress : public synfig::ProgressCallback
{
	const char *program;

public:

	Progress(const char *name):program(name) { }

	virtual bool
	task(const std::string& task)
	{
		VERBOSE_OUT(1)<<program<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	error(const std::string& task)
	{
		std::cerr<<program<<": "<<_("error")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	warning(const std::string& task)
	{
		std::cerr<<program<<": "<<_("warning")<<": "<<task<<std::endl;
		return true;
	}

	virtual bool
	amount_complete(int /*current*/, int /*total*/)
	{
		return true;
	}
};

#endif
