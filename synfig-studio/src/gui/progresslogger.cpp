/* === S Y N F I G ========================================================= */
/*!	\file gui/progresslogger.cpp
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

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "progresslogger.h"
#endif


using namespace studio;

ProgressLogger::~ProgressLogger() { }

bool ProgressLogger::error(const std::string& task) {
	std::lock_guard<std::mutex> lock(mutex);
	if (!error_message.empty())
		error_message += '\n';
	error_message += task;
	return true;
}

void ProgressLogger::clear()
{
	std::lock_guard<std::mutex> lock(mutex);
	error_message.clear();
}

std::string ProgressLogger::get_error_message() const
{
	std::lock_guard<std::mutex> lock(mutex);
	return error_message;
}
