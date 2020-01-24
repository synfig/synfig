/* =========================================================================
** Extended Template and Library
** stringf Procedure Implementation
** $Id$
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
** Copyright (c) 2007 Chris Moore
**
** This package is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** General Public License for more details.
**
** === N O T E S ===========================================================
**
** This is an internal header file, included by other ETL headers.
** You should not attempt to use it directly.
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __ETL__STRINGF_H
#define __ETL__STRINGF_H

/* === H E A D E R S ======================================================= */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>

/* === M A C R O S ========================================================= */

#ifdef _WIN32
#define POPEN_BINARY_READ_TYPE "rb"
#define POPEN_BINARY_WRITE_TYPE "wb"
#else
#define POPEN_BINARY_READ_TYPE "r"
#define POPEN_BINARY_WRITE_TYPE "w"
#endif

/* === T Y P E D E F S ===================================================== */

extern "C" {
#include <unistd.h>
}

/* === C L A S S E S & S T R U C T S ======================================= */

namespace etl {

inline std::string
vstrprintf(const char *format, va_list args)
{
	// determine the length
	va_list args_copy;
	va_copy(args_copy, args);
	int size = vsnprintf(NULL, 0, format, args_copy);
	va_end(args_copy);
	if (size < 0) size = 0;
	++size;
	
	// allocate buffer in stack (c99/c++11 only) and call vsnprintf again
	char buffer[size];
	vsnprintf(buffer, size, format, args);
	return buffer;
}

inline std::string
strprintf(const char *format, ...)
{
	va_list args;
	va_start(args,format);
	const std::string buf = vstrprintf(format, args);
	va_end(args);
	return buf;
}

inline int
vstrscanf(const std::string &data, const char*format, va_list args)
{
    return vsscanf(data.c_str(),format,args);
}

inline int
strscanf(const std::string &data, const char*format, ...)
{
	va_list args;
	va_start(args,format);
	const int buf = vstrscanf(data, format, args);
	va_end(args);
	return buf;
}


inline double stratof(const std::string &str)
{
	return atof(str.c_str());
}

inline double stratoi(const std::string &str)
{
	return atoi(str.c_str());
}


inline bool is_separator(char c)
{
	return c == ETL_DIRECTORY_SEPARATOR0 || c == ETL_DIRECTORY_SEPARATOR1;
}

inline std::string
basename(const std::string &str)
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

	for(;iter!=str.begin();iter--)
		if(is_separator(*iter))
			break;

	if (is_separator(*iter))
		iter++;

	//if(is_separator((&*str.end())[-1]))
	if (is_separator(*(str.end()-1)))	
		return std::string(iter,str.end()-1);

	return std::string(iter,str.end());
}

inline std::string
dirname(const std::string &str)
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

	for(;iter!=str.begin();iter--)
		if(is_separator(*iter))
			break;

	if(iter==str.begin())
	{
	   if (is_separator(*iter))
		   return std::string() + ETL_DIRECTORY_SEPARATOR;
	   else
		   return ".";
	}

	return std::string(str.begin(),iter);
}

// filename_extension("/f.e/d.c") => ".c"
inline std::string
filename_extension(const std::string &str)
{
	std::string base = basename(str);
	std::string::size_type pos = base.find_last_of('.');
	if (pos == std::string::npos) return std::string();
	return base.substr(pos);
}

// filename_sans_extension("/f.e/d.c") => "/f.e/d"
inline std::string
filename_sans_extension(const std::string &str)
{
	std::string base = basename(str);
	std::string::size_type pos = base.find_last_of('.');
	if (pos == std::string::npos) return str;
	std::string dir = dirname(str);
	if (dir == ".") return base.substr(0,pos);
	return dir + ETL_DIRECTORY_SEPARATOR + base.substr(0,pos);
}

inline bool
is_absolute_path(const std::string &path)
{
#ifdef _WIN32
	if(path.size()>=3 && path[1]==':' && is_separator(path[2]))
		return true;
#endif
	if(!path.empty() && is_separator(path[0]))
		return true;
	return false;
}

inline std::string
unix_to_local_path(const std::string &path)
{
	std::string ret;
	std::string::const_iterator iter;
	for(iter=path.begin();iter!=path.end();iter++)
		if (is_separator(*iter))
			ret+=ETL_DIRECTORY_SEPARATOR;
		else
		switch(*iter)
		{
		case '~':
			ret+='~';
			break;
		default:
			ret+=*iter;
			break;
		}
	return ret;
}

inline std::string
current_working_directory()
{
	char dir[256];
	// TODO: current_working_directory() should use Glib::locale_to_utf8()
	std::string ret(getcwd(dir,sizeof(dir)));
	return ret;
}

inline std::string
get_root_from_path(std::string path)
{
	std::string ret;
	std::string::const_iterator iter;

	for(iter=path.begin();iter!=path.end();++iter)
	{
		if(is_separator(*iter))
			break;
		ret+=*iter;
	}
	//if(iter!=path.end())
		ret+=ETL_DIRECTORY_SEPARATOR;
	return ret;
}

inline std::string
remove_root_from_path(std::string path)
{
	while(!path.empty())
	{
		if(is_separator(path[0]))
		{
			path.erase(path.begin());
			return path;
		}
		path.erase(path.begin());
	}
	return path;
}

inline std::string
cleanup_path(std::string path)
{
    std::string ret;

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
				size_t pos = path.find_last_of(ETL_DIRECTORY_SEPARATORS, i-1);
				if (pos == std::string::npos) {
					path.erase(0, i+3 >= (int)path.size() ? i+3 : i+4);
					i = 0;
				}
				else
				{
					path.erase(pos + 1, (i+3 >= (int)path.size() ? i+3 : i+4) - (int)pos - 1);
					i = (int)pos;
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

inline std::string
absolute_path(std::string curr_path, std::string path)
{
	std::string ret(curr_path);
	if(path.empty())
		return cleanup_path(ret);
	if(is_absolute_path(path))
		return cleanup_path(path);
	return cleanup_path(ret+ETL_DIRECTORY_SEPARATOR+path);
}

inline std::string
absolute_path(std::string path)
	{ return absolute_path(current_working_directory(), path); }

inline std::string
relative_path(std::string curr_path,std::string dest_path)
{
	// If dest_path is already a relative path,
	// then there is no need to do anything.
	if(!is_absolute_path(dest_path))
		dest_path=absolute_path(dest_path);
	else
		dest_path=cleanup_path(dest_path);

	if(!is_absolute_path(curr_path))
		curr_path=absolute_path(curr_path);
	else
		curr_path=cleanup_path(curr_path);

#ifdef _WIN32
	// If we are on windows and the dest path is on a different drive,
	// then there is no way to make a relative path to it.
	if(dest_path.size()>=3 && dest_path[1]==':' && dest_path[0]!=curr_path[0])
		return dest_path;
#endif

	if(curr_path==dirname(dest_path))
		return basename(dest_path);

	while(!dest_path.empty() && !curr_path.empty() && get_root_from_path(dest_path)==get_root_from_path(curr_path))
	{
		dest_path=remove_root_from_path(dest_path);
		curr_path=remove_root_from_path(curr_path);
	}

	while(!curr_path.empty())
	{
		dest_path=std::string("..")+ETL_DIRECTORY_SEPARATOR+dest_path;
		curr_path=remove_root_from_path(curr_path);
	}

	return dest_path;
}

inline std::string
relative_path(std::string path)
	{ return relative_path(current_working_directory(), path); }

inline std::string
solve_relative_path(std::string curr_path,std::string dest_path)
{
	if(is_absolute_path(dest_path))
		return cleanup_path(dest_path);
	if(dest_path.empty())
		return cleanup_path(curr_path);
	return cleanup_path(curr_path + ETL_DIRECTORY_SEPARATOR + dest_path);
}


};

/* === E N D =============================================================== */

#endif
