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
			Handle file_system() const { return file_system_; }
		};

		class ReadStream :
			public Stream,
			private std::streambuf,
			public std::istream
		{
		protected:
			char buffer_;

			ReadStream(Handle file_system);
			virtual int underflow();
			virtual size_t internal_read(void *buffer, size_t size) = 0;

		public:
			size_t read_block(void *buffer, size_t size)
				{ return read((char*)buffer, size).gcount(); }
			bool read_whole_block(void *buffer, size_t size)
				{ return size == read_block(buffer, size); }
			template<typename T> bool read_variable(T &v)
				{ return read_whole_block(&v, sizeof(T)); }
		};

		typedef etl::handle< ReadStream > ReadStreamHandle;

		class WriteStream :
			public Stream,
			private std::streambuf,
			public std::ostream
		{
		protected:
			WriteStream(Handle file_system);
	        virtual int overflow(int ch);
			virtual size_t internal_write(const void *buffer, size_t size) = 0;

		public:
			bool write_block(const void *buffer, size_t size)
			{
				for(size_t i = 0; i < size; i++)
					if (!put(((const char*)buffer)[i]).good())
						return i;
				return size;
			}
			bool write_whole_block(const void *buffer, size_t size)
				{ return size == write_block(buffer, size); }
			bool write_whole_stream(std::streambuf &streambuf)
				{ return (*this << &streambuf).good(); }
			bool write_whole_stream(std::istream &stream)
				{ return write_whole_stream(*stream.rdbuf()); }
			bool write_whole_stream(ReadStreamHandle stream)
				{ return !stream || write_whole_stream(*(std::istream*)&(*stream)); }
			template<typename T> bool write_variable(const T &v)
				{ return write_whole_block(&v, sizeof(T)); }
		};

		typedef etl::handle< WriteStream > WriteStreamHandle;

		class Identifier {
		public:
			Handle file_system;
			std::string filename;
			Identifier() { }
			Identifier(const Handle &file_system, const std::string &filename):
				file_system(file_system), filename(filename) { }

			bool empty() const { return file_system; }
			operator bool () const { return !empty(); }

			bool operator < (const Identifier &other) const
			{
				if (file_system.get() < other.file_system.get()) return true;
				if (other.file_system.get() < file_system.get()) return false;
				if (filename < other.filename) return true;
				if (other.filename < filename) return false;
				return false;
			}
			bool operator > (const Identifier &other) const
				{ return other < *this; }
			bool operator != (const Identifier &other) const
				{ return *this > other || other < *this; }
			bool operator == (const Identifier &other) const
				{ return !(*this != other); }

			ReadStreamHandle get_read_stream() const;
			WriteStreamHandle get_write_stream() const;
		};

		FileSystem();
		virtual ~FileSystem();

		virtual bool is_file(const std::string &filename) = 0;
		virtual bool is_directory(const std::string &filename) = 0;

		virtual bool directory_create(const std::string &dirname) = 0;

		virtual bool file_remove(const std::string &filename) = 0;
		virtual bool file_rename(const std::string &from_filename, const std::string &to_filename);
		virtual ReadStreamHandle get_read_stream(const std::string &filename) = 0;
		virtual WriteStreamHandle get_write_stream(const std::string &filename) = 0;

		inline bool is_exists(const std::string filename) { return is_file(filename) || is_directory(filename); }

		Identifier get_identifier(const std::string &filename) { return Identifier(this, filename); }
		static bool copy(Handle from_file_system, const std::string &from_filename, Handle to_file_system, const std::string &to_filename);

		static std::string fix_slashes(const std::string &filename);

		///!@brief Read a stream line by line even '\r\n' ended
		static std::istream& safeGetline(std::istream& is, std::string& t);
	};

}

/* === E N D =============================================================== */

#endif
