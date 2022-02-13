/* === S Y N F I G ========================================================= */
/*!	\file filesystemtemporary.cpp
**	\brief FileSystemTemporary Implementation
**
**	\legal
**	......... ... 2016 Ivan Mahonin
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <libxml++/libxml++.h>

#include <ETL/stringf>

#include "general.h"
#include "localization.h"

#include "filesystemtemporary.h"

#include "guid.h"
#include "zstreambuf.h"

#endif

/* === U S I N G =========================================================== */

using namespace etl;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

FileSystemTemporary::FileSystemTemporary(const String &tag, const String &temporary_directory, const FileSystem::Handle &sub_file_system):
	file_system(FileSystemNative::instance()),
	tag(tag),
	temporary_directory(temporary_directory.empty() ? get_system_temporary_directory() : temporary_directory),
	temporary_filename_base(generate_temporary_filename_base(tag)),
	keep_files_when_destroyed(false),
	autosave(true)
{
	set_sub_file_system(sub_file_system);
}

FileSystemTemporary::~FileSystemTemporary()
{
	if (!keep_files_when_destroyed) {
		discard_changes();
	}
}

String
FileSystemTemporary::get_system_temporary_directory()
{
    const char *tmpdir;
    if ((tmpdir = getenv("TEMP")) == NULL)
    if ((tmpdir = getenv("TMP")) == NULL)
    if ((tmpdir = getenv("TMPDIR")) == NULL)
    	 tmpdir = "/tmp";
    return String(tmpdir);
}

String
FileSystemTemporary::generate_temporary_filename_base(const String &tag)
{
    return "synfig_" + tag + "_" + GUID().get_string();
}

bool
FileSystemTemporary::scan_temporary_directory(const String &tag, FileList &out_files, const String &dirname)
{
	String tmpdir = dirname.empty() ? get_system_temporary_directory() : dirname;

	FileList files;
	if (!FileSystemNative::instance()->directory_scan(dirname, files))
		return false;

	String prefix = "synfig_" + tag + "_";
	for(FileList::const_iterator i = files.begin(); i != files.end(); ++i)
		if (i->substr(0, prefix.size()) == prefix)
			if (FileSystemNative::instance()->is_file(tmpdir + ETL_DIRECTORY_SEPARATOR + *i))
				out_files.push_back(*i);
	return true;
}

String
FileSystemTemporary::generate_system_temporary_filename(const String &tag)
{
    return get_system_temporary_directory() + ETL_DIRECTORY_SEPARATOR + generate_temporary_filename_base(tag);
}

bool
FileSystemTemporary::create_temporary_directory() const
{
	return file_system->directory_create_recursive(get_temporary_directory());
}

bool
FileSystemTemporary::is_file(const String &filename)
{
	FileMap::const_iterator i = files.find(fix_slashes(filename));
	if (i != files.end())
		return !i->second.is_removed && !i->second.is_directory;
	return get_sub_file_system() && get_sub_file_system()->is_file(filename);
}

bool
FileSystemTemporary::is_directory(const String &filename)
{
	if (filename.empty()) return true;
	FileMap::const_iterator i = files.find(fix_slashes(filename));
	if (i != files.end())
		return !i->second.is_removed && i->second.is_directory;
	return get_sub_file_system() && get_sub_file_system()->is_directory(filename);
}

bool
FileSystemTemporary::directory_create(const String &dirname)
{
	if (is_file(dirname)) return false;
	if (is_directory(dirname)) return true;

	FileInfo info;
	info.name = fix_slashes(dirname);
	info.is_directory = true;
	files[info.name] = info;
	autosave_temporary();
	return true;
}

bool
FileSystemTemporary::directory_scan(const String &dirname, FileList &out_files)
{
	out_files.clear();
	if (!is_directory(dirname)) return false;

	String clean_dirname = fix_slashes(dirname);
	std::set<String> files_set;

	if (get_sub_file_system())
	{
		FileList list;
		if (!get_sub_file_system()->directory_scan(clean_dirname, list))
			return false;
		for(FileList::const_iterator i = list.begin(); i != list.end(); ++i)
			files_set.insert(*i);
	}

	for(FileMap::iterator i = files.begin(); i != files.end(); i++)
		if (etl::dirname(i->second.name) == clean_dirname)
		{
			if (i->second.is_removed)
				files_set.erase(etl::basename(i->second.name));
			else
				files_set.insert(etl::basename(i->second.name));
		}

	for(std::set<String>::const_iterator i = files_set.begin(); i != files_set.end(); ++i)
		out_files.push_back(*i);
	return true;
}

bool
FileSystemTemporary::file_remove(const String &filename)
{
	// remove directory
	if (is_directory(filename))
	{
		// directory should be empty
		// NB: This code can check temporary files only,
		//     but directory may contain other not tracked real files.
		String prefix = fix_slashes(filename + "/");
		for(FileMap::iterator i = files.begin(); i != files.end(); i++)
			if ( !i->second.is_removed
			  && i->second.name.substr(0, prefix.size()) == prefix )
				return false;

		FileMap::iterator i = files.find(fix_slashes(filename));
		if (i == files.end())
		{
			FileInfo &info = files[fix_slashes(filename)];
			info.name = fix_slashes(filename);
			info.is_directory = true;
			info.is_removed = true;
			autosave_temporary();
		}
		else
		{
			FileInfo &info = i->second;
			info.is_removed = true;
			autosave_temporary();
		}
	}
	else
	// remove file
	if (is_file(filename))
	{
		FileMap::iterator i = files.find(fix_slashes(filename));
		if (i == files.end())
		{
			FileInfo &info = files[fix_slashes(filename)];
			info.name = fix_slashes(filename);
			info.is_directory = false;
			info.is_removed = true;
			autosave_temporary();
		}
		else
		{
			FileInfo &info = i->second;
			info.is_removed = true;
			if (!info.tmp_filename.empty())
			{
				file_system->file_remove(info.tmp_filename);
				info.tmp_filename.clear();
			}
			autosave_temporary();
		}
	}

	return true;
}

FileSystem::ReadStream::Handle
FileSystemTemporary::get_read_stream(const String &filename)
{
	FileMap::const_iterator i = files.find(fix_slashes(filename));
	if (i != files.end())
	{
		if (!i->second.is_removed && !i->second.is_directory && !i->second.tmp_filename.empty())
			return file_system->get_read_stream(i->second.tmp_filename);
	}
	else
	{
		if (get_sub_file_system())
			return get_sub_file_system()->get_read_stream(filename);
	}
	return FileSystem::ReadStream::Handle();
}

FileSystem::WriteStream::Handle
FileSystemTemporary::get_write_stream(const String &filename)
{
	FileSystem::WriteStream::Handle stream;

	FileMap::iterator i = files.find(fix_slashes(filename));
	if (i == files.end())
	{
		// create new file
		create_temporary_directory();
		FileInfo new_info;
		new_info.name = fix_slashes(filename);
		new_info.tmp_filename = get_temporary_directory()
				              + ETL_DIRECTORY_SEPARATOR
							  + generate_temporary_filename_base(tag + ".file");
		stream = file_system->get_write_stream(new_info.tmp_filename);
		if (stream)
		{
			files[new_info.name] = new_info;
			autosave_temporary();
		}
	}
	else
	if (!i->second.is_directory || i->second.is_removed)
	{
		create_temporary_directory();
		String tmp_filename = i->second.tmp_filename.empty()
				            ? get_temporary_directory()
							  + ETL_DIRECTORY_SEPARATOR
							  + generate_temporary_filename_base(tag + ".file")
				            : i->second.tmp_filename;
		stream = file_system->get_write_stream(tmp_filename);
		if (stream)
		{
			i->second.tmp_filename = tmp_filename;
			i->second.is_directory = false;
			i->second.is_removed = false;
			autosave_temporary();
		}
	}

	return stream;
}

String
FileSystemTemporary::get_real_uri(const String &filename)
{
	FileMap::const_iterator i = files.find(fix_slashes(filename));
	if (i != files.end())
	{
		if (!i->second.tmp_filename.empty())
			return file_system->get_real_uri(i->second.tmp_filename);
	}
	else
	{
		if (get_sub_file_system())
			return get_sub_file_system()->get_real_uri(filename);
	}
	return String();
}

bool
FileSystemTemporary::save_changes(
	const FileSystemNative::Handle &file_system,
	const FileSystem::Handle &target_file_system,
	FileMap &files,
	bool remove_files)
{
	assert(file_system);
	assert(target_file_system);

	// remove files
	bool processed = true;
	while(processed)
	{
		processed = false;
		for(FileMap::iterator i = files.begin(); i != files.end(); i++)
		{
			bool to_remove = i->second.is_directory
					       ? target_file_system->is_file(i->second.name)
					       : target_file_system->is_directory(i->second.name);
			to_remove = to_remove || i->second.is_removed;
			if (to_remove && target_file_system->file_remove(i->second.name))
			{
				processed = true;
				if (i->second.is_removed) files.erase(i);
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
			 && target_file_system->directory_create(i->second.name))
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
		 && copy(file_system, i->second.tmp_filename, target_file_system, i->second.name))
		{
			if (remove_files)
				file_system->file_remove(i->second.tmp_filename);
				
			files.erase(i++);
		}
		else i++;
	}

	return files.empty();
}

bool
FileSystemTemporary::save_changes_copy(const FileSystem::Handle &sub_file_system) const
{
	assert(sub_file_system);
	assert(sub_file_system != get_sub_file_system());
	FileMap files_copy = files;
	return save_changes(file_system, sub_file_system, files_copy, false);
}

bool
FileSystemTemporary::save_changes(const FileSystem::Handle &sub_file_system, bool as_copy)
{
	if (as_copy)
		return save_changes_copy(sub_file_system);
	set_sub_file_system(sub_file_system);
	return save_changes();
}

bool
FileSystemTemporary::save_changes()
{
	assert(get_sub_file_system());
	bool result = save_changes(this->file_system, get_sub_file_system(), files, true);
	autosave_temporary();
	return result;
}

void
FileSystemTemporary::discard_changes()
{
	// remove temporary files
	for(FileMap::iterator i = files.begin(); i != files.end(); i++)
	{
		if (!i->second.is_removed
		 && !i->second.is_directory
		 && !i->second.tmp_filename.empty())
		{
			file_system->file_remove(i->second.tmp_filename);
		}
	}

	// update internal state
	files.clear();
	meta.clear();

	// remove file with description
	assert(empty());
	save_temporary();
}

void
FileSystemTemporary::reset_temporary_filename_base(const String &tag, const String &temporary_directory)
{
	// remove previous file
	assert(empty());
	save_temporary();
	this->tag = tag;
	this->temporary_directory = temporary_directory;
	temporary_filename_base = generate_temporary_filename_base(this->tag);
}

String
FileSystemTemporary::get_meta(const String &key) const
{
	std::map<String, String>::const_iterator i = meta.find(key);
	return i == meta.end() ? String() : i->second;
}

void
FileSystemTemporary::set_meta(const String &key, const String &value)
{
	meta[key] = value;
	autosave_temporary();
}

void
FileSystemTemporary::clear_meta()
{
	meta.clear();
	autosave_temporary();
}

const std::map<String, String>&
FileSystemTemporary::get_metadata() const
	{ return meta; }

void
FileSystemTemporary::set_metadata(const std::map<String, String> &data)
{
	meta = data;
	autosave_temporary();
}

bool
FileSystemTemporary::autosave_temporary() const
{
	return !autosave || save_temporary();
}

bool
FileSystemTemporary::save_temporary() const
{
	if (empty())
	{
		file_system->file_remove(get_temporary_directory() + ETL_DIRECTORY_SEPARATOR + get_temporary_filename_base());
		return true;
	}

	xmlpp::Document document;
	xmlpp::Element *root = document.create_root_node("temporary-file-system");

	xmlpp::Element *meta_node = root->add_child("meta");
	for(std::map<String, String>::const_iterator i = meta.begin(); i != meta.end(); i++)
	{
		xmlpp::Element *entry = meta_node->add_child("entry");
		entry->add_child("key")->set_child_text(i->first);
		entry->add_child("value")->set_child_text(i->second);
	}

	xmlpp::Element *files_node = root->add_child("files");
	for(FileMap::const_iterator i = files.begin(); i != files.end(); i++)
	{
		xmlpp::Element *entry = files_node->add_child("entry");
		entry->add_child("name")->set_child_text(i->second.name);
		entry->add_child("tmp-basename")->set_child_text(basename(i->second.tmp_filename));
		entry->add_child("is-directory")->set_child_text(i->second.is_directory ? "true" : "false");
		entry->add_child("is-removed")->set_child_text(i->second.is_removed ? "true" : "false");
	}

	create_temporary_directory();
	FileSystem::WriteStream::Handle stream =
		file_system->get_write_stream(
			get_temporary_directory()
		  + ETL_DIRECTORY_SEPARATOR
		  + get_temporary_filename_base() );
	if (!stream) return false;

	stream = new ZWriteStream(stream);
	try
	{
		document.write_to_stream_formatted(*stream, "UTF-8");
	}
	catch(...)
	{
		synfig::error("FileSystemTemporary::save_temporary(): Caught unknown exception");
		return false;
	}
	stream.reset();

	return true;
}

String
FileSystemTemporary::get_xml_node_text(xmlpp::Node *node)
{
	String s;
	if (node != NULL)
	{
		xmlpp::Element::NodeList list = node->get_children();
		for(xmlpp::Element::NodeList::iterator i = list.begin(); i != list.end(); i++)
			if (dynamic_cast<xmlpp::TextNode*>(*i))
				s += dynamic_cast<xmlpp::TextNode*>(*i)->get_content();
	}
	return s;
}

bool
FileSystemTemporary::open_temporary(const String &filename)
{
	assert(empty());
	discard_changes();

	String tag;
	String temporary_directory = etl::dirname(filename);
	String temporary_filename_base = etl::basename(filename);

	size_t tag_begin = temporary_filename_base.find_first_of('_');
	size_t tag_end   = temporary_filename_base.find_last_of('_');
	if (tag_begin != String::npos && tag_end != String::npos && tag_end - tag_begin > 1)
		tag = temporary_filename_base.substr(tag_begin + 1, tag_end - tag_begin - 1);

	FileSystem::ReadStream::Handle stream = file_system->get_read_stream(filename);
	if (!stream) return false;
	stream = new ZReadStream(stream, zstreambuf::compression::gzip);

	xmlpp::DomParser parser;
	parser.parse_stream(*stream);
	stream.reset();
	if (!parser) return false;

	xmlpp::Element *root = parser.get_document()->get_root_node();
	if (root->get_name() != "temporary-file-system") return false;

	xmlpp::Element::NodeList list = root->get_children();
	for(xmlpp::Element::NodeList::iterator i = list.begin(); i != list.end(); i++)
	{
		if ((*i)->get_name() == "meta")
		{
			xmlpp::Element::NodeList meta_list = (*i)->get_children();
			for(xmlpp::Element::NodeList::iterator j = meta_list.begin(); j != meta_list.end(); j++)
			{
				if ((*j)->get_name() == "entry")
				{
					String key, value;
					xmlpp::Element::NodeList fields_list = (*j)->get_children();
					for(xmlpp::Element::NodeList::iterator k = fields_list.begin(); k != fields_list.end(); k++)
					{
						if ((*k)->get_name() == "key")
							key = get_xml_node_text(*k);
						if ((*k)->get_name() == "value")
							value = get_xml_node_text(*k);
					}
					meta[key] = value;
				}
			}
		}

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
							info.name = fix_slashes(get_xml_node_text(*k));
						if ((*k)->get_name() == "tmp-basename")
							info.tmp_filename = get_xml_node_text(*k);
						if ((*k)->get_name() == "is-directory")
							info.is_directory = get_xml_node_text(*k) == "true";
						if ((*k)->get_name() == "is-removed")
							info.is_removed = get_xml_node_text(*k) == "true";
					}
					if (!info.tmp_filename.empty())
						info.tmp_filename = temporary_directory + ETL_DIRECTORY_SEPARATOR + info.tmp_filename;
					files[info.name] = info;
				}
			}
		}
	}

	this->tag = tag;
	this->temporary_directory = temporary_directory;
	this->temporary_filename_base = temporary_filename_base;
	return true;
}

String
FileSystemTemporary::generate_indexed_temporary_filename(const FileSystem::Handle &fs, const String &filename)
{
	String extension = filename_extension(filename);
	String sans_extension = filename_sans_extension(filename);
	for(int index = 1; index < 10000; ++index)
	{
		String indexed_filename = strprintf("%s_%04d%s", sans_extension.c_str(), index, extension.c_str());
		if (!fs->is_exists(indexed_filename))
			return indexed_filename;
	}
	assert(false);
	return String();
}

/* === E N T R Y P O I N T ================================================= */


