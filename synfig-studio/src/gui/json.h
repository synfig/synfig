/* === S Y N F I G ========================================================= */
/*!	\file gui/json.h
**	\brief JSON parser and writer
**
**	\legal
**	Copyright (c) 2023-2025 Synfig Contributors
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

#ifndef SYNFIG_STUDIO_JSON_H
#define SYNFIG_STUDIO_JSON_H

/* === H E A D E R S ======================================================= */

#include <map>
#include <string>
#include <vector>

#include <synfig/filesystem_path.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace JSON {

class Parser {
public:
	static std::map<std::string, std::string> parse(const std::string& json_string);
	static std::map<std::string, std::string> parse(const synfig::filesystem::Path& json_filepath);

private:
	const char* input_;
	size_t pos_;
	size_t end_pos_;

	explicit Parser(const char* input, size_t length) : input_(input), pos_(0), end_pos_(length) {}
	void skip_whitespace();
	std::string parse_string();
	std::map<std::string, std::string> parse_object();
};

/** Escape UTF-8 string to JSON string */
std::string escape_string(const std::string& str);

/** Convert a JSON 'object' to JSON string {"key1":"value1","key2":"value2"} */
std::string stringify(const std::map<std::string, std::string>& obj);
/** Convert a JSON 'array' to JSON string ["value1","value2"] */
std::string stringify(const std::vector<std::string>& arr);
/** Convert a string value to JSON string "multiple\\nlines." (quotes + escapes) */
std::string stringify(const std::string& str);
/** Convert a string value to JSON string "multiple\\nlines." (quotes + escapes) */
std::string stringify(const char* str);
/** Convert a boolean value to JSON token: true or false */
std::string stringify(bool data);
/** Convert an integer number to JSON string representation */
std::string stringify(int data);
/** Convert a real number to JSON string representation */
std::string stringify(double data);
/** Convert a null value to JSON string representation: null */
std::string stringify(std::nullptr_t data);
}

#endif // JSON_H
