/* === S Y N F I G ========================================================= */
/*!	\file string_helper.h
**	\brief Helper functions to handle strings
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

#ifndef SYNFIG_STRING_HELPER_H
#define SYNFIG_STRING_HELPER_H

#include <string>

namespace synfig
{

/// Remove trailing zeroes of a string with a real number.
/// It respects decimal point defined by locale and leave at least one decimal place
/// \param force_decimal_point The result string will always show the decimal point even if it isn't needed (e.g. 4 -> 4.0)
std::string remove_trailing_zeroes(const std::string& text, bool force_decimal_point = true);

/// Remove whitespaces from both ends of a string
std::string trim(const std::string& text);
std::wstring trim(const std::wstring& text);
/// Remove the leading whitespaces from a string
std::string left_trim(const std::string& text);
std::wstring left_trim(const std::wstring& text);
/// Remove the trailing whitespaces from a string
std::string right_trim(const std::string& text);
std::wstring right_trim(const std::wstring& text);

}; // END of namespace synfig

#endif // SYNFIG_STRING_HELPER_H
