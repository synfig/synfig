/* === S Y N F I G ========================================================= */
/*!	\file filesystemgroup.cpp
**	\brief FileSystemGroup
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

#include "filesystemgroup.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

FileSystemGroup::FileSystemGroup() { }

FileSystemGroup::FileSystemGroup(FileSystem::Handle default_file_system)
{
	if (default_file_system) register_system(String(), default_file_system);
}

bool FileSystemGroup::find_system(const String &filename, FileSystem::Handle &out_file_system, String &out_filename)
{
	for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end(); i++)
	{
		if (filename.substr(0, i->prefix.size()) == i->prefix)
		{
			out_file_system = i->file_system;
			out_filename = filename.substr(i->prefix.size());
			return true;
		}
	}

	out_file_system.reset();
	out_filename.clear();
	return false;
}

void FileSystemGroup::register_system(const String &prefix, FileSystem::Handle file_system)
{
	if (file_system)
	{
		// keep list sorted by length of prefix desc
		for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end(); i++)
		{
			if (i->prefix == prefix)
			{
				i->file_system = file_system;
				return;
			}
			if (i->prefix.size() <= prefix.size())
			{
				entries_.insert( i, Entry(prefix, file_system) );
				return;
			}
		}
		entries_.push_back( Entry(prefix, file_system) );
	}
}

void FileSystemGroup::unregister_system(const String &prefix)
{
	for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end();)
		if (i->prefix == prefix)
			i = entries_.erase(i); else i++;
}

bool FileSystemGroup::is_file(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->is_file(internal_filename);
}

bool FileSystemGroup::is_directory(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->is_directory(internal_filename);
}

bool FileSystemGroup::directory_create(const String &dirname)
{
	FileSystem::Handle file_system;
	String internal_dirname;
	return find_system(dirname, file_system, internal_dirname)
	    && file_system->directory_create(internal_dirname);
}

bool FileSystemGroup::file_remove(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->file_remove(internal_filename);
}

bool FileSystemGroup::file_rename(const String &from_filename, const String &to_filename)
{
	FileSystem::Handle from_file_system, to_file_system;
	String from_internal_filename, to_internal_filename;

	if (!find_system(from_filename, from_file_system, from_internal_filename))
		return false;
	if (!find_system(to_filename, to_file_system, to_internal_filename))
		return false;

	if (from_file_system == to_file_system)
	    return from_file_system->file_rename(from_internal_filename, to_internal_filename);

	// move file across file systems
	return FileSystem::file_rename(from_filename, to_filename);
}

FileSystem::ReadStream::Handle FileSystemGroup::get_read_stream(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
	     ? file_system->get_read_stream(internal_filename)
	     : FileSystem::ReadStream::Handle();
}

FileSystem::WriteStream::Handle FileSystemGroup::get_write_stream(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
	     ? file_system->get_write_stream(internal_filename)
	     : FileSystem::WriteStream::Handle();
}

String FileSystemGroup::get_real_uri(const String &filename)
{
	FileSystem::Handle file_system;
	String internal_filename;
	return find_system(filename, file_system, internal_filename)
		 ? file_system->get_real_uri(internal_filename)
		 : String();
}

/* === E N T R Y P O I N T ================================================= */


