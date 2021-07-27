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
		typedef std::shared_ptr<FileSystemGroup> Handle;

		struct Entry
		{
			String prefix;
			FileSystem::Handle sub_file_system;
			String sub_prefix;
			bool is_separator;

			inline Entry(): is_separator() { }
			inline Entry(const String &prefix, const FileSystem::Handle &sub_file_system, const String &sub_prefix, bool is_separator):
				prefix(prefix), sub_file_system(sub_file_system), sub_prefix(sub_prefix), is_separator(is_separator) { }
		};

	private:
		std::list< Entry > entries_;

		const Entry* find_system(const String &filename, FileSystem::Handle &out_file_system, String &out_filename);

	public:
		FileSystemGroup();
		explicit FileSystemGroup(FileSystem::Handle default_file_system);

		void register_system(const String &prefix, const FileSystem::Handle &sub_file_system, const String &sub_prefix = String(), bool is_separator = false);
		void unregister_system(const String &prefix);

		virtual bool is_file(const String &filename);
		virtual bool is_directory(const String &filename);

		virtual bool directory_create(const String &dirname);
		virtual bool directory_scan(const String &dirname, FileList &out_files);

		virtual bool file_remove(const String &filename);
		virtual bool file_rename(const String &from_filename, const String &to_filename);
		virtual FileSystem::ReadStream::Handle get_read_stream(const String &filename);
		virtual FileSystem::WriteStream::Handle get_write_stream(const String &filename);
		virtual String get_real_uri(const String &filename);
	};

}

/* === E N D =============================================================== */

#endif
