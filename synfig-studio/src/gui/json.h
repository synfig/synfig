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

	explicit Parser(const char* input) : input_(input), pos_(0) {}
	void skip_whitespace();
	std::string parse_string();
	std::map<std::string, std::string> parse_object();
};

std::string escape_string(const std::string& str);
}

#endif // JSON_H
