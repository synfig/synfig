/* === S Y N F I G ========================================================= */
/*!	\file filecontainer.cpp
**	\brief FileContainer
**
**	\legal
**	......... ... 2013 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "filecontainer.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// ReadStream

FileContainer::ReadStream::ReadStream(FileSystem::Handle file_system):
	FileSystem::ReadStream(file_system) { }

FileContainer::ReadStream::~ReadStream()
{
	FileContainer::Handle container( FileContainer::Handle::cast_static(file_system_) );
	if (container->stream_valid_) container->file_close();
	container->stream_opened_ = false;
}

size_t FileContainer::ReadStream::internal_read(void *buffer, size_t size)
{
	FileContainer::Handle container( FileContainer::Handle::cast_static(file_system_) );
	if (!container->stream_valid_) return 0;
	return container->file_read(buffer, size);
}


// WriteStream

FileContainer::WriteStream::WriteStream(FileSystem::Handle file_system):
	FileSystem::WriteStream(file_system) { }

FileContainer::WriteStream::~WriteStream()
{
	FileContainer::Handle container( FileContainer::Handle::cast_static(file_system_) );
	if (container->stream_valid_) container->file_close();
	container->stream_opened_ = false;
}

size_t FileContainer::WriteStream::internal_write(const void *buffer, size_t size)
{
	FileContainer::Handle container( FileContainer::Handle::cast_static(file_system_) );
	if (!container->stream_valid_) return 0;
	return container->file_write(buffer, size);
}


// FileContainer

FileContainer::FileContainer(): stream_opened_(false), stream_valid_(false) { }

FileContainer::~FileContainer() { }

bool FileContainer::file_open_read_whole_container() { return false; }

void FileContainer::file_close() { stream_valid_ = false; }

FileSystem::ReadStream::Handle FileContainer::get_read_stream_whole_container()
{
	if (stream_opened_ || !file_open_read_whole_container())
		return ReadStream::Handle();
	stream_opened_ = true;
	stream_valid_ = true;
	return ReadStream::Handle(new ReadStream(this));
}

FileSystem::ReadStream::Handle FileContainer::get_read_stream(const String &filename)
{
	if (stream_opened_ || !file_open_read(filename))
		return ReadStream::Handle();
	stream_opened_ = true;
	stream_valid_ = true;
	return ReadStream::Handle(new ReadStream(this));
}

FileSystem::WriteStream::Handle FileContainer::get_write_stream(const String &filename)
{
	if (stream_opened_ || !file_open_write(filename))
		return WriteStream::Handle();
	stream_opened_ = true;
	stream_valid_ = true;
	return WriteStream::Handle(new WriteStream(this));
}



/* === E N T R Y P O I N T ================================================= */


