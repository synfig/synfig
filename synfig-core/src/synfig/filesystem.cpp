/* === S Y N F I G ========================================================= */
/*!	\file filesystem.cpp
**	\brief FileSystem
**
**	$Id$
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

#include "filesystem.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */


// Stream

FileSystem::Stream::Stream(Handle file_system): file_system_(file_system) { }
FileSystem::Stream::~Stream() { }


// ReadStream

FileSystem::ReadStream::ReadStream(Handle file_system): Stream(file_system) { }

int
FileSystem::ReadStream::getc()
{
	int character = 0;
	return 1 == read(&character, 1) ? character : EOF;
}

bool
FileSystem::ReadStream::read_whole_block(void *buffer, size_t size)
{
	return size == read(buffer, size);
}


// WriteStream

FileSystem::WriteStream::WriteStream(Handle file_system): Stream(file_system) { }

int
FileSystem::WriteStream::putc(int character)
{
	return 1 == write(&character, 1) ? character : EOF;
}

bool
FileSystem::WriteStream::write_whole_block(const void *buffer, size_t size)
{
	return size == write(buffer, size);
}


// FileSystem

FileSystem::FileSystem() { }
FileSystem::~FileSystem() { }

/* === E N T R Y P O I N T ================================================= */


