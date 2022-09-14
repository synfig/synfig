/* === S Y N F I G ========================================================= */
/*!	\file string_helper.cpp
**	\brief Implementation of helper functions to handle strings
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Rodolfo R Gomes
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

#ifdef USING_PCH
#	include "pch.h"
#else
# ifdef HAVE_CONFIG_H
#  include <config.h>
# endif

#include <synfig/string_helper.h>

#include <cstdarg>
#include <cstdlib>

#include <algorithm>
#include <locale>
#include <cctype>

#include "general.h"
#endif

static char
get_locale_decimal_point()
{
	// TODO(ice0): move all the locale related code to the initialization part
	// MinGW C++ std::locale accepts "C" and "POSIX" it does not support other locales.
	#ifdef __MINGW32__
		struct lconv *locale_info = localeconv();
		const char decimal_point = *locale_info->decimal_point;
	#else
		std::locale l(setlocale(LC_NUMERIC, nullptr));
		const char decimal_point = std::use_facet< std::numpunct<char> >(l).decimal_point();
	#endif
	return decimal_point;
}

std::string
synfig::remove_trailing_zeroes(const std::string& text, bool force_decimal_point)
{
	std::string result(text);
	auto decimal_point = get_locale_decimal_point();
	const size_t decimal_point_pos = text.find(decimal_point);

	if (decimal_point_pos == std::string::npos) {
		if (force_decimal_point)
			result += decimal_point + std::string("0");
	} else if (decimal_point_pos == text.length()-1) {
		if (force_decimal_point)
			result += '0';
		else
			result.pop_back();
	} else {
		const size_t last_non_zero_pos = result.find_last_not_of('0');
		result = result.substr(0, std::max(decimal_point_pos+1, last_non_zero_pos) + 1);
	}
	return result;
}

std::string
synfig::float_presentation(Real value, int num_decimals)
{
	std::string format = strprintf("%%.%df", num_decimals);
	std::string text = remove_trailing_zeroes(strprintf(format.c_str(), value));
	return text;
}

std::string
synfig::trim(const std::string& text)
{
	return right_trim(left_trim(text));
}

std::wstring
synfig::trim(const std::wstring& text)
{
	return right_trim(left_trim(text));
}

std::string
synfig::left_trim(const std::string& text)
{
	std::string result(text);
	result.erase(result.begin(),
			   std::find_if(result.begin(), result.end(),
							[](int chr) { return !std::isspace(chr);})
			   );
	return result;
}

std::wstring
synfig::left_trim(const std::wstring& text)
{
	std::wstring result(text);
	result.erase(result.begin(),
			   std::find_if(result.begin(), result.end(),
							[](int chr) { return !std::isspace(chr);})
			   );
	return result;
}

std::string
synfig::right_trim(const std::string& text)
{
	std::string result(text);
	result.erase(std::find_if(result.rbegin(), result.rend(),
							[](int chr) { return !std::isspace(chr);}).base(),
			   result.end()
			   );
	return result;
}

std::wstring
synfig::right_trim(const std::wstring& text)
{
	std::wstring result(text);
	result.erase(std::find_if(result.rbegin(), result.rend(),
							[](int chr) { return !std::isspace(chr);}).base(),
			   result.end()
			   );
	return result;
}

std::string
synfig::vstrprintf(const char *format, va_list args)
{
#ifdef _MSC_VER
	const int size = 8192; // MSVC doesn't support dynamic allocation, so make it static
#else
	// determine the length
	va_list args_copy;
	va_copy(args_copy, args);
	int size = vsnprintf(nullptr, 0, format, args_copy);
	va_end(args_copy);
	if (size < 0) size = 0;
	++size;
#endif
	// allocate buffer in stack (c99/c++11 only) and call vsnprintf again
	char buffer[size + 1]; // +space for trailing zero
	vsnprintf(buffer, size, format, args);
	return buffer;
}

std::string
synfig::strprintf(const char *format, ...)
{
	va_list args;
	va_start(args,format);
	const std::string buf = vstrprintf(format, args);
	va_end(args);
	return buf;
}

int
synfig::vstrscanf(const std::string &data, const char*format, va_list args)
{
    return vsscanf(data.c_str(),format,args);
}

int
synfig::strscanf(const std::string &data, const char*format, ...)
{
	va_list args;
	va_start(args,format);
	const int buf = vstrscanf(data, format, args);
	va_end(args);
	return buf;
}

// TODO: probably replace it with safer std::stod()
double
synfig::stratof(const std::string &str)
{
	return atof(str.c_str());
}

// TODO: probably replace it with safer std::stoi()
int
synfig::stratoi(const std::string &str)
{
	return atoi(str.c_str());
}
