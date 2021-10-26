/* === S Y N F I G ========================================================= */
/*!	\file filesystemgroup.cpp
**	\brief FileSystemGroup
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

#include <set>

#include <ETL/stringf>

#include "filesystemgroup.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

FileSystemGroup::FileSystemGroup() { }

FileSystemGroup::FileSystemGroup(FileSystem::Handle default_file_system)
{
	if (default_file_system) register_system(String(), default_file_system, String(), true);
}

const FileSystemGroup::Entry* FileSystemGroup::find_system(const String &filename, FileSystem::Handle &out_file_system, String &out_filename)
{
	String clean_filename = etl::cleanup_path(filename);
	for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end(); i++)
	{
		if ( clean_filename.substr(0, i->prefix.size()) == i->prefix
		  && ( i->is_separator
			|| clean_filename.size() == i->prefix.size()
			|| clean_filename[i->prefix.size()] == ETL_DIRECTORY_SEPARATOR0
			|| clean_filename[i->prefix.size()] == ETL_DIRECTORY_SEPARATOR1 ))
		{
			String sub_name = clean_filename.substr(i->prefix.size());
			if (!i->prefix.empty() && !sub_name.empty() && (sub_name[0] == ETL_DIRECTORY_SEPARATOR0 || sub_name[0] == ETL_DIRECTORY_SEPARATOR1))
				sub_name = sub_name.substr(1);
			out_file_system = i->sub_file_system;
			out_filename = i->sub_prefix.empty() || sub_name.empty()
						 ? i->sub_prefix + sub_name
				         : i->sub_prefix + ETL_DIRECTORY_SEPARATOR + sub_name;
			return &*i;
		}
	}

	out_file_system.reset();
	out_filename.clear();
	return NULL;
}

void FileSystemGroup::register_system(const String &prefix, const FileSystem::Handle &sub_file_system, const String &sub_prefix, bool is_separator)
{
	if (sub_file_system)
	{
		// keep list sorted by length of prefix desc
		for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end(); i++)
		{
			if (i->prefix == prefix)
			{
				i->sub_file_system = sub_file_system;
				i->sub_prefix = sub_prefix;
				i->is_separator = is_separator;
				return;
			}
			if (i->prefix.size() <= prefix.size())
			{
				entries_.insert( i, Entry(prefix, sub_file_system, sub_prefix, is_separator) );
				return;
			}
		}
		entries_.push_back( Entry(prefix, sub_file_system, sub_prefix, is_separator) );
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
	if (filename.empty()) return true;
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

bool FileSystemGroup::directory_scan(const String &dirname, FileList &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;

	std::set<String> files;

	FileSystem::Handle file_system;
	String internal_dirname;
	if (find_system(dirname, file_system, internal_dirname))
	{
		FileList list;
		if (!file_system->directory_scan(internal_dirname, list))
			return false;
		for(FileList::const_iterator i = list.begin(); i != list.end(); ++i)
			files.insert(*i);
	}

	String clean_dirname = etl::cleanup_path(dirname);
	for(std::list< Entry >::iterator i = entries_.begin(); i != entries_.end(); i++)
		if (!i->is_separator && !i->prefix.empty() && clean_dirname == etl::dirname(i->prefix))
		{
			if (is_exists(i->prefix))
				files.insert(etl::basename(i->prefix));
			else
				files.erase(etl::basename(i->prefix));
		}

	for(std::set<String>::const_iterator i = files.begin(); i != files.end(); ++i)
		out_files.push_back(*i);

	return true;
}

bool FileSystemGroup::file_remove(const String &filename)
{
	if (!is_exists(filename)) return true;
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


