/* === S Y N F I G ========================================================= */
/*!	\file zstreambuf.cpp
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <cstring>
#include "zstreambuf.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

zstreambuf::zstreambuf(std::streambuf *buf):
	buf_(buf),
	inflate_initialized(false),
	deflate_initialized(false)
{
}

zstreambuf::~zstreambuf()
{
	sync();
	if (inflate_initialized) inflateEnd(&inflate_stream_);
	if (deflate_initialized) deflateEnd(&deflate_stream_);
}

bool zstreambuf::inflate_buf()
{
    // initialize inflate if need
    if (!inflate_initialized)
    {
    	memset(&inflate_stream_, 0, sizeof(inflate_stream_));
    	if (Z_OK != inflateInit2(&inflate_stream_, option_window_bits)) return false;
    	inflate_initialized = true;
    }

    // read and inflate new chunk of data
    char in_buf[option_bufsize];
    inflate_stream_.avail_in = buf_->sgetn(in_buf, sizeof(in_buf));
    inflate_stream_.next_in = (Bytef*)in_buf;
	read_buffer_.resize(0);
	do
	{
		inflate_stream_.avail_out = option_bufsize;
		read_buffer_.resize(read_buffer_.size() + inflate_stream_.avail_out);
		inflate_stream_.next_out = (Bytef*)(&read_buffer_.back() + 1 - inflate_stream_.avail_out);
		int ret = ::inflate(&inflate_stream_, Z_NO_FLUSH);
		read_buffer_.resize(read_buffer_.size() - inflate_stream_.avail_out);
		if (ret != Z_OK) break;
	} while (inflate_stream_.avail_out == 0);
	assert(inflate_stream_.avail_in == 0);

	// nothing to read
	if (read_buffer_.empty()) return false;

	// set new read buffer
	char *pointer = &read_buffer_.front();
    setg(pointer, pointer, pointer + read_buffer_.size());
    return true;
}

bool zstreambuf::deflate_buf(bool flush)
{
	if (pbase() != NULL && pptr() > pbase())
	{
		// initialize deflate if need
		if (!deflate_initialized)
		{
			memset(&deflate_stream_, 0, sizeof(deflate_stream_));

			if (Z_OK != deflateInit2(&deflate_stream_,
					option_compression_level,
					option_method,
					option_window_bits,
					option_mem_level,
					option_strategy
			)) return false;

			deflate_initialized = true;
		}

		// deflate and write new chunk of data
		char out_buf[option_bufsize];
		deflate_stream_.avail_in = (uInt)(pptr() - pbase());
		deflate_stream_.next_in = (Bytef*)pbase();
		do
		{
			deflate_stream_.avail_out = sizeof(out_buf);
			deflate_stream_.next_out = (Bytef*)out_buf;
			if (Z_STREAM_ERROR == deflate(&deflate_stream_, flush ? Z_FINISH : Z_NO_FLUSH))
				return false;
			if (deflate_stream_.avail_out < sizeof(out_buf))
				buf_->sputn(out_buf, sizeof(out_buf) - deflate_stream_.avail_out);
		} while (deflate_stream_.avail_out == 0);
		assert(deflate_stream_.avail_in == 0);
		setp(NULL, NULL);
	}
	return true;
}

int zstreambuf::sync()
{
	bool deflate_success = deflate_buf(true);
	bool buf_sync_success = 0 == buf_->pubsync();
	return deflate_success && buf_sync_success ? 0 : -1;
}

int zstreambuf::underflow()
{
	// is it actually underflow?
    if (gptr() < egptr()) return traits_type::to_int_type(*gptr());
    if (!inflate_buf()) return EOF;
	return *(unsigned char *)gptr();
}

int zstreambuf::overflow(int c)
{
	// flush
	if (c == EOF) { sync(); return EOF; }

	// save data and prepare new buffer
	if (pptr() >= epptr())
	{
		if (!deflate_buf(false)) return EOF;
		if (write_buffer_.size() < option_bufsize) write_buffer_.resize(option_bufsize);
		char *pointer = &write_buffer_.front();
		setp(pointer, pointer + write_buffer_.size());
	}

	// put character
	*pptr() = traits_type::to_char_type(c);
	pbump(1);
	return c;
}

/* === E N T R Y P O I N T ================================================= */

