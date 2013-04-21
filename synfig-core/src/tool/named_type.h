/* === S Y N F I G ========================================================= */
/*!	\file tool/named_type.h
**	\brief Named type class
**
**	Parameter value type with name
**
**	$Id$
**
**	\legal
**	Copyright (c) 2012 Diego Barrios Romero
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

#ifndef __SYNFIG_NAMED_TYPE_H
#define __SYNFIG_NAMED_TYPE_H

/**
 * Class for the arguments with name since the Boost library doesn't
 * support named arguments, just hardcoded "arg".
 * The arguments with a description of the expected argument are clearer
 * and hence this class.
 * \sa https://svn.boost.org/trac/boost/ticket/4781
 */
template<typename T>
struct named_type
    : public boost::program_options::typed_value<T>
{
    named_type(std::string const& name)
        : boost::program_options::typed_value<T>(&value)
        , _name(name)
    {
    }
    std::string name() const { return _name; }
    std::string _name;
    T value;
};

#endif
