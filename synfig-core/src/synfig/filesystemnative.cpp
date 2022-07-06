/* === S Y N F I G ========================================================= */
/*!	\file filesystemnative.cpp
**	\brief FileSystemNative
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

#include <giomm.h>
#include <glibmm.h>
#include <glib/gstdio.h>

#include <ETL/stringf>

#include "filesystemnative.h"

#endif

/* === U S I N G =========================================================== */

namespace synfig {

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// ReadStream

FileSystemNative::ReadStream::ReadStream(FileSystem::Handle file_system, FILE *file):
	FileSystem::ReadStream(file_system), file_(file) { }

FileSystemNative::ReadStream::~ReadStream()
	{ fclose(file_); }

size_t FileSystemNative::ReadStream::internal_read(void *buffer, size_t size)
	{ return fread(buffer, 1, size, file_); }

std::istream::pos_type FileSystemNative::ReadStream::seekpos(std::istream::pos_type pos, std::ios_base::openmode which) {
	fseek(file_, pos, SEEK_SET);
	return ftell(file_);
}

int FileSystemNative::ReadStream::pbackfail(int ch)
{
	if (fseek(file_, -1, SEEK_CUR)) {
		return EOF;
	}
	return FileSystem::ReadStream::underflow();
}

// WriteStream

FileSystemNative::WriteStream::WriteStream(FileSystem::Handle file_system, FILE *file):
	FileSystem::WriteStream(file_system), file_(file) { }

FileSystemNative::WriteStream::~WriteStream()
	{ fclose(file_); }

size_t FileSystemNative::WriteStream::internal_write(const void *buffer, size_t size)
	{ return fwrite(buffer, 1, size, file_); }


// FileSystemNative

FileSystemNative::FileSystemNative() { }
FileSystemNative::~FileSystemNative() { }

bool FileSystemNative::is_file(const String &filename)
{
	return Gio::File::create_for_path(filename)->query_file_type()
	    == Gio::FILE_TYPE_REGULAR;
}

bool FileSystemNative::is_directory(const String &filename)
{
	return Gio::File::create_for_path(filename)->query_file_type()
	    == Gio::FILE_TYPE_DIRECTORY;
}

bool FileSystemNative::directory_create(const String &dirname)
{
	return is_directory(dirname)
	    || Gio::File::create_for_path(dirname)->make_directory();
}

bool FileSystemNative::directory_scan(const String &dirname, FileList &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;

	Glib::Dir dir(dirname);
	for(Glib::DirIterator i = dir.begin(); i != dir.end(); ++i)
		out_files.push_back(Glib::filename_to_utf8(*i));

	return true;
}

bool FileSystemNative::file_remove(const String &filename)
{
	return 0 == g_remove(filename.c_str());
}

bool FileSystemNative::file_rename(const String &from_filename, const String &to_filename)
{
	return 0 == g_rename(from_filename.c_str(), to_filename.c_str());
}


FileSystem::ReadStream::Handle FileSystemNative::get_read_stream(const String &filename)
{
	FILE *f = g_fopen(filename.c_str(), "rb");
	return f == nullptr
	     ? FileSystem::ReadStream::Handle()
	     : FileSystem::ReadStream::Handle(new ReadStream(this, f));
}

FileSystem::WriteStream::Handle FileSystemNative::get_write_stream(const String &filename)
{
	FILE *f = g_fopen(filename.c_str(), "wb");
	return f == nullptr
	     ? FileSystem::WriteStream::Handle()
	     : FileSystem::WriteStream::Handle(new WriteStream(this, f));
}

String FileSystemNative::get_real_uri(const String &filename)
{
	if (filename.empty()) return String();
	return Glib::filename_to_uri(etl::absolute_path(filename));
}


}
