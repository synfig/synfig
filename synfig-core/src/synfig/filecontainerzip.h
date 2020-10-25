/* === S Y N F I G ========================================================= */
/*!	\file filecontainerzip.h
**	\brief FileContainerZip
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
		typedef etl::handle<FileContainerZip> Handle;

		class WholeZipReadStream : public FileSystem::ReadStream
		{
		public:
			typedef etl::handle<WholeZipReadStream> Handle;
		protected:
			friend class FileContainerZip;
			WholeZipReadStream(FileSystem::Handle file_system);
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
			String name;
			bool is_directory;
			bool directory_saved;
			file_size_t size;
			file_size_t header_offset;
			unsigned int compression;
			unsigned int crc32;
			time_t time;

			String name_part_directory;
			String name_part_localname;

			void split_name();

			inline FileInfo():
				is_directory(false), directory_saved(false),
				size(0), header_offset(0), compression(0), crc32(0), time(0) { }
		};

		typedef std::map< String, FileInfo > FileMap;

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
		static String encode_history(const HistoryRecord &history_record);
		static HistoryRecord decode_history(const String &comment);
		static void read_history(std::list<HistoryRecord> &list, FILE *f, file_size_t size);

	public:
		FileContainerZip();
		virtual ~FileContainerZip();

		virtual bool create(const String &container_filename);
		virtual bool open(const String &container_filename);
		bool open_from_history(const String &container_filename, file_size_t truncate_storage_size = 0);
		virtual void close();
		virtual bool is_opened();
		bool save();

		static std::list<HistoryRecord> read_history(const String &container_filename);

		virtual bool is_file(const String &filename);
		virtual bool is_directory(const String &filename);

		bool directory_check_name(const String &dirname);
		virtual bool directory_create(const String &dirname);
		virtual bool directory_scan(const String &dirname, FileList &out_files);

		virtual bool file_remove(const String &filename);

		bool file_check_name(const String &filename);
		virtual bool file_open_read_whole_container();
		virtual bool file_open_read(const String &filename);
		virtual bool file_open_write(const String &filename);
		virtual void file_close();

		virtual bool file_is_opened_for_read();
		virtual bool file_is_opened_for_write();

		virtual size_t file_read(void *buffer, size_t size);
		virtual size_t file_write(const void *buffer, size_t size);

		virtual FileSystem::ReadStream::Handle get_read_stream(const String &filename);
	};

}

/* === E N D =============================================================== */

#endif
