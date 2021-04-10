/* === S Y N F I G ========================================================= */
/*!	\file string_helper.cpp
**	\brief Implementation of helper functions to handle strings
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2021 Rodolfo R Gomes
**
**	This package is free software; you can redistribute it and/or
**	modify it under the terms of the GNU General Public License as
**	published by the Free Software Foundation; either version 2 of
**	the License, or (at your option) any later version.
**
**	This package is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
**	General Public License for more details.
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

#include <algorithm>
#include <locale>
#endif

std::string
synfig::remove_trailing_zeroes(const std::string& text, bool force_decimal_point)
{
	std::string result(text);
	std::locale l(setlocale(LC_NUMERIC, nullptr));
	const char decimal_point = std::use_facet< std::numpunct<char> >(l).decimal_point();
	const size_t decimal_point_pos = text.find(decimal_point);

	if (decimal_point_pos == text.npos) {
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
