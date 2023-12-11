/* === S Y N F I G ========================================================= */
/*!	\file filesystem.cpp
**	\brief FileSystem
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

#include <glibmm.h>
#include <cstdio>

#include "filesystem.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

// FileSystemEmpty

FileSystemEmpty::FileSystemEmpty() { }
FileSystemEmpty::~FileSystemEmpty() { }

// Stream

FileSystem::Stream::Stream(FileSystem::Handle file_system):
	file_system_(file_system) { }

FileSystem::Stream::~Stream()
	{ }

// ReadStream

FileSystem::ReadStream::ReadStream(FileSystem::Handle file_system):
	Stream(file_system),
	std::istream((std::streambuf*)this),
	buffer_(0)
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

FileSystem::WriteStream::WriteStream(FileSystem::Handle file_system):
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

FileSystem::ReadStream::Handle FileSystem::Identifier::get_read_stream() const
	{ return file_system ? file_system->get_read_stream(filename.u8string()) : ReadStream::Handle(); }
FileSystem::WriteStream::Handle FileSystem::Identifier::get_write_stream() const
	{ return file_system ? file_system->get_write_stream(filename.u8string()) : WriteStream::Handle(); }


// FileSystem

FileSystem::FileSystem() { }

FileSystem::~FileSystem() { }

bool FileSystem::file_rename(const String &from_filename, const String &to_filename)
{
	if (fix_slashes(from_filename) == fix_slashes(to_filename))
		return true;
	ReadStream::Handle read_stream = get_read_stream(from_filename);
	if (!read_stream) return false;
	WriteStream::Handle write_stream = get_write_stream(to_filename);
	if (!write_stream) return false;
	return write_stream->write_whole_stream(read_stream)
		&& file_remove(from_filename);
}

bool FileSystem::directory_create_recursive(const String &dirname) {
	return is_directory(dirname)
		|| (directory_create_recursive(filesystem::Path::dirname(dirname)) && directory_create(dirname));
}

bool FileSystem::remove_recursive(const String &filename)
{
	assert(!filename.empty());

	if (filename.empty())
		return false;
	if (is_file(filename))
		return file_remove(filename);
	if (is_directory(filename))
	{
		FileList files;
		directory_scan(filename, files);
		bool success = true;
		for(FileList::const_iterator i = files.begin(); i != files.end(); ++i)
			if (!remove_recursive(filename + ETL_DIRECTORY_SEPARATOR + *i))
				success = false;
		return success;
	}
	return true;
}

bool FileSystem::copy(Handle from_file_system, const String &from_filename, Handle to_file_system, const String &to_filename)
{
	if (from_file_system.empty() || to_file_system.empty()) return false;
	ReadStream::Handle read_stream = from_file_system->get_read_stream(from_filename);
	if (read_stream.empty()) return false;
	WriteStream::Handle write_stream = to_file_system->get_write_stream(to_filename);
	if (write_stream.empty()) return false;
	return write_stream->write_whole_stream(read_stream);
}

bool FileSystem::copy_recursive(Handle from_file_system, const String &from_filename, Handle to_file_system, const String &to_filename)
{
	if (!from_file_system || !to_file_system) return false;
	if (from_file_system->is_file(from_filename))
		return copy(from_file_system, from_filename, to_file_system, to_filename);
	if (from_file_system->is_directory(from_filename))
	{
		if (!to_file_system->directory_create(to_filename))
			return false;
		FileList files;
		bool success = from_file_system->directory_scan(from_filename, files);
		for(FileList::const_iterator i = files.begin(); i != files.end(); ++i)
			if (!copy_recursive(
					from_file_system,
					from_filename + ETL_DIRECTORY_SEPARATOR + *i,
					to_file_system,
					to_filename + ETL_DIRECTORY_SEPARATOR + *i ))
				success = false;
		return success;
	}
	return false;
}

String FileSystem::fix_slashes(const String &filename)
{
	String fixed = filesystem::Path::cleanup_path(filename);
	if (fixed == ".")
		return String();

	String::size_type i = 0;
	// For MS Windows shared folder paths like \\host\folder\file,
	// we keep \\ for now
	if (fixed.size() > 2 && fixed.substr(0, 2) == "\\\\")
		i = 2;
	// All other backslashes \ are replaced with slashes /
	for(; i < fixed.size(); ++i)
		if (fixed[i] == '\\') fixed[i] = '/';
	return fixed;
}

std::istream&
FileSystem::safe_get_line(std::istream& is, String& t)
{
	t.clear();
	//code from http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
		case '\n':
			return is;
		case '\r':
			if(sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if(t.empty())
				is.setstate(std::ios::eofbit);
			return is;
		default:
			t += (char)c;
		}
	}

	return is;
}

String FileSystem::get_real_uri(const String & /* filename */)
	{ return String(); }

String FileSystem::get_real_filename(const String &filename) {
	return Glib::filename_from_uri(get_real_uri(filename));
}

/* === E N T R Y P O I N T ================================================= */


