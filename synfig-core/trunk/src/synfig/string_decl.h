/* === S Y N F I G ========================================================= */
/*!	\file string_decl.h
**	\brief Template Header
**
**	$Id: string_decl.h,v 1.1.1.1 2005/01/04 01:23:14 darco Exp $
**
**	\legal
**	Copyright (c) 2002 Robert B. Quattlebaum Jr.
**
**	This software and associated documentation
**	are CONFIDENTIAL and PROPRIETARY property of
**	the above-mentioned copyright holder.
**
**	You may not copy, print, publish, or in any
**	other way distribute this software without
**	a prior written agreement with
**	the copyright holder.
**	\endlegal
*/
/* ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STRING_DECL_H
#define __SYNFIG_STRING_DECL_H

/* === H E A D E R S ======================================================= */

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace std
{
template<class _CharT> struct char_traits;
template<> struct char_traits<char>;
template<typename _Alloc> class allocator;
template<typename _CharT, typename _Traits,typename _Alloc>class basic_string;
typedef basic_string<char,char_traits<char>,allocator<char> >    string;

}; // END of namespace std

namespace synfig
{

/*!	\typedef String
**	\todo writeme
*/
typedef std::string String;

}; // END of namespace synfig

/* === E N D =============================================================== */

#endif
