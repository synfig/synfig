/* === S Y N F I G ========================================================= */
/*!	\file filecontainerzip.h
**	\brief FileContainerZip
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

#ifndef __SYNFIG_FILECONTAINERZIP_H
#define __SYNFIG_FILECONTAINERZIP_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <ctime>
#include "filecontainer.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileContainerZip: public FileContainer
	{
	public:
		class WholeZipReadStream : public FileSystem::ReadStream
		{
		protected:
			friend class FileContainerZip;
			WholeZipReadStream(Handle file_system);
		public:
			virtual ~WholeZipReadStream();
			virtual size_t read(void *buffer, size_t size);
		};

		typedef long long int file_size_t;

		struct HistoryRecord {
			file_size_t prev_storage_size;
			file_size_t storage_size;
			inline explicit HistoryRecord(file_size_t prev_storage_size = 0, file_size_t storage_size = 0):
				prev_storage_size(prev_storage_size), storage_size(storage_size) { }
		};

	private:
		struct FileInfo
		{
			std::string name;
			bool is_directory;
			bool directory_saved;
			file_size_t size;
			file_size_t header_offset;
			unsigned int compression;
			unsigned int crc32;
			time_t time;

			std::string name_part_directory;
			std::string name_part_localname;

			void split_name();

			inline FileInfo():
				is_directory(false), directory_saved(false),
				size(0), header_offset(0), compression(0), crc32(0), time(0) { }
		};

		typedef std::map< std::string, FileInfo > FileMap;

		FILE *storage_file_;
		FileMap files_;
		file_size_t prev_storage_size_;
		bool file_reading_whole_container_;
		bool file_reading_;
		bool file_writing_;
		FileMap::iterator file_;
		file_size_t file_processed_size_;
		bool changed_;

		static unsigned int crc32(unsigned int previous_crc, const void *buffer, size_t size);
		static std::string encode_history(const HistoryRecord &history_record);
		static HistoryRecord decode_history(const std::string &comment);
		static void read_history(std::list<HistoryRecord> &list, FILE *f, file_size_t size);

	public:
		FileContainerZip();
		virtual ~FileContainerZip();

		virtual bool create(const std::string &container_filename);
		virtual bool open(const std::string &container_filename);
		bool open_from_history(const std::string &container_filename, file_size_t truncate_storage_size = 0);
		virtual void close();
		virtual bool is_opened();
		bool save();

		static std::list<HistoryRecord> read_history(const std::string &container_filename);

		virtual bool is_file(const std::string &filename);
		virtual bool is_directory(const std::string &filename);

		bool directory_check_name(const std::string &dirname);
		virtual bool directory_create(const std::string &dirname);
		virtual bool directory_scan(const std::string &dirname, std::list< std::string > &out_files);

		virtual bool file_remove(const std::string &filename);

		bool file_check_name(const std::string &filename);
		virtual bool file_open_read_whole_container();
		virtual bool file_open_read(const std::string &filename);
		virtual bool file_open_write(const std::string &filename);
		virtual void file_close();

		virtual bool file_is_opened_for_read();
		virtual bool file_is_opened_for_write();

		virtual size_t file_read(void *buffer, size_t size);
		virtual size_t file_write(const void *buffer, size_t size);

		virtual ReadStreamHandle get_read_stream(const std::string &filename);
	};

}

/* === E N D =============================================================== */

#endif
