/* === S I N F G =========================================================== */
/*!	\file uimanager.cpp
**	\brief Template File
**
**	$Id: uimanager.cpp,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "uimanager.h"
#include <iostream>
#include <string>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace sinfgapp;

/* === M A C R O S ========================================================= */

/* === M E T H O D S ======================================================= */

UIInterface::Response
ConsoleUIInterface::yes_no(const std::string &title, const std::string &message,Response dflt)
{
	cout<<title<<": "<<message<<' ';
	if(dflt==RESPONSE_NO)
		cout<<_("(no/yes)")<<endl;
	else
		cout<<_("(yes/no)")<<endl;
	string resp;
	cin>>resp;

	if(dflt==RESPONSE_NO)
	{
		if(resp=="yes")
			return RESPONSE_YES;
		else
			return RESPONSE_NO;
	}
	else
	{
		if(resp=="no")
			return RESPONSE_NO;
		else
			return RESPONSE_YES;
	}
}

UIInterface::Response
ConsoleUIInterface::yes_no_cancel(const string &title, const string &message,Response dflt)
{
	cout<<title<<": "<<message<<' ';
	if(dflt==RESPONSE_NO)
		cout<<_("(no/yes)")<<endl;
	else
		cout<<_("(yes/no)")<<endl;
	string resp;
	cin>>resp;

	if(dflt==RESPONSE_NO)
	{
		if(resp=="yes")
			return RESPONSE_YES;
		else
			return RESPONSE_NO;
	}
	else
	{
		if(resp=="no")
			return RESPONSE_NO;
		else
			return RESPONSE_YES;
	}
}

UIInterface::Response
ConsoleUIInterface::ok_cancel(const std::string &title, const std::string &message,Response dflt)
{
	cout<<title<<": "<<message<<' ';
	if(dflt==RESPONSE_CANCEL)
		cout<<_("(cancel/ok)")<<endl;
	else
		cout<<_("(ok/cancel)")<<endl;
	string resp;
	cin>>resp;

	if(dflt==RESPONSE_CANCEL)
	{
		if(resp=="ok")
			return RESPONSE_OK;
		else
			return RESPONSE_CANCEL;
	}
	else
	{
		if(resp=="cancel")
			return RESPONSE_CANCEL;
		else
			return RESPONSE_OK;
	}
}
	
bool
ConsoleUIInterface::task(const std::string &task)
{
	cout<<task<<endl;
	return true;
}

bool
ConsoleUIInterface::error(const std::string &task)
{
	cout<<_("error: ")<<task<<endl;
	return true;
}

bool
ConsoleUIInterface::warning(const std::string &task)
{
	cout<<_("warning: ")<<task<<endl;
	return true;
}

bool
ConsoleUIInterface::amount_complete(int current, int total)
{
	return true;
}



