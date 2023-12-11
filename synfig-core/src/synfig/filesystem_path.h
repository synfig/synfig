/* === S Y N F I G ========================================================= */
/*!	\file filesystem_path.h
**	\brief Header for class Path that handles Path components and conversions
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

/* === S T A R T =========================================================== */

#ifndef SYNFIG_FILESYSTEM_PATH_H
#define SYNFIG_FILESYSTEM_PATH_H

/* === H E A D E R S ======================================================= */

#include <string>

/* === M A C R O S ========================================================= */

// temporary macro until we replace strings with Path. Then we can get rid of it.
#define ETL_DIRECTORY_SEPARATOR	'/'

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfig
{

namespace filesystem {

/**
 * Store a file system path
 */
class Path {
public:
#ifdef _WIN32
	typedef wchar_t	value_type;
#else
	typedef char value_type;
#endif
	typedef std::basic_string<value_type> string_type;

	/**
	 * An empty file system path
	 */
	Path();

	/**
	 * Store a file system path
	 * @param path the path in UTF-8 encoding
	 */
	Path(const std::string& path);

	/**
	 * Store a file system path from a string in native encoding.
	 * @param path the path in native encoding
	 */
	static Path from_native(const string_type& native_path);

	/**
	 * Store a file system path from a file URI string.
	 *
	 * Supports a file URI as defined in IEEE RFC 8089 Appendix F.
	 *
	 * @param uri a file URI string in UTF-8 encoding ("file:///a%20file.txt")
	 */
	static Path from_uri(const std::string& uri);

	// Concatenation ---------------------

	/** Equivalent to append() */
	Path& operator/=(const Path& p);
	/**
	 * Append path_str as file path subcomponents to this path.
	 * Example: Path('one').append('two') -> Path('one/two')
	 */
	Path& append(const std::string& path_str);

	/** Equivalent to concat() */
	Path& operator+=(const Path& p);
	/**
	 * Simple string concatenation
	 * Example: Path('one').concat('two') -> Path('onetwo')
	 */
	Path& concat(const std::string& path_str);

	// Modifiers -------------------------

	/**
	 * Clears the stored pathname.
	 * empty() is true after the call.
	 */
	void clear() noexcept;
	/**
	 * Removes filename path component
	 * @return @c *this
	 */
	Path& remove_filename();
	/**
	 * Replaces the last path component with another path
	 * @param replacement the new filename component
	 * @return @c *this
	 */
	Path& replace_filename(const Path& replacement);
	/**
	 * Replaces the extension.
	 * @param replacement the new extension or let it blank to remove the current one
	 * @return @c *this
	 */
	Path& replace_extension(const Path& replacement = Path());
	/**
	 * Add a suffix to filename stem.
	 * Example:
	 * @code{.cpp}
	 * auto path = synfig::filesystem::Path("a/b.png");
	 * path.add_suffix("-000");
	 * // now variable 'path' contains "a/b-000.png".
	 * @endcode
	 *
	 * @param suffix the string to be appended in filename stem
	 * @return @c *this
	 */
	Path& add_suffix(const std::string& suffix);
	/**
	 * Swaps two paths
	 * @param other
	 */
	void swap(Path& other) noexcept;

	// Format observers ------------------

	/** Path as a character string in native encoding */
	const value_type* c_str() const noexcept;
	/** Path as a character string in native encoding */
	const string_type& native() const noexcept;
	/** Path as a character string in UTF-8 encoding */
	const std::string::value_type* u8_str() const noexcept;
	/** Path as a character string in UTF-8 encoding */
	const std::string& u8string() const noexcept;

	// Compare ---------------------------

	int compare(const Path& p) const noexcept;

	// Generation ------------------------

	/**
	 * Returns the the normal form of the path.
	 *
	 * It does not access the file system, just
	 * handles the path string itself:
	 * - remove directory separator duplicates, e.g. ///
	 * - 'parse' special dot path component: .
	 * - 'parse' special dot-dot path component: ..
	 * - use slash as directory separator: /
	 */
	Path lexically_normal() const;

	/**
	 * Alias for lexically_normal()
	 */
	Path cleanup() const;

	/**
	 * Returns @c *this made relative to @a base.
	 * @param base the reference path
	 * @return the path relative to base
	 */
	Path lexically_relative(const Path& base) const;

	/**
	 * Convenient method that calls lexically_relative() followed by lexically_normal().
	 * @param base the reference path
	 * @return the normalized path relative to base
	 */
	Path relative_to(const Path& base) const;

	/**
	 * If the value of lexically_relative(@a base) is not an empty path, return it. Otherwise return *this.
	 *
	 * Examples:
	 *     Path("a/b").lexically_relative("/a/b") returns empty
	 *     Path("a/b").lexically_proximate("/a/b") returns "a/b"
	 *
	 *     Path("/a/b").lexically_relative("c") returns empty
	 *     Path("/a/b").lexically_proximate("c") returns "/a/b"
	 *
	 * @param base the reference path
	 * @return the path relative to base
	 */
	Path lexically_proximate(const Path& base) const;

	/**
	 * Convenient method that calls lexically_proximate() followed by lexically_normal().
	 * @param base the reference path
	 * @return the normalized path proximate to base
	 */
	Path proximate_to(const Path& base) const;

	// Decomposition ---------------------

	/**
	 * The root name (MS Windows only), if present
	 * Example: "C:", "E:", "\\host", etc.
	 */
	Path root_name() const;
	/**
	 * The root directory, if present.
	 * This is the first component of an absolute path.
	 * Empty data if it is a relative path.
	 */
	Path root_directory() const;
	/**
	 * The root path of the path, if present.
	 * @return root_name() followed by root_directory()
	 */
	Path root_path() const;
	/** The path relative to (i.e. after) root path. */
	Path relative_path() const;
	/** The path to the parent directory. */
	Path parent_path() const;
	/**
	 * Last component of path.
	 * filename stem + extension
	 */
	Path filename() const;
	/**
	 * File name stem.
	 * @return the substring from the beginning of filename() up to the beginning of extension().
	 * Dot character of extension is not included.
	 */
	Path stem() const;
	/** File name extension (includes its initial dot if file has extension) */
	Path extension() const;

	// Queries ---------------------------

	bool empty() const noexcept;

	bool has_root_name() const;
	bool has_root_directory() const;
	bool has_root_path() const;
	bool has_relative_path() const;
	bool has_parent_path() const;
	bool has_filename() const;
	bool has_stem() const;
	bool has_extension() const;

	/** Checks whether the path is absolute or relative. */
	bool is_absolute() const;
	/** Checks whether the path is absolute or relative. */
	bool is_relative() const;

	friend Path operator/(const Path& lhs, const Path& rhs)
		{ return Path(lhs) /= rhs; }

	friend Path operator+(const Path& lhs, const Path& rhs)
		{ return Path(lhs) += rhs; }

	/** check if character @a c is a path separator **/
	static bool is_separator(std::string::value_type c);

	///// Temporary methods, for future replacement with equivalents
	static std::string basename(const std::string& str);
	static std::string dirname(const std::string& str);

	static std::string filename_extension(const std::string& str);
	static std::string filename_sans_extension(const std::string& str);
	static bool is_absolute_path(const std::string& path);

	static std::string cleanup_path(std::string path);

	static std::string absolute_path(const std::string& curr_path, const std::string& path);
	static std::string absolute_path(const std::string& path);

private:
	/** Path in the native encoding */
	string_type native_path_;
	/** Path in UTF-8 encoding */
	std::string path_;

	bool native_path_dirty_;
	void sync_native_path();

	std::size_t get_root_name_length() const;
	std::size_t get_relative_path_pos() const;
	std::size_t get_filename_pos() const;
	std::size_t get_extension_pos() const;

	/**
	 * Convert a UTF-8 encoded string into a native-encoded string
	 * @param utf8 the string to be converted
	 * @return a string in native encoding
	 */
	static string_type utf8_to_native(const std::string& utf8);
	/**
	 * Converts a path to its normal form
	 * @param utf8 the path in UTF-8 encoding
	 * @return the normalized path in UTF-8 encoding
	 */
	static std::string normalize(std::string path);
}; // END of class Path

// Non-member functions --------------

void swap(synfig::filesystem::Path& lhs, synfig::filesystem::Path& rhs) noexcept;

/**
 * Returns the current path, also known as current working directory.
 *
 * It just calls the synfig::OS::get_current_working_directory().
 * It is defined here for convinience and to imitate the C++17 filesystem namespace.
 *
 * @return the current working directory
 */
Path current_path();

/**
 * The absolute path of relative (or absolute) path @a p.
 *
 * If @a p is an absolute path, returns itself.
 * Otherwise, resolves the rlative path p based on the current working directory.
 * @see synfig::filesystem::current_path()
 *
 * @param p the path to find its absolute path version
 * @return current_path() / p
 */
Path absolute(const synfig::filesystem::Path& p);

inline bool operator==(const Path& lhs, const Path& rhs) noexcept
	{ return lhs.compare(rhs) == 0; }

inline bool operator!=(const Path& lhs, const Path& rhs) noexcept
	{ return !(lhs == rhs); }

inline bool operator<(const Path& lhs, const Path& rhs) noexcept
	{ return lhs.compare(rhs) < 0; }

inline bool operator<=(const Path& lhs, const Path& rhs) noexcept
	{ return !(rhs < lhs); }

inline bool operator>(const Path& lhs, const Path& rhs) noexcept
	{ return rhs < lhs; }

inline bool operator>=(const Path& lhs, const Path& rhs) noexcept
	{ return !(lhs < rhs); }

} // END of namespace filesystem

} // END of namespace synfig

/* === E N D =============================================================== */

#endif // SYNFIG_FILESYSTEM_PATH_H
