/* === S Y N F I G ========================================================= */
/*!	\file filesystemgroup.h
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_FILESYSTEMGROUP_H
#define __SYNFIG_FILESYSTEMGROUP_H

/* === H E A D E R S ======================================================= */

#include <list>
#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileSystemGroup : public FileSystem
	{
	public:
		typedef etl::handle<FileSystemGroup> Handle;

		struct Entry
		{
			String prefix;
			FileSystem::Handle file_system;
			inline Entry() { }
			inline Entry(const String &prefix, const FileSystem::Handle &file_system):
				prefix(prefix), file_system(file_system) { }
		};

	private:
		std::list< Entry > entries_;

		bool find_system(const String &filename, FileSystem::Handle &out_file_system, String &out_filename);

	public:
		FileSystemGroup();
		explicit FileSystemGroup(FileSystem::Handle default_file_system);

		void register_system(const String &prefix, FileSystem::Handle file_system);
		void unregister_system(const String &prefix);

		virtual bool is_file(const String &filename);
		virtual bool is_directory(const String &filename);

		virtual bool directory_create(const String &dirname);

		virtual bool file_remove(const String &filename);
		virtual bool file_rename(const String &from_filename, const String &to_filename);
		virtual FileSystem::ReadStream::Handle get_read_stream(const String &filename);
		virtual FileSystem::WriteStream::Handle get_write_stream(const String &filename);
		virtual String get_real_uri(const String &filename);
	};

}

/* === E N D =============================================================== */

#endif
