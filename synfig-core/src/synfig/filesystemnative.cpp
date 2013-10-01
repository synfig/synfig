/* === S Y N F I G ========================================================= */
/*!	\file filesystemnative.cpp
**	\brief FileSystemNative
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

#include "filesystemnative.h"
#include <giomm.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

const etl::handle< FileSystemNative > FileSystemNative::instance__(new FileSystemNative);

// ReadStream

FileSystemNative::ReadStream::ReadStream(Handle file_system, FILE *file):
	FileSystem::ReadStream(file_system), file_(file) { }

FileSystemNative::ReadStream::~ReadStream()
	{ fclose(file_); }

size_t FileSystemNative::ReadStream::internal_read(void *buffer, size_t size)
	{ return fread(buffer, 1, size, file_); }


// WriteStream

FileSystemNative::WriteStream::WriteStream(Handle file_system, FILE *file):
	FileSystem::WriteStream(file_system), file_(file) { }

FileSystemNative::WriteStream::~WriteStream()
	{ fclose(file_); }

size_t FileSystemNative::WriteStream::internal_write(const void *buffer, size_t size)
	{ return fwrite(buffer, 1, size, file_); }


// FileSystemNative

FileSystemNative::FileSystemNative() { }
FileSystemNative::~FileSystemNative() { }

bool FileSystemNative::is_file(const std::string &filename)
{
	return Gio::File::create_for_path(filename)->query_file_type()
	    == Gio::FILE_TYPE_REGULAR;
}

bool FileSystemNative::is_directory(const std::string &filename)
{
	return Gio::File::create_for_path(filename)->query_file_type()
	    == Gio::FILE_TYPE_DIRECTORY;
}

bool FileSystemNative::directory_create(const std::string &dirname)
{
	return is_directory(dirname)
	    || Gio::File::create_for_path(dirname)->make_directory();
}

bool FileSystemNative::file_remove(const std::string &filename)
{
	return 0 == remove(filename.c_str());
}

bool FileSystemNative::file_rename(const std::string &from_filename, const std::string &to_filename)
{
	return 0 == rename(from_filename.c_str(), to_filename.c_str());
}


FileSystem::ReadStreamHandle FileSystemNative::get_read_stream(const std::string &filename)
{
#ifdef WIN32
	FILE *f = fopen(Glib::locale_from_utf8(filename).c_str(), "rb");
#else
	FILE *f = fopen(filename.c_str(), "rb");
#endif
	return f == NULL
	     ? ReadStreamHandle()
	     : ReadStreamHandle(new ReadStream(this, f));
}

FileSystem::WriteStreamHandle FileSystemNative::get_write_stream(const std::string &filename)
{
#ifdef WIN32
	FILE *f = fopen(Glib::locale_from_utf8(filename).c_str(), "wb");
#else
	FILE *f = fopen(filename.c_str(), "wb");
#endif
	return f == NULL
	     ? WriteStreamHandle()
	     : WriteStreamHandle(new WriteStream(this, f));
}


/* === E N T R Y P O I N T ================================================= */


