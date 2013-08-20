/* === S Y N F I G ========================================================= */
/*!	\file storage.h
**	\brief Storage
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

#ifndef __SYNFIG_STORAGE_H
#define __SYNFIG_STORAGE_H

/* === H E A D E R S ======================================================= */

#include <cstdio>
#include <string>
#include <list>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class Storage
	{
	public:
		Storage();
		virtual ~Storage();

		virtual bool create(const std::string &storage_filename) = 0;
		virtual bool open(const std::string &storage_filename) = 0;
		virtual void close() = 0;
		virtual bool is_opened() = 0;

		virtual bool is_file(const std::string &filename) = 0;
		virtual bool is_directory(const std::string &filename) = 0;

		virtual bool directory_create(const std::string &dirname) = 0;
		virtual bool directory_scan(const std::string &dirname, std::list< std::string > &out_files) = 0;

		virtual bool file_remove(const std::string &filename) = 0;

		virtual bool file_open_read(const std::string &filename) = 0;
		virtual bool file_open_write(const std::string &filename) = 0;
		virtual void file_close() = 0;

		virtual bool file_is_opened_for_read() = 0;
		virtual bool file_is_opened_for_write() = 0;

		virtual size_t file_read(void *buffer, size_t size) = 0;
		virtual size_t file_write(const void *buffer, size_t size) = 0;

		inline bool file_is_opened()
		{
			return file_is_opened_for_read() || file_is_opened_for_write();
		}

		inline bool is_exists(const std::string filename) { return is_file(filename) || is_directory(filename); }

	};

}

/* === E N D =============================================================== */

#endif
