/* === S Y N F I G ========================================================= */
/*!	\file canvasfilenaming.h
**	\brief CanvasFileNaming Header
**
**	$Id$
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

#ifndef __SYNFIG_CANVASFILENAMING_H
#define __SYNFIG_CANVASFILENAMING_H

/* === H E A D E R S ======================================================= */


#include "canvas.h"
#include "filesystem.h"
#include "filecontainerzip.h"
#include "string.h"
#include "synfig_export.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig {

class CanvasFileNaming
{
public:
	SYNFIG_EXPORT static const String container_prefix;
	static const String container_directory_separator;
	static const String container_canvas_filename;

	static String filename_base(const String &filename);
	static String filename_extension_lower(const String &filename);
	static String append_directory_separator(const String &path);

	static String content_folder_by_extension(const String &ext);
	static String content_folder_by_filename(const String &filename)
		{ return content_folder_by_extension(filename_extension_lower(filename)); }

	static bool is_container_extension(const String &ext);
	static bool is_container_filename(const String &filename)
		{ return is_container_extension(filename_extension_lower(filename)); }

	static String make_short_filename(const String &canvas_filename, const String &filename);
	static String make_full_filename(const String &canvas_filename, const String &filename);
	static String make_canvas_independent_filename(const String &canvas_filename, const String &filename);
	static String make_local_filename(const String &canvas_filename, const String &filename);

	static FileSystem::Handle make_filesystem_container(const String &filename, FileContainerZip::file_size_t truncate_storage_size = 0, bool create_new = false);
	static FileSystem::Handle make_filesystem(const FileSystem::Handle &filesystem_container);
	static FileSystem::Handle make_filesystem(const String &filename, FileContainerZip::file_size_t truncate_storage_size = 0, bool create_new = false);

	static String project_file(const FileSystem::Handle &canvas_filesystem);
	static String project_file(const String &filename);

	static bool is_embeded(const String &filename);
	static bool is_embeded(const String &canvas_filename, const String &filename);
	static bool can_embed(const String &filename);
	static String generate_container_filename(const FileSystem::Handle &canvas_filesystem, const String &filename);
	static String container_canvas_full_filename();
}; // END of class CanvasFileNaming

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
