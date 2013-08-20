/* === S Y N F I G ========================================================= */
/*!	\file storagezip.h
**	\brief StorageZip
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

#ifndef __SYNFIG_STORAGEZIP_H
#define __SYNFIG_STORAGEZIP_H

/* === H E A D E R S ======================================================= */

#include <map>
#include "storage.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class StorageZip: public Storage
	{
	private:
		typedef long long int file_size_t;

		struct FileInfo
		{
			std::string name;
			bool is_directory;
			file_size_t size;
			file_size_t header_offset;
			unsigned int crc32;

			std::string name_part_directory;
			std::string name_part_localname;

			void split_name();

			inline FileInfo(): is_directory(false), size(0), header_offset(0), crc32(0) { }
		};

		typedef std::map< std::string, FileInfo > FileMap;

		FILE *storage_file_;
		FileMap files_;
		file_size_t prev_storage_size_;
		bool file_reading_;
		bool file_writing_;
		FileMap::iterator file_;
		file_size_t file_processed_size_;
		bool changed_;

	public:
		StorageZip();
		virtual ~StorageZip();

		virtual bool create(const std::string &storage_filename);
		virtual bool open(const std::string &storage_filename);
		virtual void close();
		virtual bool is_opened() = 0;

		virtual bool is_file(const std::string &filename);
		virtual bool is_directory(const std::string &filename);

		virtual bool directory_create(const std::string &dirname);
		virtual bool directory_scan(const std::string &dirname, std::list< std::string > &out_files);

		virtual bool file_remove(const std::string &filename);

		virtual bool file_open_read(const std::string &filename);
		virtual bool file_open_write(const std::string &filename);
		virtual void file_close();

		virtual bool file_is_opened_for_read();
		virtual bool file_is_opened_for_write();

		virtual size_t file_read(void *buffer, size_t size);
		virtual size_t file_write(const void *buffer, size_t size);
	};

}

/* === E N D =============================================================== */

#endif
