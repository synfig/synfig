/* === S Y N F I G ========================================================= */
/*!	\file filecontainertemporary.cpp
**	\brief FileContainerTemporary
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
#include <cstdlib>
#include "filecontainertemporary.h"
#include "guid.h"

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void FileContainerTemporary::FileInfo::split_name()
{
	size_t posA = name.rfind('/');
	size_t posB = name.rfind('\\');
	size_t pos = posA == std::string::npos ? posB :
				 posB == std::string::npos ? posA :
				 posA > posB ? posA : posB;
	if (pos == std::string::npos || pos == 0)
	{
		name_part_directory.clear();
		name_part_localname = name;
	}
	else
	{
		name_part_directory = name.substr(0, pos);
		name_part_localname = name.substr(pos + 1);
	}
}

FileContainerTemporary::FileContainerTemporary():
is_opened_(false),
container_(new FileContainerZip()),
file_system_(new FileSystemNative())
{ }

FileContainerTemporary::~FileContainerTemporary() { close(); }

std::string FileContainerTemporary::generate_tmp_filename()
{
    const char *tmpdir;
    if ((tmpdir = getenv("TEMP")) == NULL)
    if ((tmpdir = getenv("TMP")) == NULL)
    if ((tmpdir = getenv("TMPDIR")) == NULL)
    	 tmpdir = "/tmp";
    return std::string(tmpdir) + ETL_DIRECTORY_SEPARATOR + "synfig_" + GUID().get_string();
}

bool FileContainerTemporary::create(const std::string &container_filename)
{
	return !is_opened()
		&& (container_filename.empty() || container_->create(container_filename))
		&& ((container_filename_ = container_filename).empty() || true)
		&& (is_opened_ = true);
}

bool FileContainerTemporary::open(const std::string &container_filename)
{
	return !is_opened()
		&& container_->open(container_filename)
		&& ((container_filename_ = container_filename).empty() || true)
		&& (is_opened_ = true);
}

void FileContainerTemporary::close()
{
	if (!is_opened()) return;
	file_close();
	discard_changes();
	container_->close();
	container_filename_.clear();
	is_opened_ = false;
}


bool FileContainerTemporary::is_opened()
{
	return is_opened_;
}

bool FileContainerTemporary::is_file(const std::string &filename)
{
	if (!is_opened()) return false;
	FileMap::const_iterator i = files_.find(filename);
	return i == files_.end()
		 ? container_->is_file(filename)
		 : !i->second.is_removed && !i->second.is_directory;
}

bool FileContainerTemporary::is_directory(const std::string &filename)
{
	if (!is_opened()) return false;
	FileMap::const_iterator i = files_.find(filename);
	return i == files_.end()
		 ? container_->is_directory(filename)
		 : !i->second.is_removed && i->second.is_directory;
}

bool FileContainerTemporary::directory_create(const std::string &dirname)
{
	if (!is_opened()) return false;
	if (is_file(dirname)) return false;
	if (is_directory(dirname)) return true;
	if (!container_->directory_check_name(dirname)) return false;

	FileInfo info;
	info.name = dirname;
	info.split_name();
	info.is_directory = true;
	if (info.name_part_localname.empty()
	 || !is_directory(info.name_part_directory)) return false;

	files_[info.name] = info;
	return true;
}

bool FileContainerTemporary::directory_scan(const std::string &dirname, std::list< std::string > &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;

	container_->directory_scan(dirname, out_files);

	for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
	{
		if (i->second.name_part_directory == dirname)
		{
			if (i->second.is_removed)
			{
				for(std::list< std::string >::iterator j = out_files.begin(); j != out_files.end();)
					if (*j == i->second.name_part_localname)
						j = out_files.erase(j); else j++;
			}
			else
			{
				bool found = false;
				for(std::list< std::string >::iterator j = out_files.begin(); j != out_files.end();)
					if (*j == i->second.name_part_localname)
						{ found = true; break; }
				if (!found)
					out_files.push_back(i->second.name_part_localname);
			}
		}
	}

	return true;
}

bool FileContainerTemporary::file_remove(const std::string &filename)
{
	// remove directory
	if (is_directory(filename))
	{
		std::list< std::string > files;
		directory_scan(filename, files);
		if (!files.empty()) return false;

		FileMap::iterator i = files_.find(filename);
		if (i == files_.end())
		{
			FileInfo &info = files_[filename];
			info.name = filename;
			info.is_directory = true;
			info.is_removed = true;
			info.split_name();
		}
		else
		{
			FileInfo &info = files_[filename];
			info.is_removed = true;
		}
	}
	else
	// remove file
	if (is_file(filename))
	{
		if (file_is_opened() && file_ == filename)
			return false;

		FileMap::iterator i = files_.find(filename);
		if (i == files_.end())
		{
			FileInfo &info = files_[filename];
			info.name = filename;
			info.is_directory = false;
			info.is_removed = true;
			info.split_name();
		}
		else
		{
			FileInfo &info = files_[filename];
			info.is_removed = true;
			if (!info.tmp_filename.empty())
			{
				file_system_->file_remove(info.tmp_filename);
				info.tmp_filename.clear();
			}
		}
	}
	return true;
}

bool FileContainerTemporary::file_open_read(const std::string &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	FileMap::const_iterator i = files_.find(filename);

	if (i == files_.end())
		file_read_stream_ = container_->get_read_stream(filename);
	else
	if (!i->second.is_removed && !i->second.is_directory && !i->second.tmp_filename.empty())
		file_read_stream_ = file_system_->get_read_stream(i->second.tmp_filename);

	if (!file_read_stream_) return false;
	file_ = filename;
	return true;
}

bool FileContainerTemporary::file_open_write(const std::string &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	if (!container_->file_check_name(filename)) return false;

	FileMap::iterator i = files_.find(filename);

	FileInfo new_info;
	if (i == files_.end())
	{
		// create new file
		new_info.name = filename;
		new_info.split_name();
		if (new_info.name_part_localname.empty()
		 || !is_directory(new_info.name_part_directory)) return false;
		new_info.tmp_filename = generate_tmp_filename();
		file_write_stream_ = file_system_->get_write_stream(new_info.tmp_filename);
		if (file_write_stream_) files_[new_info.name] = new_info;
	}
	else
	if (!i->second.is_removed && !i->second.is_directory && !i->second.tmp_filename.empty())
	{
		file_write_stream_ = file_system_->get_write_stream(i->second.tmp_filename);
	}

	if (!file_write_stream_) return false;
	file_ = filename;
	return true;
}

void FileContainerTemporary::file_close()
{
	file_read_stream_.reset();
	file_write_stream_.reset();

	// call base-class method to invalidate streams
	FileContainer::file_close();
}

bool FileContainerTemporary::file_is_opened_for_read()
{
	return is_opened() && file_read_stream_;
}

bool FileContainerTemporary::file_is_opened_for_write()
{
	return is_opened() && file_write_stream_;
}

size_t FileContainerTemporary::file_read(void *buffer, size_t size)
{
	if (!file_is_opened_for_read()) return 0;
	return file_read_stream_->read(buffer, size);
}

size_t FileContainerTemporary::file_write(const void *buffer, size_t size)
{
	if (!file_is_opened_for_write()) return 0;
	return file_write_stream_->write(buffer, size);
}

bool FileContainerTemporary::save_changes(const std::string &filename, bool as_copy)
{
	if (file_is_opened()) return false;

	etl::handle< FileContainerZip > container;

	bool save_at_place = filename.empty() || filename == container_filename_;
 	if (save_at_place) as_copy = false;


	if (save_at_place)
	{
		if (!container_->is_opened()) return false;
		container = container_;
	}
	else
	{
		if (container_->is_opened())
		{
			{ // copy container
				ReadStreamHandle read_steram = container_->get_read_stream_whole_container();
				if (read_steram.empty()) return false;
				WriteStreamHandle write_stream = file_system_->get_write_stream(filename);
				if (write_stream.empty()) return false;
				if (!write_stream->write_whole_stream(read_steram)) return false;
			}

			// open container
			container = new FileContainerZip();
			if (!container->open(filename)) return false;
		}
		else
		{
			// create container
			container = new FileContainerZip();
			if (!container->create(filename)) return false;
		}
	}

	FileMap files = files_;

	// remove files
	bool processed = true;
	while(processed)
	{
		processed = false;
		for(FileMap::iterator i = files.begin(); i != files.end(); i++)
		{
			if (i->second.is_removed && container->file_remove(i->second.name))
			{
				processed = true;
				files.erase(i);
				break;
			}
		}
	}

	// create directories
	processed = false;
	while(processed)
	{
		processed = false;
		for(FileMap::iterator i = files.begin(); i != files.end(); i++)
		{
			if (!i->second.is_removed
			 && i->second.is_directory
			 && container->directory_create(i->second.name))
			{
				processed = true;
				files.erase(i);
				break;
			}
		}
	}

	// create files
	for(FileMap::iterator i = files.begin(); i != files.end();)
	{
		if (!i->second.is_removed
		 && !i->second.is_directory
		 && !i->second.tmp_filename.empty()
		 && copy(file_system_, i->second.tmp_filename, container, i->second.name))
		{
			file_system_->file_remove(i->second.tmp_filename);
			processed = true;
			files.erase(i++);
		}
		else i++;
	}

	// try to save container
	if (container->save())
	{
		// update internal state
		if (save_at_place)
		{
			files_ = files;
		}
		else
		if (!as_copy && files.empty())
		{
			container_ = container;
			files_ = files;
		}
		return files.empty();
	}

	return false;
}

void FileContainerTemporary::discard_changes()
{
	if (is_opened())
	{
		file_close();

		// remove temporary files
		for(FileMap::iterator i = files_.begin(); i != files_.end(); i++)
		{
			if (!i->second.is_removed
			 && !i->second.is_directory
			 && !i->second.tmp_filename.empty())
			{
				file_system_->file_remove(i->second.tmp_filename);
			}
		}

		// update internal state
		files_.clear();
	}
}

/* === E N T R Y P O I N T ================================================= */


