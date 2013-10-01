/* === S Y N F I G ========================================================= */
/*!	\file zstreambuf.h
**	\brief zstreambuf
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

#ifndef __SYNFIG_ZSTREAMBUF_H
#define __SYNFIG_ZSTREAMBUF_H

/* === H E A D E R S ======================================================= */

#include <streambuf>
#include <istream>
#include <ostream>
#include <vector>
#include <zlib.h>
#include "filesystem.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

	class zstreambuf : public std::streambuf
	{
	public:
		enum {
			option_bufsize				= 4096,
			option_method				= Z_DEFLATED,
			option_compression_level	= 9,
			option_window_bits			= 16+MAX_WBITS,
			option_mem_level			= 9,
			option_strategy				= Z_DEFAULT_STRATEGY
		};

	private:
		std::streambuf *buf_;

		bool inflate_initialized;
		z_stream inflate_stream_;
		std::vector<char> read_buffer_;

		bool deflate_initialized;
		z_stream deflate_stream_;
		std::vector<char> write_buffer_;

		bool inflate_buf();
		bool deflate_buf(bool flush);

	public:
		explicit zstreambuf(std::streambuf *buf);
		virtual ~zstreambuf();

	protected:
		virtual int sync();
		virtual int underflow();
		virtual int overflow(int c = EOF);
	};

	class ZReadStream : public FileSystem::ReadStream
	{
	private:
		FileSystem::ReadStreamHandle stream_;
		zstreambuf buf_;
		std::istream istream_;

	protected:
		virtual size_t internal_read(void *buffer, size_t size)
			{ return (size_t)istream_.read((char*)buffer, size).gcount(); }

	public:
		ZReadStream(FileSystem::ReadStreamHandle stream):
			FileSystem::ReadStream(stream->file_system()),
			stream_(stream),
			buf_(stream_->rdbuf()),
			istream_(&buf_)
		{ }

	};

	class ZWriteStream : public FileSystem::WriteStream
	{
	private:
		FileSystem::WriteStreamHandle stream_;
		zstreambuf buf_;
		std::ostream ostream_;

	protected:
		virtual size_t internal_write(const void *buffer, size_t size)
		{
			for(size_t i = 0; i < size; i++)
				if (!ostream_.put(((const char*)buffer)[i]).good())
					return i;
			return size;
		}

	public:
		ZWriteStream(FileSystem::WriteStreamHandle stream):
			FileSystem::WriteStream(stream->file_system()),
			stream_(stream),
			buf_(stream_->rdbuf()),
			ostream_(&buf_)
		{ }
	};
}

/* === E N D =============================================================== */

#endif
