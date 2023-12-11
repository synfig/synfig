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

#include <glibmm/miscutils.h>

#include <synfig/general.h>
#include <synfig/os.h>

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

static std::string
native_to_utf8(const filesystem::Path::string_type& native)
{
#ifdef _WIN32
	// Windows uses UTF-16 for filenames, so we do need to convert it to UTF-8.
	try {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> wcu16;
		return wcu16.to_bytes(native);
	} catch (const std::range_error& exception) {
		synfig::error("Failed to convert UTF-16 string (%s)", native.c_str());
		throw;
	}
#else
	// For other OS, it's the file name as it is
	return native;
#endif
}


/* === M E T H O D S ======================================================= */

filesystem::Path::Path()
{
}

filesystem::Path::Path(const std::string& path)
	: path_(path), native_path_dirty_(true)
{
}

filesystem::Path
filesystem::Path::from_native(const string_type& native_path)
{
	return Path(native_to_utf8(native_path));
}

filesystem::Path
filesystem::Path::from_uri(const std::string& uri)
{
	if (uri.empty())
		return {};

	if (uri.size() >= 5 && uri.compare(0, 5, "file:") != 0) {
		synfig::warning("String is not a file URI \"%s\"", uri.c_str());
		return {};
	}

	bool is_unc_auth = false;
	bool is_file_auth = false;

	auto auth_pos = std::string::npos;
	auto path_absolute_pos = std::string::npos;

	std::string auth;

	if (uri.size() >= 9 && uri.compare(5, 4, "////") == 0) { // "file:////"
		is_unc_auth = true;

		if (uri.size() >= 10 && uri[9] == '/') // "file://///"
			auth_pos = 10;
		else
			auth_pos = 9;
	} else if (uri.size() >= 7 && uri.compare(5, 2, "//") == 0) { // "file://"
		is_file_auth = true;

		auth_pos = 7;
	}

	if (!is_file_auth && !is_unc_auth) {
		path_absolute_pos = 5;
	} else {
		path_absolute_pos = uri.find('/', auth_pos);

		if (path_absolute_pos == std::string::npos) {
			synfig::warning("URI has authority part \"%s\", but does not have the file absolute path in URI \"%s\"", uri.substr(auth_pos).c_str(), uri.c_str());
			return {};
		}

		auth = uri.substr(auth_pos, path_absolute_pos - auth_pos);
	}


	std::string drive_letter;

	if (!is_unc_auth) {
		auto get_drive_letter = [] (const char* str) -> char {
			if (str[1] == ':' || str[1] == '|') {
				char letter = str[0];
				if ((letter >= 'A' && letter <= 'Z') || (letter >= 'a' && letter <= 'z')) {
					//return letter < 'Z' ? letter : (letter - 'a' + 'A');
					return letter;
				}
			}
			return 0;
		};

		if (path_absolute_pos + 3 < uri.size()) {
			if (uri[path_absolute_pos] == '/') {
				char letter = get_drive_letter(&uri[path_absolute_pos+1]);
				if (letter) {
					drive_letter.push_back(letter);
					drive_letter.push_back(':');
					path_absolute_pos += 3;
				}
			}
		}
		if (drive_letter.empty() && !is_file_auth && path_absolute_pos + 2 < uri.size()) {
			char letter = get_drive_letter(&uri[path_absolute_pos]);
			if (letter) {
				drive_letter.push_back(letter);
				drive_letter.push_back(':');
				path_absolute_pos += 2;
			}
		}
	}

	// Just for safety, eliminate (improbable) query and fragment components
	auto path_absolute_end_pos = uri.size();
	{
		auto query_pos = uri.find('?', path_absolute_end_pos);
		if (query_pos != std::string::npos) {
			path_absolute_end_pos = query_pos;
		} else {
			auto fragment_pos = uri.find('#', path_absolute_end_pos);
			if (fragment_pos != std::string::npos) {
				path_absolute_end_pos = fragment_pos;
			}
		}
	}
	// Decode percent-encoding
	std::string file_absolute;

	for (auto p = path_absolute_pos; p < path_absolute_end_pos; ++p) {
		if (uri[p] != '%') {
			file_absolute.push_back(uri[p]);
		} else {
			if (p + 2 < path_absolute_end_pos) {
				// decode char
				char c1 = uri[++p];
				char c2 = uri[++p];
				if (c1 >= '0' && c1 <= '9')
					c1 -= '0';
				else
					c1 -= (c1 >= 'a') ? 'a' : 'A';
				if (c1 < 0 || c1 > 15) {
					break;
				}
				if (c2 >= '0' && c2 <= '9')
					c2 -= '0';
				else
					c2 -= (c2 >= 'a') ? 'a' : 'A';
				if (c2 < 0 || c2 > 15) {
					break;
				}
				file_absolute.push_back(c1 << 4 | c2);
			} else {
				break;
			}
		}
	}

	if (!auth.empty() && auth != "localhost") { // section 2 says "localhost" is interpreted exactly as if no authority were present
		return Path{"\\\\" + auth + (!drive_letter.empty() ? '/' + drive_letter : "") + file_absolute};
	} else {
		return Path{drive_letter + file_absolute};
	}
}

filesystem::Path&
filesystem::Path::operator/=(const Path& p)
{
	if (p.is_absolute() || (p.has_root_name() && p.root_name() != root_name())) {
		*this = p;
		return *this;
	}

	if (p.has_root_directory())
		path_.erase(get_root_name_length());
	else if (has_filename() || (!has_root_directory() && is_absolute()))
		path_.push_back('/');
	path_.append(p.path_, p.get_root_name_length(), std::string::npos);
	native_path_dirty_ = true;
	return *this;
}

filesystem::Path&
filesystem::Path::append(const std::string& path_str)
{
	return *this /= Path(path_str);
}

filesystem::Path&
filesystem::Path::operator+=(const Path& p)
{
	return concat(p.path_);
}

filesystem::Path&
filesystem::Path::concat(const std::string& path_str)
{
	if (!path_str.empty()) {
		path_.append(path_str);
		native_path_dirty_ = true;
	}
	return *this;
}

void
filesystem::Path::clear() noexcept
{
	path_.clear();
	native_path_.clear();
	native_path_dirty_ = false;
}

filesystem::Path&
filesystem::Path::remove_filename()
{
	auto pos = get_filename_pos();
	if (pos != std::string::npos) {
		path_.erase(pos);
		native_path_dirty_ = true;
	}
	return *this;
}

filesystem::Path&
filesystem::Path::replace_filename(const Path& replacement)
{
	remove_filename();
	return operator/=(replacement);
}

filesystem::Path&
filesystem::Path::replace_extension(const Path& replacement)
{
	auto pos = get_extension_pos();
	if (pos != std::string::npos) {
		path_.erase(pos);
		native_path_dirty_ = true;
	}
	if (!replacement.empty()) {
		if (replacement.u8string()[0] != '.') {
			path_.push_back('.');
		}
		path_.append(replacement.u8string());
		native_path_dirty_ = true;
	}
	return *this;
}

filesystem::Path&
filesystem::Path::add_suffix(const std::string& suffix)
{
	auto ext_pos = get_extension_pos();
	if (ext_pos == std::string::npos) {
		auto file_name = filename().u8string();
		if (file_name != "." && file_name != "..")
			path_.append(suffix);
	} else {
		path_.insert(ext_pos, suffix);
	}
	native_path_dirty_ = true;
	return *this;
}

void
filesystem::Path::swap(Path& other) noexcept
{
	path_.swap(other.path_);
	if (!native_path_dirty_ || !other.native_path_dirty_) {
		native_path_.swap(other.native_path_);
		std::swap(native_path_dirty_, other.native_path_dirty_);
	}
}

const filesystem::Path::value_type*
filesystem::Path::c_str() const noexcept
{
	return native().c_str();
}

const filesystem::Path::string_type&
filesystem::Path::native() const noexcept
{
	const_cast<Path*>(this)->sync_native_path();
	return native_path_;
}

const std::string&
filesystem::Path::u8string() const noexcept
{
	return path_;
}

const std::string::value_type*
filesystem::Path::u8_str() const noexcept
{
	return path_.c_str();
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
filesystem::Path::cleanup() const
{
	return lexically_normal();
}

filesystem::Path
filesystem::Path::lexically_relative(const Path& base) const
{
	if (root_name() != base.root_name()
		|| is_absolute() != base.is_absolute()
		|| (!has_root_directory() && base.has_root_directory()))
	{
		return Path();
	}

	class simple_iterator {
		const Path& path_;
		std::size_t pos_;
		std::string element_;

	public:
		explicit simple_iterator(const Path& p)
			: path_(p), pos_(p.get_relative_path_pos())
		{
			if (p.empty()) {
				pos_ = std::string::npos;
				return;
			}

			element_ = fetch_current_element(pos_);
		}

		simple_iterator& operator++()
		{
			// at end already?
			if (pos_ == std::string::npos || pos_ >= path_.path_.length()) {
				pos_ = std::string::npos;
				element_.clear();
				return *this;
			}

			// note: root path element ignored

			// skip current element string
			auto end = pos_ + element_.length();
			if (end >= path_.path_.length()) {
				pos_ = std::string::npos;
				element_.clear();
				return *this;
			}

			// search for next element string beginning
			auto next = path_.path_.find_first_not_of("/\\", end + 1);
			if (next == std::string::npos) {
				pos_ = path_.path_.length();
				element_.clear();
				return *this;
			}

			element_ = fetch_current_element(next);
			pos_ = next;

			return *this;
		}

		const std::string& operator*()
		{
			return element_;
		}

		const std::string* operator->()
		{
			return &element_;
		}

		bool is_end() const
		{
			return element_.empty() && pos_ == std::string::npos;
		}

	private:
		std::string fetch_current_element(std::size_t pos)
		{
			if (pos == std::string::npos)
				return "";
			auto end = path_.path_.find_first_of("/\\", pos);
			auto length = end == std::string::npos ? end : (end - pos);
			return path_.path_.substr(pos, length);
		}
	};

	simple_iterator a(*this), b(base);
	while (!a.is_end() && !b.is_end() && *a == *b) {
		++a;
		++b;
	}

	if (a.is_end() && b.is_end())
		return Path(".");

	int N = 0;
	while (!b.is_end()) {
		if (*b == "..")
			--N;
		else if (*b != ".")
			++N;
		++b;
	}

	if (N < 0)
		return Path();

	if (N == 0 && (a.is_end() || a->empty()))
		return Path(".");

	Path q;
	while (N > 0) {
		q /= Path("..");
		--N;
	}

	while (!a.is_end()) {
		q /= *a;
		++a;
	}

	return q;
}

filesystem::Path
filesystem::Path::relative_to(const Path& base) const
{
	return lexically_relative(base).lexically_normal();
}

filesystem::Path
filesystem::Path::lexically_proximate(const Path& base) const
{
	Path rel(lexically_relative(base));
	if (rel.empty())
		return *this;
	return rel;
}

filesystem::Path
filesystem::Path::proximate_to(const Path& base) const
{
	return /*absolute*/(*this).lexically_normal().lexically_proximate(/*absolute*/(base)).lexically_normal();
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

void
filesystem::Path::sync_native_path()
{
	if (!native_path_dirty_)
		return;
	native_path_dirty_ = false;
	native_path_ = utf8_to_native(path_);
}

std::size_t
filesystem::Path::get_root_name_length() const
{
#ifdef _WIN32
	if (path_.size() >= 2 && path_[1]==':' && ((path_[0] >= 'A' && path_[0] <= 'Z') || (path_[0] >= 'a' && path_[0] <= 'z')))
		return 2;
#endif
	if (path_.size() >= 3 && path_[0] == '\\' && path_[1] == '\\' && path_[2] != '\\') {
		auto root_name_end_pos = path_.find_first_of("/\\", 3);
		if (root_name_end_pos == std::string::npos)
			return path_.length();
		return root_name_end_pos;
	}
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
		return get_relative_path_pos();
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

bool
filesystem::Path::is_separator(std::string::value_type c)
{
	return c == '/' || c == '\\';
}

void
filesystem::swap(Path& lhs, Path& rhs) noexcept
{
	return lhs.swap(rhs);
}

filesystem::Path
filesystem::current_path()
{
	return synfig::OS::get_current_working_directory();
}

filesystem::Path
filesystem::absolute(const Path& p)
{
	if (p.is_absolute()) // avoid current_path() computation
		return p;
	return current_path() / p;
}

std::string
filesystem::Path::basename(const std::string& str)
{
	std::string::const_iterator iter;

	if(str.empty())
		return std::string();

	if(str.size() == 1 && is_separator(str[0]))
		return str;

	//if(is_separator((&*str.end())[-1]))
	//if (is_separator(*str.rbegin()))
	if(is_separator(*(str.end()-1)))
		iter=str.end()-2;
	else
		iter=str.end()-1;

	for(;iter!=str.begin();--iter)
		if(is_separator(*iter))
			break;

	if (is_separator(*iter))
		++iter;

	//if(is_separator((&*str.end())[-1]))
	if (is_separator(*(str.end()-1)))
		return std::string(iter,str.end()-1);

	return std::string(iter,str.end());
}

std::string
filesystem::Path::dirname(const std::string& str)
{
	std::string::const_iterator iter;

	if(str.empty())
		return std::string();

	if(str.size() == 1 && is_separator(str[0]))
		return str;

	//if(is_separator((&*str.end())[-1]))
	if(is_separator(*(str.end()-1)))
	//if (is_separator(*str.rbegin()))
		iter=str.end()-2;
	else
		iter=str.end()-1;

	for(;iter!=str.begin();--iter)
		if(is_separator(*iter))
			break;

	if(iter==str.begin())
	{
	   if (is_separator(*iter))
		   return std::string() + "/";
	   else
		   return ".";
	}

#ifdef _WIN32
	// leave the trailing separator after windows drive name
	if (std::distance(str.begin(), iter) == 2 && str.size() >= 3 && str[1] == ':' && is_separator(str[2]))
		++iter;
#endif

	return std::string(str.begin(),iter);
}

std::string
filesystem::Path::filename_extension(const std::string& str)
{
	std::string base = basename(str);
	std::string::size_type pos = base.find_last_of('.');
	if (pos == std::string::npos) return std::string();
	return base.substr(pos);
}

std::string
filesystem::Path::filename_sans_extension(const std::string& str)
{
	std::string base = basename(str);
	std::string::size_type pos = base.find_last_of('.');
	if (pos == std::string::npos) return str;
	std::string dir = dirname(str);
	if (dir == ".") return base.substr(0,pos);
	return dir + "/" + base.substr(0,pos);
}

bool
filesystem::Path::is_absolute_path(const std::string& path)
{
#ifdef _WIN32
	if(path.size()>=3 && path[1]==':' && is_separator(path[2]))
		return true;
#endif
	if(!path.empty() && is_separator(path[0]))
		return true;
	return false;
}

std::string
filesystem::Path::cleanup_path(std::string path)
{
	// remove '.'
	for(int i = 0; i < (int)path.size();)
	{
		if ( path[i] == '.'
		  && (i-1 <  0                || is_separator(path[i-1]))
		  && (i+1 >= (int)path.size() || is_separator(path[i+1])) )
		{
			path.erase(i, i+1 < (int)path.size() ? 2 : 1);
		} else {
			++i;
		}
	}

	// remove double separators
	for(int i = 0; i < (int)path.size()-1;)
		if ( is_separator(path[i]) && is_separator(path[i+1]) )
			path.erase(i+1, 1); else ++i;

	// solve '..'
	for(int i = 0; i < (int)path.size()-3;)
	{
		if ( is_separator(path[i])
		  && path[i+1] == '.'
		  && path[i+2] == '.'
		  && (i+3 >= (int)path.size() || is_separator(path[i+3])) )
		{
			// case "/../some/path", remove "../"
			if (i == 0) {
				path.erase(i+1, i+3 >= (int)path.size() ? 2 : 3);
			}
			else
			// case "../../some/path", skip
			if ( i-2 >= 0
			  && path[i-1] == '.'
			  && path[i-2] == '.'
			  && (i-3 < 0 || is_separator(path[i-3])) )
			{
				++i;
			}
			// case "some/thing/../some/path", remove "thing/../"
			else
			{
				// so now we have:
				// i > 0, see first case,
				// path[i-1] is not a separator (double separators removed already),
				// so path[i-1] is part of valid directory entry,
				// also is not a special entry ('.' or '..'), see previous case and stage "remove '.'"
				size_t dir_separator_pos = path.find_last_of("/\\", i-1);
				if (dir_separator_pos == std::string::npos) {
					path.erase(0, i+3 >= (int)path.size() ? i+3 : i+4);
					i = 0;
				}
				else
				{
					path.erase(dir_separator_pos + 1, (i+3 >= (int)path.size() ? i+3 : i+4) - (int)dir_separator_pos - 1);
					i = (int)dir_separator_pos;
				}
			}
		}
		else
		{
			++i;
		}
	}

	// remove separator from end of path
	if (path.size() > 1u && is_separator(path[path.size() - 1]))
		path.erase(path.size() - 1, 1);

	return path;
}

std::string
filesystem::Path::absolute_path(const std::string& curr_path, const std::string& path)
{
	std::string ret(curr_path);
	if(path.empty())
		return cleanup_path(ret);
	if(is_absolute_path(path))
		return cleanup_path(path);
	return cleanup_path(ret+"/"+path);
}

std::string
filesystem::Path::absolute_path(const std::string& path)
	{ return absolute_path(Glib::get_current_dir(), path); }
