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
#include "general.h"
#include <synfig/localization.h>
#include "guid.h"
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
file_system_(FileSystemNative::instance()),
temporary_filename_base_(generate_temporary_filename_base())
{ }

FileContainerTemporary::~FileContainerTemporary() { close(); }

std::string FileContainerTemporary::get_temporary_directory()
{
    const char *tmpdir;
    if ((tmpdir = getenv("TEMP")) == NULL)
    if ((tmpdir = getenv("TMP")) == NULL)
    if ((tmpdir = getenv("TMPDIR")) == NULL)
    	 tmpdir = "/tmp";
    return std::string(tmpdir) + ETL_DIRECTORY_SEPARATOR;
}

std::string FileContainerTemporary::generate_temporary_filename_base()
{
    return "synfig_" + GUID().get_string();
}

std::string FileContainerTemporary::generate_temporary_filename()
{
    return get_temporary_directory() + generate_temporary_filename_base();
}

bool FileContainerTemporary::create(const std::string &container_filename)
{
	bool res
		 = !is_opened()
		&& (container_filename.empty() || container_->create(container_filename))
		&& ((container_filename_ = fix_slashes(container_filename)).empty() || true)
		&& (is_opened_ = true);
	if (res && !container_filename_.empty() && !is_absolute_path(fix_slashes(container_filename_)))
		container_filename_ = absolute_path(fix_slashes(container_filename_));
	return res;
}

bool FileContainerTemporary::open(const std::string &container_filename)
{
	bool res
	     = !is_opened()
		&& container_->open(container_filename)
		&& ((container_filename_ = fix_slashes(container_filename)).empty() || true)
		&& (is_opened_ = true);
	if (res && !container_filename_.empty() && !is_absolute_path(fix_slashes(container_filename_)))
		container_filename_ = absolute_path(fix_slashes(container_filename_));
	return res;
}

bool FileContainerTemporary::open_from_history(const std::string &container_filename, FileContainerZip::file_size_t truncate_storage_size)
{
	bool res
	     = !is_opened()
		&& container_->open_from_history(container_filename, truncate_storage_size)
		&& ((container_filename_ = fix_slashes(container_filename)).empty() || true)
		&& (is_opened_ = true);
	if (res && !container_filename_.empty() && !is_absolute_path(fix_slashes(container_filename_)))
		container_filename_ = absolute_path(fix_slashes(container_filename_));
	return res;
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
	FileMap::const_iterator i = files_.find(fix_slashes(filename));
	return i == files_.end()
		 ? container_->is_file(filename)
		 : !i->second.is_removed && !i->second.is_directory;
}

bool FileContainerTemporary::is_directory(const std::string &filename)
{
	if (!is_opened()) return false;
	if (filename.empty()) return true;
	FileMap::const_iterator i = files_.find(fix_slashes(filename));
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
	info.name = fix_slashes(dirname);
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
		if (i->second.name_part_directory == fix_slashes(dirname))
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

		FileMap::iterator i = files_.find(fix_slashes(filename));
		if (i == files_.end())
		{
			FileInfo &info = files_[fix_slashes(filename)];
			info.name = fix_slashes(filename);
			info.is_directory = true;
			info.is_removed = true;
			info.split_name();
		}
		else
		{
			FileInfo &info = files_[fix_slashes(filename)];
			info.is_removed = true;
		}
	}
	else
	// remove file
	if (is_file(filename))
	{
		if (file_is_opened() && file_ == fix_slashes(filename))
			return false;

		FileMap::iterator i = files_.find(fix_slashes(filename));
		if (i == files_.end())
		{
			FileInfo &info = files_[fix_slashes(filename)];
			info.name = fix_slashes(filename);
			info.is_directory = false;
			info.is_removed = true;
			info.split_name();
		}
		else
		{
			FileInfo &info = files_[fix_slashes(filename)];
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
	FileMap::const_iterator i = files_.find(fix_slashes(filename));

	if (i == files_.end())
		file_read_stream_ = container_->get_read_stream(filename);
	else
	if (!i->second.is_removed && !i->second.is_directory && !i->second.tmp_filename.empty())
		file_read_stream_ = file_system_->get_read_stream(i->second.tmp_filename);

	if (!file_read_stream_) return false;
	file_ = fix_slashes(filename);
	return true;
}

bool FileContainerTemporary::file_open_write(const std::string &filename)
{
	if (!is_opened() || file_is_opened()) return false;
	if (!container_->file_check_name(filename)) return false;

	FileMap::iterator i = files_.find(fix_slashes(filename));
	std::string tmp_filename;

	FileInfo new_info;
	if (i == files_.end())
	{
		// create new file
		new_info.name = fix_slashes(filename);
		new_info.split_name();
		if (new_info.name_part_localname.empty()
		 || !is_directory(new_info.name_part_directory)) return false;
		new_info.tmp_filename = generate_temporary_filename();
		file_write_stream_ = file_system_->get_write_stream(new_info.tmp_filename);
		if (file_write_stream_) files_[new_info.name] = new_info;
	}
	else
	if (!i->second.is_removed && !i->second.is_directory)
	{
		tmp_filename = generate_temporary_filename();
		file_write_stream_ = file_system_->get_write_stream(tmp_filename);
	}

	if (!file_write_stream_) return false;
	file_ = fix_slashes(filename);
	file_tmp_name_ = tmp_filename;
	return true;
}

void FileContainerTemporary::file_close()
{
	if (file_write_stream_ && !file_tmp_name_.empty())
	{
		std::string prev_name = files_[file_].tmp_filename;
		files_[file_].tmp_filename = file_tmp_name_;
		file_tmp_name_.clear();
		file_system_->file_remove(prev_name);
	}

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
	return file_read_stream_->read_block(buffer, size);
}

size_t FileContainerTemporary::file_write(const void *buffer, size_t size)
{
	if (!file_is_opened_for_write()) return 0;
	return file_write_stream_->write_block(buffer, size);
}

bool FileContainerTemporary::save_changes(const std::string &filename, bool as_copy)
{
	if (file_is_opened()) return false;

	etl::handle< FileContainerZip > container;

	std::string fname_abs = fix_slashes(filename);
	if (!is_absolute_path(fname_abs)) fname_abs = absolute_path(fname_abs);

	bool save_at_place = filename.empty() || fname_abs == container_filename_;
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
	processed = true;
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
			container_filename_ = fname_abs;
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

		file_system_->file_remove( get_temporary_directory() + get_temporary_filename_base() );
		temporary_filename_base_ = generate_temporary_filename_base();
	}
}

bool FileContainerTemporary::save_temporary() const
{
	xmlpp::Document document;
	xmlpp::Element *root = document.create_root_node("temporary-container");
	root->add_child("container-filename")->set_child_text(container_filename_);
	xmlpp::Element *files = root->add_child("files");
	for(FileMap::const_iterator i = files_.begin(); i != files_.end(); i++)
	{
		xmlpp::Element *entry = files->add_child("entry");
		entry->add_child("name")->set_child_text(i->second.name);
		entry->add_child("tmp-basename")->set_child_text(basename(i->second.tmp_filename));
		entry->add_child("is-directory")->set_child_text(i->second.is_directory ? "true" : "false");
		entry->add_child("is-removed")->set_child_text(i->second.is_removed ? "true" : "false");
	}

	FileSystem::WriteStreamHandle stream =
		file_system_->get_write_stream(
			get_temporary_directory()
		  + get_temporary_filename_base());
	if (!stream) return false;
	stream = new ZWriteStream(stream);
	try
	{
		document.write_to_stream_formatted(*stream, "UTF-8");
	}
	catch(...)
	{
		synfig::error("FileContainerTemporary::save_temporary(): Caught unknown exception");
		return false;
	}
	stream.reset();

	return true;
}

std::string FileContainerTemporary::get_xml_node_text(xmlpp::Node *node)
{
	std::string s;
	if (node != NULL)
	{
		xmlpp::Element::NodeList list = node->get_children();
		for(xmlpp::Element::NodeList::iterator i = list.begin(); i != list.end(); i++)
			if (dynamic_cast<xmlpp::TextNode*>(*i))
				s += dynamic_cast<xmlpp::TextNode*>(*i)->get_content();
	}
	return s;
}

bool FileContainerTemporary::open_temporary(const std::string &filename_base)
{
	if (is_opened()) return false;

	FileSystem::ReadStreamHandle stream =
		file_system_->get_read_stream(
			get_temporary_directory()
		  + filename_base);
	if (!stream) return false;
	stream = new ZReadStream(stream);

	xmlpp::DomParser parser;
	parser.parse_stream(*stream);
	stream.reset();
	if (!parser) return false;

	xmlpp::Element *root = parser.get_document()->get_root_node();
	if (root->get_name() != "temporary-container") return false;

	xmlpp::Element::NodeList list = root->get_children();
	for(xmlpp::Element::NodeList::iterator i = list.begin(); i != list.end(); i++)
	{
		if ((*i)->get_name() == "container-filename")
			container_filename_ = get_xml_node_text(*i);
		if ((*i)->get_name() == "files")
		{
			xmlpp::Element::NodeList files_list = (*i)->get_children();
			for(xmlpp::Element::NodeList::iterator j = files_list.begin(); j != files_list.end(); j++)
			{
				if ((*j)->get_name() == "entry")
				{
					FileInfo info;
					xmlpp::Element::NodeList fields_list = (*j)->get_children();
					for(xmlpp::Element::NodeList::iterator k = fields_list.begin(); k != fields_list.end(); k++)
					{
						if ((*k)->get_name() == "name")
							info.name = get_xml_node_text(*k);
						if ((*k)->get_name() == "tmp-basename")
							info.tmp_filename = get_xml_node_text(*k);
						if ((*k)->get_name() == "is-directory")
							info.is_directory = get_xml_node_text(*k) == "true";
						if ((*k)->get_name() == "is-removed")
							info.is_removed = get_xml_node_text(*k) == "true";
					}
					if (!info.tmp_filename.empty())
						info.tmp_filename = get_temporary_directory() + info.tmp_filename;
					info.split_name();
					files_[info.name] = info;
				}
			}
		}
	}

	if (!container_filename_.empty() && !container_->open(container_filename_))
	{
		container_filename_.clear();
		files_.clear();
		return false;
	}

	temporary_filename_base_ = filename_base;
	is_opened_ = true;
	return true;
}

std::string
FileContainerTemporary::generate_indexed_temporary_filename(const FileSystem::Handle & /* fs */, const std::string &filename)
{
	String extension = filename_extension(filename);
	String sans_extension = filename_sans_extension(filename);
	for(int index = 1; index < 10000; ++index)
	{
		String indexed_filename = strprintf("%s_%04d%s", sans_extension.c_str(), index, extension.c_str());
		if (!FileSystemNative::instance()->is_exists(indexed_filename))
			return indexed_filename;
	}
	assert(false);
	return std::string();
}

/* === E N T R Y P O I N T ================================================= */


