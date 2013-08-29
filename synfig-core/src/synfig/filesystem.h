/* === S Y N F I G ========================================================= */
/*!	\file filesystem.h
**	\brief FileSystem
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

#ifndef __SYNFIG_FILESYSTEM_H
#define __SYNFIG_FILESYSTEM_H

/* === H E A D E R S ======================================================= */

#include <cstdio>
#include <string>
#include <streambuf>
#include <istream>
#include <ostream>
#include <ETL/handle>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

	class FileSystem : public etl::rshared_object
	{
	public:
		typedef etl::handle< FileSystem > Handle;

		class Stream : public etl::rshared_object
		{
		protected:
			Handle file_system_;
			Stream(Handle file_system);
		public:
			virtual ~Stream();
		};

		class ReadStream : public Stream
		{
		protected:
			class istreambuf : public std::streambuf
			{
			private:
				ReadStream *stream_;
			public:
				istreambuf(ReadStream *stream): stream_(stream) { }
			protected:
		        virtual int underflow() { return stream_->getc(); }
			};

			istreambuf buf_;
			std::istream stream_;

			ReadStream(Handle file_system);
		public:
			virtual size_t read(void *buffer, size_t size) = 0;
			int getc();
			bool read_whole_block(void *buffer, size_t size);
			std::istream& stream() { return stream_; }
		};

		typedef etl::handle< ReadStream > ReadStreamHandle;

		class WriteStream : public Stream
		{
		protected:
			class ostreambuf : public std::streambuf
			{
			private:
				WriteStream *stream_;
			public:
				ostreambuf(WriteStream *stream): stream_(stream) { }
			protected:
		        virtual int overflow(int ch) { return stream_->putc(ch); }
			};

			ostreambuf buf_;
			std::ostream stream_;

			WriteStream(Handle file_system);
		public:
			virtual size_t write(const void *buffer, size_t size) = 0;
			int putc(int character);
			bool write_whole_block(const void *buffer, size_t size);
			bool write_whole_stream(ReadStreamHandle stream);
			std::ostream& stream() { return stream_; }
		};

		typedef etl::handle< WriteStream > WriteStreamHandle;

		FileSystem();
		virtual ~FileSystem();

		virtual bool file_remove(const std::string &filename) = 0;
		virtual bool file_rename(const std::string &from_filename, const std::string &to_filename);
		virtual ReadStreamHandle get_read_stream(const std::string &filename) = 0;
		virtual WriteStreamHandle get_write_stream(const std::string &filename) = 0;

		static bool copy(Handle from_file_system, const std::string &from_filename, Handle to_file_system, const std::string &to_filename);
	};

}

/* === E N D =============================================================== */

#endif
