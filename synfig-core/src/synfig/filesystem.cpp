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

FileSystem::ReadStream::ReadStream(Handle file_system):
Stream(file_system),
std::istream((std::streambuf*)this)
{
	setg(&buffer_ + 1, &buffer_ + 1, &buffer_ + 1);
}

int FileSystem::ReadStream::underflow()
{
	if (gptr() < egptr()) return std::streambuf::traits_type::to_int_type(*gptr());
	if (sizeof(buffer_) != internal_read(&buffer_, sizeof(buffer_))) return EOF;
	setg(&buffer_, &buffer_, &buffer_ + 1);
	return std::streambuf::traits_type::to_int_type(*gptr());
}

// WriteStream

FileSystem::WriteStream::WriteStream(Handle file_system):
Stream(file_system),
std::ostream((std::streambuf*)this)
{ }

int
FileSystem::WriteStream::overflow(int character)
{
	char c = std::streambuf::traits_type::to_char_type(character);
	return character != EOF && sizeof(c) == internal_write(&c, sizeof(c)) ? character : EOF;
}

// Identifier

FileSystem::ReadStreamHandle FileSystem::Identifier::get_read_stream() const
	{ return file_system ? file_system->get_read_stream(filename) : ReadStreamHandle(); }
FileSystem::WriteStreamHandle FileSystem::Identifier::get_write_stream() const
	{ return file_system ? file_system->get_write_stream(filename) : WriteStreamHandle(); }


// FileSystem

FileSystem::FileSystem() { }

FileSystem::~FileSystem() { }

bool FileSystem::file_rename(const std::string & /* from_filename */, const std::string & /* to_filename */)
{
	return false;
}

bool FileSystem::copy(Handle from_file_system, const std::string &from_filename, Handle to_file_system, const std::string &to_filename)
{
	if (from_file_system.empty() || to_file_system.empty()) return false;
	ReadStreamHandle read_stream = from_file_system->get_read_stream(from_filename);
	if (read_stream.empty()) return false;
	WriteStreamHandle write_stream = to_file_system->get_write_stream(to_filename);
	if (write_stream.empty()) return false;
	return write_stream->write_whole_stream(read_stream);
}

/* === E N T R Y P O I N T ================================================= */


