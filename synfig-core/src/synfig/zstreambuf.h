/* === S Y N F I G ========================================================= */
/*!	\file zstreambuf.h
**	\brief zstreambuf
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
		// From zlib documentation (https://zlib.net/manual.html):
		// windowBits can also be greater than 15 for optional gzip decoding.
		// Add 32 to windowBits to enable zlib and gzip decoding with automatic
		// header detection, or add 16 to decode only the gzip format
		enum compression : int {
			gzip        = 16+MAX_WBITS,
			zip_or_gzip = 32+MAX_WBITS,
			deflate     = -MAX_WBITS
		};
		enum {
			option_bufsize				= 32*1024,
			option_method				= Z_DEFLATED,
			option_compression_level	= Z_BEST_COMPRESSION,
			option_window_bits			= 16+MAX_WBITS,
			option_mem_level			= 9,
			option_strategy				= Z_DEFAULT_STRATEGY,

			fast_option_compression_level = Z_BEST_SPEED,
			fast_option_mem_level		= 9,
			fast_option_strategy		= Z_FIXED
		};

	private:
		std::streambuf *buf_;
		zstreambuf::compression compression_;

		bool inflate_initialized;
		z_stream inflate_stream_;
		std::vector<char> read_buffer_;

		bool deflate_initialized;
		z_stream deflate_stream_;
		std::vector<char> write_buffer_;

		bool inflate_buf();
		bool deflate_buf(bool flush);

	public:
		zstreambuf(std::streambuf *buf, zstreambuf::compression compression);
		virtual ~zstreambuf();

	protected:
		virtual int sync();
		virtual int underflow();
		virtual int overflow(int c = EOF);

	public:
		static bool pack(std::vector<char> &dest, const void *src, size_t size, bool fast = false);
		static size_t pack(void *dest, size_t dest_size, const void *src, size_t size, bool fast = false);
		static bool unpack(std::vector<char> &dest, const void *src, size_t size);
		static size_t unpack(void *dest, size_t dest_size, const void *src, size_t src_size);
	};

	class ZReadStream : public FileSystem::ReadStream
	{
	public:
		typedef etl::handle<ZReadStream> Handle;

	private:
		FileSystem::ReadStream::Handle stream_;
		zstreambuf buf_;
		std::istream istream_;

	protected:
		virtual size_t internal_read(void *buffer, size_t size)
			{ return (size_t)istream_.read((char*)buffer, size).gcount(); }

	public:
		ZReadStream(FileSystem::ReadStream::Handle stream, zstreambuf::compression compression):
			FileSystem::ReadStream(stream->file_system()),
			stream_(stream),
			buf_(stream_->rdbuf(), compression),
			istream_(&buf_)
		{ }

	};

	class ZWriteStream : public FileSystem::WriteStream
	{
	public:
		typedef etl::handle<ZWriteStream> Handle;

	private:
		FileSystem::WriteStream::Handle stream_;
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
		ZWriteStream(FileSystem::WriteStream::Handle stream):
			FileSystem::WriteStream(stream->file_system()),
			stream_(stream),
			buf_(stream_->rdbuf(), zstreambuf::compression::gzip),
			ostream_(&buf_)
		{ }
	};
}

/* === E N D =============================================================== */

#endif
