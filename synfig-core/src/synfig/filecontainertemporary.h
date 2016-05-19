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

#include "filesystemnative.h"
#include "filecontainerzip.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace xmlpp { class Node; }

namespace synfig
{

	class FileContainerTemporary: public FileContainer
	{
	public:
		typedef etl::handle<FileContainerTemporary> Handle;

	private:
		struct FileInfo
		{
			String name;
			String tmp_filename;
			bool is_directory;
			bool is_removed;

			String name_part_directory;
			String name_part_localname;

			void split_name();

			inline FileInfo(): is_directory(false), is_removed(false) { }
		};

		typedef std::map< String, FileInfo > FileMap;

		bool is_opened_;
		FileMap files_;
		String container_filename_;
		FileContainerZip::Handle container_;
		FileSystemNative::Handle file_system_;

		String file_;
		String file_tmp_name_;
		FileSystem::ReadStream::Handle file_read_stream_;
		FileSystem::WriteStream::Handle file_write_stream_;

		String temporary_filename_base_;

		static String get_xml_node_text(xmlpp::Node *node);

	public:
		FileContainerTemporary();
		virtual ~FileContainerTemporary();

		virtual bool create(const String &container_filename);
		virtual bool open(const String &container_filename);
		bool open_from_history(const String &container_filename, FileContainerZip::file_size_t truncate_storage_size = 0);
		virtual void close();
		virtual bool is_opened();

		virtual bool is_file(const String &filename);
		virtual bool is_directory(const String &filename);

		virtual bool directory_create(const String &dirname);
		virtual bool directory_scan(const String &dirname, FileList &out_files);

		virtual bool file_remove(const String &filename);

		virtual bool file_open_read(const String &filename);
		virtual bool file_open_write(const String &filename);
		virtual void file_close();

		virtual bool file_is_opened_for_read();
		virtual bool file_is_opened_for_write();

		virtual size_t file_read(void *buffer, size_t size);
		virtual size_t file_write(const void *buffer, size_t size);

		static String get_temporary_directory();
		static String generate_temporary_filename_base();
		static String generate_temporary_filename();

		const String& get_container_filename() const { return container_filename_; }
		const String& get_temporary_filename_base() const { return temporary_filename_base_; }

		bool save_temporary() const;
		bool open_temporary(const String &filename_base);

		bool save_changes(const String &filename = String(), bool as_copy = false);
		void discard_changes();

		static String generate_indexed_temporary_filename(const FileSystem::Handle &fs, const String &filename);
	};

}

/* === E N D =============================================================== */

#endif
