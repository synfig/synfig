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
		struct Entry
		{
			std::string prefix;
			Handle file_system;
			inline Entry() { }
			inline Entry(const std::string &prefix, const Handle &file_system):
				prefix(prefix), file_system(file_system) { }
		};

	private:
		std::list< Entry > entries_;

		bool find_system(const std::string &filename, FileSystem::Handle &out_file_system, std::string &out_filename);

	public:
		FileSystemGroup();
		explicit FileSystemGroup(Handle default_file_system);

		void register_system(const std::string &prefix, FileSystem::Handle file_system);
		void unregister_system(const std::string &prefix);

		virtual bool is_file(const std::string &filename);
		virtual bool is_directory(const std::string &filename);

		virtual bool directory_create(const std::string &dirname);

		virtual bool file_remove(const std::string &filename);
		virtual bool file_rename(const std::string &from_filename, const std::string &to_filename);
		virtual ReadStreamHandle get_read_stream(const std::string &filename);
		virtual WriteStreamHandle get_write_stream(const std::string &filename);
	};

}

/* === E N D =============================================================== */

#endif
