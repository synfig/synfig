/* === S Y N F I G ========================================================= */
/*!	\file filecontainer.h
**	\brief FileContainer
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

#ifndef __SYNFIG_FILECONTAINER_H
#define __SYNFIG_FILECONTAINER_H

/* === H E A D E R S ======================================================= */

#include <list>
#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileContainer : public FileSystem
	{
	public:
		typedef std::shared_ptr<FileContainer> Handle;

		class ReadStream : public FileSystem::ReadStream
		{
		public:
			typedef std::shared_ptr<ReadStream> Handle;
		protected:
			friend class FileContainer;
			ReadStream(FileSystem::Handle file_system);
			virtual size_t internal_read(void *buffer, size_t size);
		public:
			virtual ~ReadStream();
		};

		class WriteStream : public FileSystem::WriteStream
		{
		public:
			typedef std::shared_ptr<WriteStream> Handle;
		protected:
			friend class FileContainer;
			WriteStream(FileSystem::Handle file_system);
			virtual size_t internal_write(const void *buffer, size_t size);
		public:
			virtual ~WriteStream();
		};

	protected:
		bool stream_opened_;
		bool stream_valid_;

	public:
		FileContainer();
		virtual ~FileContainer();

		virtual bool create(const String &container_filename) = 0;
		virtual bool open(const String &container_filename) = 0;
		virtual void close() = 0;
		virtual bool is_opened() = 0;

		virtual bool file_open_read_whole_container();
		virtual bool file_open_read(const String &filename) = 0;
		virtual bool file_open_write(const String &filename) = 0;
		virtual void file_close();

		virtual bool file_is_opened_for_read() = 0;
		virtual bool file_is_opened_for_write() = 0;

		virtual size_t file_read(void *buffer, size_t size) = 0;
		virtual size_t file_write(const void *buffer, size_t size) = 0;

		inline bool file_is_opened()
		{
			return file_is_opened_for_read() || file_is_opened_for_write();
		}

		FileSystem::ReadStream::Handle get_read_stream_whole_container();
		virtual FileSystem::ReadStream::Handle get_read_stream(const String &filename);
		virtual FileSystem::WriteStream::Handle get_write_stream(const String &filename);
	};

}

/* === E N D =============================================================== */

#endif
