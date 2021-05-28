/* === S Y N F I G ========================================================= */
/*!	\file filesystemtemporary.h
**	\brief FileSystemTemporary Header
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_FILESYSTEMTEMPORARY_H
#define __SYNFIG_FILESYSTEMTEMPORARY_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <set>

#include "filesystemnative.h"

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace xmlpp { class Node; }

namespace synfig
{

	class FileSystemTemporary: public FileSystem
	{
	public:
		typedef etl::handle<FileSystemTemporary> Handle;

	private:
		struct FileInfo
		{
			String name;
			String tmp_filename;
			bool is_directory;
			bool is_removed;

			inline FileInfo(): is_directory(false), is_removed(false) { }
		};

		typedef std::map<String, FileInfo> FileMap;

		FileMap files;

		FileSystemNative::Handle file_system;
		FileSystem::Handle sub_file_system;

		String tag;
		String temporary_directory;
		String temporary_filename_base;

		std::map<String, String> meta;

		bool keep_files_when_destroyed;
		bool autosave;

		static bool save_changes(
			const FileSystemNative::Handle &file_system,
			const FileSystem::Handle &target_file_system,
			FileMap &files,
			bool remove_files);

		bool create_temporary_directory() const;
		bool autosave_temporary() const;

		static String get_xml_node_text(xmlpp::Node *node);

	public:
		explicit FileSystemTemporary(const String &tag, const String &temporary_directory = String(), const FileSystem::Handle &sub_file_system = FileSystem::Handle());
		~FileSystemTemporary();

		virtual bool is_file(const String &filename);
		virtual bool is_directory(const String &filename);

		virtual bool directory_create(const String &dirname);
		virtual bool directory_scan(const String &dirname, FileList &out_files);

		virtual bool file_remove(const String &filename);
		virtual FileSystem::ReadStream::Handle get_read_stream(const String &filename);
		virtual FileSystem::WriteStream::Handle get_write_stream(const String &filename);
		virtual String get_real_uri(const String &filename);

		const FileSystem::Handle& get_sub_file_system() const
			{ return sub_file_system; }
		void set_sub_file_system(const FileSystem::Handle &file_system)
			{ this->sub_file_system = file_system; }

		bool empty() const { return files.empty(); }

		bool save_changes(const FileSystem::Handle &sub_file_system, bool as_copy);
		bool save_changes_copy(const FileSystem::Handle &sub_file_system) const;
		bool save_changes();
		void discard_changes();

		const String& get_tag() const { return tag; }
		const String& get_temporary_directory() const { return temporary_directory; }
		const String& get_temporary_filename_base() const { return temporary_filename_base; }
		void reset_temporary_filename_base() { reset_temporary_filename_base(get_tag(), get_temporary_directory()); }
		void reset_temporary_filename_base(const String &tag, const String &temporary_directory);

		String get_meta(const String &key) const;
		void set_meta(const String &key, const String &value);
		void clear_meta();

		const std::map<String, String>& get_metadata() const;
		void set_metadata(const std::map<String, String> &data);

		void set_keep_files_when_destroyed(bool value)
			{ keep_files_when_destroyed = value; }
		bool get_autosave() const
			{ return autosave; }
		void set_autosave(bool value)
			{ autosave = value; }

		bool save_temporary() const;
		bool open_temporary(const String &filename);

		static String get_system_temporary_directory();
		static String generate_temporary_filename_base(const String &tag);
		static String generate_system_temporary_filename(const String &tag);
		static String generate_indexed_temporary_filename(const FileSystem::Handle &fs, const String &filename);
		static bool scan_temporary_directory(const String &tag, FileList &out_files, const String &dirname = String());
	};

}

/* === E N D =============================================================== */

#endif
