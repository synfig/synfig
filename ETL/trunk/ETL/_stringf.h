/* =========================================================================
** Extended Template and Library
** stringf Prodecure Implementation
** $Id: _stringf.h,v 1.1.1.1 2005/01/04 01:31:48 darco Exp $
**
** Copyright (c) 2002 Robert B. Quattlebaum Jr.
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

#include <string>
#include <cstdarg>

/* === M A C R O S ========================================================= */

#ifndef ETL_STRPRINTF_MAX_LENGTH
#define ETL_STRPRINTF_MAX_LENGTH	(800)
#endif

/* === T Y P E D E F S ===================================================== */

_ETL_BEGIN_CDECLS

#if defined(__APPLE__) || defined(__CYGWIN__) || defined(_WIN32)
#define ETL_NO_THROW
#else
#define ETL_NO_THROW throw()
#endif

#ifdef HAVE_VASPRINTF	// This is the prefered method
 extern int vasprintf(char **,const char *,va_list)ETL_NO_THROW;
#else

# ifdef HAVE_VSNPRINTF	// This is the secondary method
#  if defined(__CYGWIN__)  || defined(_WIN32)
extern int vsnprintf(char *,unsigned int,const char*,va_list)ETL_NO_THROW;
#  else
extern int vsnprintf(char *,int,const char*,va_list)ETL_NO_THROW;
#  endif
# endif

#endif

#ifdef HAVE_VSSCANF
extern int vsscanf(const char *,const char *,va_list)ETL_NO_THROW;
#else
#define ETL_NO_VSTRSCANF
#ifdef HAVE_SSCANF
extern int sscanf(const char *buf, const char *format, ...)ETL_NO_THROW;
#endif
#endif

#include <unistd.h>

_ETL_END_CDECLS

/* === C L A S S E S & S T R U C T S ======================================= */

_ETL_BEGIN_NAMESPACE

inline std::string
vstrprintf(const char *format, va_list args)
{
#ifdef HAVE_VASPRINTF	// This is the prefered method (and safest)
	char *buffer;
	std::string ret;
	vasprintf(&buffer,format,args);
	ret=buffer;
	free(buffer);
	return ret;
#else
#ifdef HAVE_VSNPRINTF	// This is the secondary method (Safe, but bulky)
#warning etl::vstrprintf() has a maximum size of ETL_STRPRINTF_MAX_LENGTH in this configuration.
#ifdef ETL_THREAD_SAFE
	char buffer[ETL_STRPRINTF_MAX_LENGTH];
#else
	static char buffer[ETL_STRPRINTF_MAX_LENGTH];
#endif
	vsnprintf(buffer,sizeof(buffer),format,args);
	return buffer;
#else					// This is the worst method (UNSAFE, but "works")
#warning Potential for Buffer-overflow bug using vsprintf
#define ETL_UNSAFE_STRPRINTF	(true)
// Here, we are doubling the size of the buffer to make this case
// slightly more safe.
#ifdef ETL_THREAD_SAFE
	char buffer[ETL_STRPRINTF_MAX_LENGTH*2];
#else
	static char buffer[ETL_STRPRINTF_MAX_LENGTH*2];
#endif
	vsprintf(buffer,format,args);
	return buffer;
#endif
#endif
}

inline std::string
strprintf(const char *format, ...)
{
	va_list args;
	va_start(args,format);
	return vstrprintf(format,args);
}

#ifndef ETL_NO_VSTRSCANF
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
	return vstrscanf(data, format,args);
}
#else

#if defined (HAVE_SSCANF) && defined (__GNUC__)
#define strscanf(data,format,...) sscanf(data.c_str(),format,__VA_ARGS__)
#endif
#endif


#define stratof(X) (atof((X).c_str()))
#define stratoi(X) (atoi((X).c_str()))

inline std::string
basename(const std::string &str)
{
	std::string::const_iterator iter;

	if(str.end()[-1]==ETL_DIRECTORY_SEPERATOR)
		iter=str.end()-2;
	else
		iter=str.end()-1;

	for(;iter!=str.begin();iter--)
		if(*iter==ETL_DIRECTORY_SEPERATOR)
			break;

	if(iter==str.begin())
		return str;

	iter++;

	if(str.end()[-1]==ETL_DIRECTORY_SEPERATOR)
		return std::string(iter,str.end()-1);

	return std::string(iter,str.end());
}

inline std::string
dirname(const std::string &str)
{
	std::string::const_iterator iter;

	if(str.end()[-1]==ETL_DIRECTORY_SEPERATOR)
		iter=str.end()-2;
	else
		iter=str.end()-1;

	for(;iter!=str.begin();iter--)
		if(*iter==ETL_DIRECTORY_SEPERATOR)
			break;

	if(iter==str.begin())
		return ".";

	return std::string(str.begin(),iter);
}

inline bool
is_absolute_path(const std::string &path)
{
#ifdef WIN32
	if(path.size()>=3 && path[1]==':' && (path[2]=='\\' || path[2]=='/'))
		return true;
#endif	
	if(!path.empty() && path[0]==ETL_DIRECTORY_SEPERATOR)
		return true;
	return false;
}

inline std::string
unix_to_local_path(const std::string &path)
{
	std::string ret;
	std::string::const_iterator iter;
	for(iter=path.begin();iter!=path.end();iter++)
		switch(*iter)
		{
		case '/':
			ret+=ETL_DIRECTORY_SEPERATOR;
			break;
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
		if(*iter==ETL_DIRECTORY_SEPERATOR)
			break;
		ret+=*iter;
	}
	//if(iter!=path.end())
		ret+=ETL_DIRECTORY_SEPERATOR;
	return ret;
}

inline std::string
remove_root_from_path(std::string path)
{
	while(!path.empty())
	{
		if(path[0]==ETL_DIRECTORY_SEPERATOR)
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

	while(basename(path)==".")path=dirname(path);
	
	while(!path.empty())
	{
		std::string dir(get_root_from_path(path));
		if((dir=="../" || dir=="..\\") && ret.size())
		{
			ret=dirname(ret)+ETL_DIRECTORY_SEPERATOR;
		}
		else if((dir!="./" && dir!=".\\") && dir!=".")
		{
			ret+=dir;
		}
		path=remove_root_from_path(path);
	}
	
	// Remove any trailing directory seperators
	if(ret.size() && ret[ret.size()-1]==ETL_DIRECTORY_SEPERATOR)
	{
		ret.erase(ret.begin()+ret.size()-1);
	}
	return ret;
}

inline std::string
absolute_path(std::string path)
{
	std::string ret(current_working_directory());
	
	if(path.empty())
		return cleanup_path(ret);
	if(is_absolute_path(path))
		return cleanup_path(path);
	// TODO: This needs to be written
	return cleanup_path(ret+ETL_DIRECTORY_SEPERATOR+path);
}

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

#ifdef WIN32
	// If we are on windows and the dest path is on a different drive,
	// then there is no way to make a relative path to it.
	if(dest_path.size()>=3 && dest_path[1]==':' && dest_path[0]!=curr_path[0])
	{
		return dest_path;
	}
#endif

	if(curr_path==dirname(dest_path))
	{
		return basename(dest_path);
	}
	
	while(!dest_path.empty() && !curr_path.empty() && get_root_from_path(dest_path)==get_root_from_path(curr_path))
	{
		dest_path=remove_root_from_path(dest_path);
		curr_path=remove_root_from_path(curr_path);
	}

	while(!curr_path.empty())
	{
		dest_path=std::string("..")+ETL_DIRECTORY_SEPERATOR+dest_path;
		curr_path=remove_root_from_path(curr_path);
	}
	
	return dest_path;
}

_ETL_END_NAMESPACE

/* === E N D =============================================================== */

#endif
