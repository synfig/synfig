/* === S Y N F I G ========================================================= */
/*!	\file canvasfilenaming.cpp
**	\brief CanvasFilenaming Implementation
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "canvasfilenaming.h"

#include "filecontainerzip.h"
#include "filesystemnative.h"
#include "filesystemgroup.h"
#include "filesystemtemporary.h"
#include "importer.h"

#endif

using namespace synfig;
using namespace etl;
using namespace std;

/* === M A C R O S ========================================================= */

SYNFIG_EXPORT const String CanvasFileNaming::container_prefix("#");
const String CanvasFileNaming::container_directory_separator("/");
const String CanvasFileNaming::container_canvas_filename("project.sifz");


String
CanvasFileNaming::filename_base(const String &filename)
{
	String base = filename;
	size_t pos;
	while((pos = base.find(container_prefix)) != String::npos)
		base.replace(pos, container_prefix.size(), container_directory_separator);
	return etl::basename(base);
}

String
CanvasFileNaming::filename_extension_lower(const String &filename)
{
	String ext = etl::filename_extension(filename);
	if (!ext.empty()) ext = ext.substr(1); // skip initial '.'
	std::transform(ext.begin(), ext.end(), ext.begin(), &::tolower);
	return ext;
}

String
CanvasFileNaming::append_directory_separator(const String &path)
{
	return path.empty() ? String() : path + container_directory_separator;
}

String
CanvasFileNaming::content_folder_by_extension(const String &ext)
{
	if (Importer::book().count(ext))
		return "images";
	if (ext == "pgo")
		return "animations";
	return String();
}

bool
CanvasFileNaming::is_container_extension(const String &ext)
{
	return ext == "sfg";
}

String
CanvasFileNaming::make_short_filename(const String &canvas_filename, const String &filename)
{
	if (filename.empty()) return String();
	String clean_filename = etl::cleanup_path(filename);
	if (filename_base(clean_filename).empty())
		clean_filename = etl::dirname(clean_filename);
	if (filename_base(clean_filename).empty()) return String();

	// any file inside container accessible without folder:
	//     "#filename.png" equals to "#images/filename.png"
	//     "#filename.pgo" equals to "#animations/filename.pgo"
	if (clean_filename.substr(0, container_prefix.size()) == container_prefix)
		return clean_filename.size() > container_prefix.size()
			 ? container_prefix + filename_base(clean_filename.substr(container_prefix.size()))
			 : String();

	if (!etl::is_absolute_path(canvas_filename))
		return clean_filename;

	String canvas_absolute_filename = etl::absolute_path(canvas_filename);
	String canvas_path = etl::dirname(canvas_absolute_filename);
	String canvas_basename = filename_base(canvas_absolute_filename);
	String absolute_filename = etl::absolute_path(canvas_path, clean_filename);
	String relative_filename = etl::relative_path(canvas_path, absolute_filename);

	// convert "mycanvas.sfg#images/filename.png" to "#filename.png"
	String prefix = canvas_basename + container_prefix;
	if (relative_filename.substr(0, prefix.size()) == prefix)
		return relative_filename.size() > prefix.size()
			 ? container_prefix + filename_base(relative_filename.substr(prefix.size()))
			 : String();

	if (!is_container_filename(canvas_basename))
	{
		// for non-containers convert "mycanvas.images/filename.png" to "#filename.png"
		String content_folder = content_folder_by_filename(clean_filename);
		String prefix = canvas_basename + "." + content_folder + container_directory_separator;
		if ( !content_folder.empty()
		  && relative_filename.substr(0, prefix.size()) == prefix)
			return relative_filename.size() > prefix.size()
				 ? container_prefix + filename_base(relative_filename.substr(prefix.size()))
				 : String();
	}

	return relative_filename;
}

String
CanvasFileNaming::make_full_filename(const String &canvas_filename, const String &filename)
{
	String short_filename = make_short_filename(canvas_filename, filename);
	if (short_filename.empty()) return String();
	if (filename_base(short_filename).empty()) return String();

	if (short_filename.substr(0, container_prefix.size()) == container_prefix)
	{
		String content_folder = content_folder_by_filename(filename);
		String content_folder_prefix = !content_folder.empty()
				                     ? content_folder + container_directory_separator
				                     : String();
		return short_filename.size() > container_prefix.size()
			 ? container_prefix + content_folder_prefix + filename_base(short_filename.substr(container_prefix.size()))
			 : String();
	}

	if (!etl::is_absolute_path(canvas_filename))
		return short_filename;

	String canvas_absolute_filename = etl::absolute_path(canvas_filename);
	String canvas_path = etl::dirname(canvas_absolute_filename);
	String absolute_filename = etl::absolute_path(canvas_path, short_filename);

	return absolute_filename;
}

String
CanvasFileNaming::make_canvas_independent_filename(const String &canvas_filename, const String &filename)
{
	String full_filename = make_full_filename(canvas_filename, filename);
	if (full_filename.empty()) return String();
	if (filename_base(full_filename).empty()) return String();

	if (full_filename.substr(0, container_prefix.size()) != container_prefix)
		return full_filename;

	if (!etl::is_absolute_path(canvas_filename))
		return full_filename;

	String canvas_absolute_filename = etl::absolute_path(canvas_filename);
	if (is_container_filename(canvas_absolute_filename))
		return canvas_absolute_filename + full_filename;

	String content_folder = content_folder_by_filename(full_filename);
	if (content_folder.empty())
		return canvas_absolute_filename + full_filename;

	String canvas_path = etl::dirname(canvas_absolute_filename);
	String canvas_basename = filename_base(canvas_absolute_filename);
	return canvas_path
		 + ETL_DIRECTORY_SEPARATOR
		 + canvas_basename
		 + "."
		 + content_folder
		 + ETL_DIRECTORY_SEPARATOR
		 + filename_base(full_filename.substr(container_prefix.size()));
}

String
CanvasFileNaming::make_local_filename(const String &canvas_filename, const String &filename)
{
	if (filename.empty()) return String();
	String base = filename_base(filename);
	if (base.empty())
		base = filename_base(etl::dirname(filename));
	if (base.substr(0, container_prefix.size()) == container_prefix)
		base = base.substr(container_prefix.size());

	if (!etl::is_absolute_path(canvas_filename))
		return base;

	String canvas_absolute_filename = etl::absolute_path(canvas_filename);
	String canvas_path = etl::dirname(canvas_absolute_filename);
	return canvas_path + ETL_DIRECTORY_SEPARATOR + base;
}

FileSystem::Handle
CanvasFileNaming::make_filesystem_container(const String &filename, FileContainerZip::file_size_t truncate_storage_size, bool create_new)
{
	if (is_container_filename(filename))
	{
		FileContainerZip::Handle container(new FileContainerZip());
		if ( create_new
		   ? container->create(filename)
		   : container->open_from_history(filename, truncate_storage_size) )
			return container;
	}
	else
	{
		String dir = etl::dirname(filename);
		String base = filename_base(filename);
		String name = etl::filename_sans_extension(base);
		String ext = filename_extension_lower(base);
		String prefix = dir + ETL_DIRECTORY_SEPARATOR + name + ".";

		FileSystemGroup::Handle group(new FileSystemGroup());
		group->register_system("images", FileSystemNative::instance(), prefix + "images");
		group->register_system("animations", FileSystemNative::instance(), prefix + "animations");
		group->register_system(ext == "sif" ? "project.sif" : container_canvas_filename, FileSystemNative::instance(), filename);
		return group;
	}

	return FileSystem::Handle();
}

FileSystem::Handle
CanvasFileNaming::make_filesystem(const FileSystem::Handle &filesystem_container)
{
	if (!filesystem_container) return FileSystem::Handle();
	FileSystemGroup::Handle group(new FileSystemGroup(FileSystemNative::instance()));
	group->register_system(container_prefix, filesystem_container, String(), true);
	return group;
}

FileSystem::Handle
CanvasFileNaming::make_filesystem(const String &filename, FileContainerZip::file_size_t truncate_storage_size, bool create_new)
{
	return make_filesystem(make_filesystem_container(filename, truncate_storage_size, create_new));
}

String
CanvasFileNaming::project_file(const FileSystem::Handle &canvas_filesystem)
{
	if (!canvas_filesystem)
		return String();
	if (canvas_filesystem->is_file(container_canvas_full_filename()))
		return container_canvas_full_filename();
	if (canvas_filesystem->is_file(container_prefix + "project.sif"))
		return container_prefix + "project.sif";
	return String();
}

String
CanvasFileNaming::project_file(const String &filename) {
	return filename_extension_lower(filename) == "sif"
		 ? container_prefix + "project.sif"
		 : container_canvas_full_filename();
}

bool
CanvasFileNaming::is_embeded(const String &filename)
{
	String clean_filename = etl::cleanup_path(filename);
	return clean_filename.size() > container_prefix.size()
		&& clean_filename.substr(0, container_prefix.size()) == container_prefix;
}

bool
CanvasFileNaming::is_embeded(const String &canvas_filename, const String &filename)
{
	if (canvas_filename.empty())
		return is_embeded(filename);

	String short_filename = make_short_filename(canvas_filename, filename);
	return short_filename.size() > container_prefix.size()
		&& short_filename.substr(0, container_prefix.size()) == container_prefix;
}

bool
CanvasFileNaming::can_embed(const String &filename)
{
	return !content_folder_by_filename(filename).empty();
}

String
CanvasFileNaming::generate_container_filename(const FileSystem::Handle &canvas_filesystem, const String &filename)
{
	String base = filename_base(filename);
	if (base.empty())
		base = filename_base(etl::dirname(filename));
	if (base.substr(0, container_prefix.size()) == container_prefix)
		base = base.substr(container_prefix.size());
	String ext = filename_extension_lower(base);
	base = etl::filename_sans_extension(base);
	String content_folder = content_folder_by_extension(ext);
	String content_prefix = content_folder.empty() ? String() : content_folder + container_directory_separator;

	for(int i = 1; i < 10000; ++i)
	{
		String short_filename = i <= 1
				              ? container_prefix + base + "." + ext
				              : etl::strprintf("%s%s_%d.%s", container_prefix.c_str(), base.c_str(), i, ext.c_str());
		String full_filename = i <= 1
				             ? container_prefix + base + "." + ext
				             : etl::strprintf("%s%s%s_%d.%s", container_prefix.c_str(), content_prefix.c_str(), base.c_str(), i, ext.c_str());
		if (!canvas_filesystem->is_exists(full_filename))
			return short_filename;
	}

	assert(false);
	return String();
}

String
CanvasFileNaming::container_canvas_full_filename()
{
	return container_prefix + container_canvas_filename;
}
