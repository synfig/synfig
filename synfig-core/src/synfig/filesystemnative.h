/* === S Y N F I G ========================================================= */
/*!	\file filesystemnative.h
**	\brief FileSystemNative
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

#ifndef __SYNFIG_FILESYSTEMNATIVE_H
#define __SYNFIG_FILESYSTEMNATIVE_H

/* === H E A D E R S ======================================================= */

#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileSystemNative : public FileSystem
	{
	public:
		typedef etl::handle<FileSystemNative> Handle;

		class ReadStream : public FileSystem::ReadStream
		{
		public:
			typedef etl::handle<ReadStream> Handle;
		protected:
			friend class FileSystemNative;
			FILE *file_;
			ReadStream(FileSystem::Handle file_system, FILE *file);
			virtual size_t internal_read(void *buffer, size_t size);
			std::istream::pos_type seekpos(std::istream::pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
		public:
			virtual ~ReadStream();
		};

		class WriteStream : public FileSystem::WriteStream
		{
		public:
			typedef etl::handle<ReadStream> Handle;
		protected:
			friend class FileSystemNative;
			FILE *file_;
			WriteStream(FileSystem::Handle file_system, FILE *file_);
			virtual size_t internal_write(const void *buffer, size_t size);
		public:
			virtual ~WriteStream();
		};

	private:
		FileSystemNative();

	public:
		static const Handle& instance() {
			static const Handle fs_instance_(new FileSystemNative());
			return fs_instance_;
		}

		virtual ~FileSystemNative();

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
