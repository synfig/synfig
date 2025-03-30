/* === S Y N F I G ========================================================= */
/*!	\file gui/json.cpp
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "json.h"

#include <sstream>
#include <stdexcept>

#include <synfig/filesystemnative.h>
#include <synfig/general.h>
#include <synfig/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace JSON;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

void
Parser::skip_whitespace()
{
	while (pos_ < end_pos_ && (
			input_[pos_] == ' ' || input_[pos_] == '\n' ||
			input_[pos_] == '\r' || input_[pos_] == '\t')
		   ) {
		pos_++;
	}
}

std::string
Parser::parse_string()
{
	pos_++; // Skip opening quote
	std::string value;
	while (pos_ < end_pos_ && input_[pos_] != '"') {
		if (input_[pos_] != '\\') {
			value += input_[pos_];
		} else {
			pos_++;
			if (pos_ >= end_pos_) {
				value += u8"�";
				synfig::error("JSON: malformed string: incomplete escape sequence");
				break;
			}
			switch (input_[pos_]) {
			case '"': value += '"'; break;
			case '\\': value += '\\'; break;
			case '/': value += '/'; break;
			case 'b': value += '\b'; break;
			case 'f': value += '\f'; break;
			case 'n': value += '\n'; break;
			case 'r': value += '\r'; break;
			case 't': value += '\t'; break;
			case 'u': {
				// unicode escape: \u____ (codepoint in hexadecimal)
				if (pos_ + 4 >= end_pos_) {
					value += u8"�";
					synfig::error("JSON: malformed string: incomplete unicode escape sequence");
					break;
				}
				// Skip unicode handling for now
				pos_ += 4;
				value += '?';
				break;
			}
			default: {
				value += u8"�";
				synfig::error("JSON: malformed string: unknown escape sequence");
				break;
			}
			}
		}
		pos_++;
	}
	if (pos_ < end_pos_) {
		pos_++; // Skip closing quote
	} else {
		synfig::error("JSON: malformed string: string must end with a closing quote \"");
	}
	return value;
}

std::map<std::string, std::string>
Parser::parse_object()
{
	pos_++; // Skip opening brace
	std::map<std::string, std::string> object;

	while (true) {
		skip_whitespace();
		if (pos_ >= end_pos_) {
			throw std::runtime_error("JSON: unfinished object: missing closing brace");
		}

		if (input_[pos_] == '}') {
			pos_++;
			break;
		}

		if (!object.empty()) {
			if (input_[pos_] != ',')
				throw std::runtime_error("JSON: Expected comma in object");
			pos_++;
			skip_whitespace();
			if (pos_ >= end_pos_) {
				throw std::runtime_error("JSON: malformed object: missing next key-value pair");
			}
		}

		if (input_[pos_] != '"')
			throw std::runtime_error("JSON: Expected string key in object");

		std::string key = parse_string();
		if (pos_ >= end_pos_ || key.empty()) {
			throw std::runtime_error("JSON: malformed object: missing key string");
		}
		skip_whitespace();
		if (pos_ >= end_pos_)
			throw std::runtime_error("JSON: Expected colon after key in object");

		pos_++; // Skip colon

		skip_whitespace();
		if (pos_ >= end_pos_)
			throw std::runtime_error("JSON: missing value for key in object");

		if (input_[pos_] == '"') {
			object[key] = parse_string();
			if (pos_ >= end_pos_)
				throw std::runtime_error("JSON: incomplete string value in object");
		} else if (input_[pos_] == '{') {
			// Skip nested objects for now, treat as empty string
			int depth = 1;
			while (depth > 0) {
				pos_++;
				if (pos_ >= end_pos_)
					throw std::runtime_error("JSON: malformed object");

				if (input_[pos_] == '{') depth++;
				if (input_[pos_] == '}') depth--;
			}
			pos_++;
			if (pos_ >= end_pos_)
				throw std::runtime_error("JSON: malformed object");
			object[key] = "";
		} else if (input_[pos_] == '[') {
			// Skip arrays for now, treat as empty string
			int depth = 1;
			while (depth > 0) {
				pos_++;
				if (pos_ >= end_pos_)
					throw std::runtime_error("JSON: malformed array");

				if (input_[pos_] == '[') depth++;
				if (input_[pos_] == ']') depth--;
			}
			pos_++;
			if (pos_ >= end_pos_)
				throw std::runtime_error("JSON: malformed object");
			object[key] = "";
		} else {
			// Parse number or other value until next delimiter
			std::string value;
			while (pos_ < end_pos_ && input_[pos_] != ',' && input_[pos_] != '}') {
				value += input_[pos_];
				pos_++;
			}
			object[key] = value;
		}
	}

	return object;
}

std::map<std::string, std::string>
Parser::parse(const std::string& json_string)
{
	Parser parser(json_string.c_str(), json_string.size());
	parser.skip_whitespace();
	if (parser.input_[parser.pos_] != '{')
		throw std::runtime_error("Expected object");
	return parser.parse_object();
}

std::map<std::string, std::string>
Parser::parse(const synfig::filesystem::Path& json_filepath) {
	auto stream = synfig::FileSystemNative::instance()->get_read_stream(json_filepath.u8string());
	if (!stream) {
		synfig::error(_("Could not open JSON file for reading: %s"), json_filepath.u8_str());
		return std::map<std::string, std::string>{};
	}
	std::stringstream buffer;
	buffer << stream->rdbuf();
	return JSON::Parser::parse(buffer.str());
}

std::string
JSON::escape_string(const std::string& str)
{
	std::string value;
	for (char c : str) {
		switch (c) {
		case '"':  value += "\\\""; break;
		case '\\': value += "\\\\"; break;
		case '/':  value += "\\/"; break;
		case '\b': value += "\\b"; break;
		case '\f': value += "\\f"; break;
		case '\n': value += "\\n"; break;
		case '\r': value += "\\r"; break;
		case '\t': value += "\\t"; break;
		default:
			if ((unsigned char)c >= 0x20) {
				value.push_back(c);
			} else {
				char unicode[8];
				snprintf(unicode, 8, "\\u00%02x", (unsigned char)c);
				value += unicode;
			}
		}
	}
	return value;
}
