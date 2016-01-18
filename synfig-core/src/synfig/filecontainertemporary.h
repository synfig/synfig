/* === S Y N F I G ========================================================= */
/*!	\file filecontainertemporary.h
**	\brief FileContainerTemporary
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

#ifndef __SYNFIG_FILECONTAINERTEMPORARY_H
#define __SYNFIG_FILECONTAINERTEMPORARY_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <ctime>
#include <libxml++/libxml++.h>

#include "filesystemnative.h"
#include "filecontainerzip.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileContainerTemporary: public FileContainer
	{
	private:
		struct FileInfo
		{
			std::string name;
			std::string tmp_filename;
			bool is_directory;
			bool is_removed;

			std::string name_part_directory;
			std::string name_part_localname;

			void split_name();

			inline FileInfo(): is_directory(false), is_removed(false) { }
		};

		typedef std::map< std::string, FileInfo > FileMap;

		bool is_opened_;
		FileMap files_;
		std::string container_filename_;
		etl::handle< FileContainerZip > container_;
		etl::handle< FileSystemNative > file_system_;

		std::string file_;
		std::string file_tmp_name_;
		FileSystem::ReadStreamHandle file_read_stream_;
		FileSystem::WriteStreamHandle file_write_stream_;

		std::string temporary_filename_base_;

		static std::string get_xml_node_text(xmlpp::Node *node);

	public:
		FileContainerTemporary();
		virtual ~FileContainerTemporary();

		virtual bool create(const std::string &container_filename);
		virtual bool open(const std::string &container_filename);
		bool open_from_history(const std::string &container_filename, FileContainerZip::file_size_t truncate_storage_size = 0);
		virtual void close();
		virtual bool is_opened();

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

		static std::string get_temporary_directory();
		static std::string generate_temporary_filename_base();
		static std::string generate_temporary_filename();

		const std::string& get_container_filename() const { return container_filename_; }
		const std::string& get_temporary_filename_base() const { return temporary_filename_base_; }

		bool save_temporary() const;
		bool open_temporary(const std::string &filename_base);

		bool save_changes(const std::string &filename = std::string(), bool as_copy = false);
		void discard_changes();

		static std::string generate_indexed_temporary_filename(const FileSystem::Handle &fs, const std::string &filename);
	};

}

/* === E N D =============================================================== */

#endif
