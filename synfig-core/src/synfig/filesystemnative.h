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
		class ReadStream : public FileSystem::ReadStream
		{
		protected:
			friend FileSystemNative;
			FILE *file_;
			ReadStream(Handle file_system, FILE *file);
		public:
			virtual ~ReadStream();
			virtual size_t read(void *buffer, size_t size);
		};

		class WriteStream : public FileSystem::WriteStream
		{
		protected:
			friend FileSystemNative;
			FILE *file_;
			WriteStream(Handle file_system, FILE *file_);
		public:
			virtual ~WriteStream();
			virtual size_t write(const void *buffer, size_t size);
		};

		FileSystemNative();
		virtual ~FileSystemNative();

		virtual bool file_remove(const std::string &filename);
		virtual ReadStreamHandle get_read_stream(const std::string &filename);
		virtual WriteStreamHandle get_write_stream(const std::string &filename);
	};

}

/* === E N D =============================================================== */

#endif
