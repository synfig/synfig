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

FileSystemGroup::FileSystemGroup(Handle default_file_system)
{
	if (default_file_system) register_system(std::string(), default_file_system);
}

bool FileSystemGroup::find_system(const std::string &filename, Handle &out_file_system, std::string &out_filename)
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

void FileSystemGroup::register_system(const std::string &prefix, FileSystem::Handle file_system)
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

void FileSystemGroup::unregister_system(const std::string &prefix)
{
	for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end();)
		if (i->prefix == prefix)
			i = entries_.erase(i); else i++;
}

bool FileSystemGroup::is_file(const std::string &filename)
{
	Handle file_system;
	std::string internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->is_file(internal_filename);
}

bool FileSystemGroup::is_directory(const std::string &filename)
{
	Handle file_system;
	std::string internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->is_directory(internal_filename);
}

bool FileSystemGroup::directory_create(const std::string &dirname)
{
	Handle file_system;
	std::string internal_dirname;
	return find_system(dirname, file_system, internal_dirname)
	    && file_system->directory_create(internal_dirname);
}

bool FileSystemGroup::file_remove(const std::string &filename)
{
	Handle file_system;
	std::string internal_filename;
	return find_system(filename, file_system, internal_filename)
	    && file_system->file_remove(internal_filename);
}

bool FileSystemGroup::file_rename(const std::string &from_filename, const std::string &to_filename)
{
	// move file across file systems not supported
	Handle from_file_system, to_file_system;
	std::string from_internal_filename, to_internal_filename;
	return find_system(from_filename, from_file_system, from_internal_filename)
	    && find_system(to_filename, to_file_system, to_internal_filename)
	    && from_file_system == to_file_system
	    && from_file_system->file_rename(from_internal_filename, to_internal_filename);
}

FileSystem::ReadStreamHandle FileSystemGroup::get_read_stream(const std::string &filename)
{
	Handle file_system;
	std::string internal_filename;
	return find_system(filename, file_system, internal_filename)
	     ? file_system->get_read_stream(internal_filename)
	     : ReadStreamHandle();
}

FileSystem::WriteStreamHandle FileSystemGroup::get_write_stream(const std::string &filename)
{
	Handle file_system;
	std::string internal_filename;
	return find_system(filename, file_system, internal_filename)
	     ? file_system->get_write_stream(internal_filename)
	     : WriteStreamHandle();
}

/* === E N T R Y P O I N T ================================================= */


