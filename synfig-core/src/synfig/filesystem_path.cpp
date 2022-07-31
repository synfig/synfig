/* === S Y N F I G ========================================================= */
/*!	\file filesystem_path.cpp
**	\brief class Path that handles Path components and conversions
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	......... ... 2022 Synfig Contributors
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
# include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

# include "filesystem_path.h"

# include <algorithm>
# include <vector>

# ifdef _WIN32
#  include <codecvt>
#  include <locale>

#  include "general.h" // synfig::error(...)
# endif

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

filesystem::Path::Path()
{
}

filesystem::Path::Path(const std::string& path)
	: path_(path), native_path_dirty_(true)
{
}

const filesystem::Path::value_type*
filesystem::Path::c_str() const noexcept
{
	return native().c_str();
}

const filesystem::Path::string_type&
filesystem::Path::native() const noexcept
{
	return native_path_;
}

const std::string&
filesystem::Path::u8string() const
{
	return path_;
}

int
filesystem::Path::compare(const Path& p) const noexcept
{
	int root_cmp = root_name().u8string().compare(p.root_name().u8string());
	if (root_cmp != 0)
		return root_cmp;
	bool has_root_dir = has_root_directory();
	if (has_root_dir != p.has_root_directory())
		return has_root_dir ? -1 : 1;

	auto rel_path_pos = get_relative_path_pos();
	auto p_rel_path_pos = p.get_relative_path_pos();

	while (rel_path_pos != std::string::npos && p_rel_path_pos != std::string::npos) {
		auto   next_separator_pos =   path_.find_first_of("/\\",   rel_path_pos);
		auto p_next_separator_pos = p.path_.find_first_of("/\\", p_rel_path_pos);

		auto   cur_length =   next_separator_pos != std::string::npos ?   next_separator_pos -   rel_path_pos : std::string::npos;
		auto p_cur_length = p_next_separator_pos != std::string::npos ? p_next_separator_pos - p_rel_path_pos : std::string::npos;

		auto diff = path_.compare(rel_path_pos, cur_length, p.path_, p_rel_path_pos, p_cur_length);
		if (diff != 0) {
			return diff;
		} else {
			  rel_path_pos =   path_.find_first_not_of("/\\",   next_separator_pos);
			p_rel_path_pos = p.path_.find_first_not_of("/\\", p_next_separator_pos);
		}
	}
	if (rel_path_pos == p_rel_path_pos) // == std::string::npos;
		return 0;
	if (rel_path_pos == std::string::npos)
		return -1;
	return 1;
}

filesystem::Path
filesystem::Path::lexically_normal() const
{
	return normalize(path_);
}

filesystem::Path
filesystem::Path::root_name() const
{
	return path_.substr(0, get_root_name_length());
}

filesystem::Path
filesystem::Path::root_directory() const
{
	auto root_name_length = get_root_name_length();
	if (root_name_length < path_.size() && is_separator(path_[root_name_length]))
		return path_.substr(root_name_length, 1);
	return Path();
}

filesystem::Path
filesystem::Path::root_path() const
{
	return root_name().u8string() + root_directory().u8string();
}

filesystem::Path
filesystem::Path::relative_path() const
{
	if (path_.empty())
		return Path();

	auto relative_path_pos = get_relative_path_pos();
	if (relative_path_pos == std::string::npos)
		return Path();

	return path_.substr(relative_path_pos);
}

filesystem::Path
filesystem::Path::parent_path() const
{
	auto relative_path_pos = get_relative_path_pos();
	if (relative_path_pos == std::string::npos)
		return *this;

	auto previous_component_end_pos = path_.find_last_of("/\\");

	// no directory separator? single component without root dir
	if (previous_component_end_pos == std::string::npos)
		return Path();

	// skip consecutive directory-separator /
	while (previous_component_end_pos > relative_path_pos && is_separator(path_[previous_component_end_pos - 1]))
		--previous_component_end_pos;

	auto root_name_pos = get_root_name_length();
	if (previous_component_end_pos <= root_name_pos)
		previous_component_end_pos = root_name_pos + 1;

	return path_.substr(0, previous_component_end_pos);
}

filesystem::Path
filesystem::Path::filename() const
{
	auto filename_pos = get_filename_pos();

	if (filename_pos == std::string::npos)
		return Path();

	return path_.substr(filename_pos);
}

filesystem::Path
filesystem::Path::stem() const
{
	auto filename_pos = get_filename_pos();
	if (filename_pos == std::string::npos)
		return Path();

	auto extension_pos = get_extension_pos();

	auto stem_length = extension_pos == std::string::npos ? extension_pos : extension_pos - filename_pos;

	return path_.substr(filename_pos, stem_length);
}

filesystem::Path
filesystem::Path::extension() const
{
	auto extension_pos = get_extension_pos();

	if (extension_pos == std::string::npos)
		return Path();

	return path_.substr(extension_pos);
}

bool
filesystem::Path::empty() const noexcept
{
	return path_.empty();
}

bool
filesystem::Path::has_root_name() const
{
	return get_root_name_length() > 0;
}

bool
filesystem::Path::has_root_directory() const
{
	auto root_name_length = get_root_name_length();
	return root_name_length < path_.length() && is_separator(path_[root_name_length]);
}

bool
filesystem::Path::has_root_path() const
{
	return has_root_directory() || has_root_name();
}

bool
filesystem::Path::has_relative_path() const
{
	auto relative_path_pos = get_relative_path_pos();
	return relative_path_pos != std::string::npos && relative_path_pos < path_.length();
}

bool
filesystem::Path::has_parent_path() const
{
	if (path_.empty())
		return false;
	if (has_root_directory())
		return true; // the parent path of root directory is its own parent path
	auto relative_path_pos = get_relative_path_pos();
	if (relative_path_pos == std::string::npos)
		return true; // it has a root name, but not a root directory
	auto previous_slash_pos = path_.find_first_of("/\\", relative_path_pos);
	return previous_slash_pos != std::string::npos;
}

bool
filesystem::Path::has_filename() const
{
	return get_filename_pos() != std::string::npos;
}

bool
filesystem::Path::has_stem() const
{
	auto filename_pos = get_filename_pos();
	auto extension_pos = get_extension_pos();
	return filename_pos != std::string::npos && (extension_pos == std::string::npos || filename_pos < extension_pos);
}

bool
filesystem::Path::has_extension() const
{
	return get_extension_pos() != std::string::npos;
}

bool
filesystem::Path::is_absolute() const
{
#ifdef _WIN32
	return has_root_name() && has_root_directory();
#endif
	return has_root_directory();
}

bool
filesystem::Path::is_relative() const
{
#ifdef _WIN32
	return !has_root_name() || !has_root_directory();
#endif
	return !has_root_directory();
}

std::size_t
filesystem::Path::get_root_name_length() const
{
#ifdef _WIN32
	if (path_.size() >= 2 && path_[1]==':')
		return 2;
	if (path_.size() >= 3 && path_[0] == '\\' && path_[1] == '\\' && path_[2] != '\\') {
		auto root_name_end_pos = path_.find_first_of("/\\", 3);
		if (root_name_end_pos == std::string::npos)
			return path_.length();
		return root_name_end_pos;
	}
#endif
	return 0;
}

std::size_t
filesystem::Path::get_relative_path_pos() const
{
	auto root_end_pos = root_path().path_.length();
	if (root_end_pos == 0)
		return 0;
	for (; root_end_pos < path_.length(); ++root_end_pos) {
		if (!is_separator(path_[root_end_pos]))
			return root_end_pos;
	}
	return std::string::npos;
}

std::size_t
filesystem::Path::get_filename_pos() const
{
	if (path_.empty())
		return std::string::npos;
	auto separator_pos = path_.find_last_of("/\\");
	if (separator_pos == std::string::npos)
		return 0;
	if (separator_pos + 1 == path_.size())
		return std::string::npos;
	return separator_pos + 1;
}

std::size_t
filesystem::Path::get_extension_pos() const
{
	auto dot_pos = path_.rfind('.');
	// no dot char
	if (dot_pos == std::string::npos)
		return std::string::npos;

	auto filename_pos = get_filename_pos();
	// no filename? no extension then
	if (filename_pos == std::string::npos)
		return std::string::npos;

	// last dot  char was found before filename? not an extension separator then
	if (filename_pos > dot_pos)
		return std::string::npos;

	// path is hidden file (.foo) or special dot file
	if (filename_pos == dot_pos)
		return std::string::npos;

	// path is special dot-dot (..)
	if (path_.size() - filename_pos == 2 && path_.compare(filename_pos, 2, "..") == 0)
		return std::string::npos;

	return dot_pos;
}

filesystem::Path::string_type
filesystem::Path::utf8_to_native(const std::string& utf8)
{
#ifdef _WIN32
	// Windows uses UTF-16 for filenames, so we need to convert it from UTF-8.
	try {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wcu8;
		return wcu8.from_bytes(utf8);
	} catch (const std::range_error& exception) {
		synfig::error("Failed to convert UTF-8 string (%s)", utf8.c_str());
		throw;
	}
#else
	// For other OS, it's the file name as it is
	return utf8;
#endif
}

std::string
filesystem::Path::normalize(std::string path)
{
	// Algorithm described in https://en.cppreference.com/w/cpp/filesystem/path

	// 1. If the path is empty, stop (normal form of an empty path is an empty path)
	if (path.empty())
		return path;

	// 2. Replace each directory-separator (which may consist of multiple slashes) with a single path::preferred_separator.
	// 3. Replace each slash character in the root-name with path::preferred_separator.
	// Both steps above were modified to:
	// 2-3. (modified) a. convert backslashes to slashes, except initial double slashes (\\host)
	{
		std::string::size_type i = 0;
		// For MS Windows shared folder paths like \\host\folder\file,
		// we keep \\ for now
		if (path.size() > 2 && path[0] == '\\' && path[1] == '\\')
			i = 2;
		// All other backslashes \ are replaced with slashes /
		std::replace(path.begin() + i, path.end(), '\\', '/');
	}

	// 2-3. (modified) b. replace double separators with single one
	std::string::size_type double_slash_pos = 0;
	while (true) {
		double_slash_pos = path.find("//", double_slash_pos);
		if (double_slash_pos == std::string::npos)
			break;
		++double_slash_pos;
		auto end_pos = double_slash_pos + 1;
		const auto length = path.length();
		while (end_pos < length && path[end_pos] == '/')
			++end_pos;
		path.erase(double_slash_pos, end_pos - double_slash_pos);
	}

	// 4. Remove each dot and any immediately following directory-separator.
	// 5. Remove each non-dot-dot filename immediately followed by a directory-separator and
	// a dot-dot, along with any immediately following directory-separator.

	auto relative_path_pos = Path(path).get_relative_path_pos();

	if (relative_path_pos != std::string::npos) {
		typedef std::pair<std::string::size_type, std::string::size_type> ComponentBounds;
		struct ComponentMeta {
			ComponentBounds bounds;
			bool is_dot_dot;
			bool has_slash;
		};

		std::vector<ComponentMeta> components;
		auto is_dot = [] (const std::string& p, const ComponentBounds& bounds) -> bool {
			return bounds.second == 1 && p[bounds.first] == '.';
		};
		auto is_dot_dot = [] (const std::string& p, const ComponentBounds& bounds) -> bool {
			return bounds.second == 2 && p[bounds.first] == '.' && p[bounds.first + 1] == '.';
		};
		bool has_removed_components = false;
		for (auto pos = relative_path_pos; pos < path.size(); ) {
			auto end = path.find('/', pos);
			if (end == std::string::npos)
				end = path.length();
			ComponentMeta compo{{pos, end - pos}, false, end != path.length()};
			compo.is_dot_dot = is_dot_dot(path, compo.bounds);

			pos = end + 1;

			const bool is_current_dot = !compo.is_dot_dot && is_dot(path, compo.bounds);
			has_removed_components |= compo.is_dot_dot || is_current_dot;

			// ignore special dot .
			if (is_current_dot)
				continue;

			// ignore heading special dot-dot if there is a root path
			if (components.empty() && compo.is_dot_dot && relative_path_pos > 0)
				continue;

			if (!components.empty() && compo.is_dot_dot) {
				if (!components.back().is_dot_dot) {
					components.pop_back();
					continue;
				}
			}

			components.push_back(compo);
		}

		if (has_removed_components) {
			std::string new_path;
			for (const auto& component : components) {
				new_path.append(path.substr(component.bounds.first, component.bounds.second));
				if (component.has_slash)
					new_path.push_back('/');
			}
			path = path.replace(relative_path_pos, path.length() - relative_path_pos, new_path);
		}
	}

	// 6. If there is root-directory, remove all dot-dots and any directory-separators immediately following them.

	// 7. If the last filename is dot-dot, remove any trailing directory-separator.
	if (path.length() >= 3 && path.back() == '/') {
		const std::string::size_type len = path.length();
		if (path[len - 2] == '.' && path[len - 3] == '.')
			path.erase(len - 1);
	}

	// 8. If the path is empty, add a dot (normal form of ./ is .)
	if (path.empty())
		path = '.';

	return path;
}

inline bool
filesystem::Path::is_separator(std::string::value_type c)
{
	return c == '/' || c == '\\';
}
