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

	// Format observers ------------------

	/** Path as a character string in native encoding */
	const value_type* c_str() const noexcept;
	/** Path as a character string in native encoding */
	const string_type& native() const noexcept;
	/** Path as a character string in UTF-8 encoding */
	const std::string& u8string() const;

	// Decomposition ---------------------

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

	bool has_filename() const;
	bool has_stem() const;
	bool has_extension() const;

private:
	/** Path in the native encoding */
	string_type native_path_;
	/** Path in UTF-8 encoding */
	std::string path_;

	std::size_t get_filename_pos() const;
	std::size_t get_extension_pos() const;

	/**
	 * Convert a UTF-8 encoded string into a native-encoded string
	 * @param utf8 the string to be converted
	 * @return a string in native encoding
	 */
	static string_type utf8_to_native(const std::string& utf8);
}; // END of class Path

} // END of namespace filesystem

} // END of namespace synfig

/* === E N D =============================================================== */

#endif // SYNFIG_FILESYSTEM_PATH_H
