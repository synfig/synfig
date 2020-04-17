/* === S Y N F I G ========================================================= */
/*!	\file gui/progresslogger.h
**	\brief ProgressCallback to log error messages
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2020 Rodolfo Ribeiro Gomes
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

#ifndef SYNFIG_STUDIO_PROGRESSLOGGER_H
#define SYNFIG_STUDIO_PROGRESSLOGGER_H

#include <synfig/progresscallback.h>
#include <mutex>

namespace studio {

class ProgressLogger : public synfig::ProgressCallback
{
	mutable std::mutex mutex;
	std::string error_message;

public:
	virtual ~ProgressLogger();

	virtual bool task(const std::string &/*task*/) { return true; }
	virtual bool error(const std::string &task);
	virtual bool warning(const std::string &/*task*/) { return true; }
	virtual bool amount_complete(int /*current*/, int /*total*/) { return true; }

	virtual bool valid() const { return true; }

	virtual void clear();

	std::string get_error_message() const;
};

}
#endif // SYNFIG_STUDIO_PROGRESSLOGGER_H
