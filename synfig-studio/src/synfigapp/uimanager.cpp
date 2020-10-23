/* === S Y N F I G ========================================================= */
/*!	\file uimanager.cpp
**	\brief Template File
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

#include "uimanager.h"
#include <iostream>
#include <string>

#include <synfig/general.h>

#include <synfigapp/localization.h>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === M E T H O D S ======================================================= */

UIInterface::Response
ConsoleUIInterface::confirmation(
			const std::string &message,
			const std::string &details,
			const std::string &confirm,
			const std::string &cancel,
			Response dflt
)
{
	cout << message.c_str() << endl;
	cout << details.c_str();

	if (dflt == RESPONSE_OK)
		cout << "(" << confirm.c_str() << "/" << cancel.c_str() << ")" << endl;
	else
		cout << "(" << cancel.c_str() << "/" << confirm.c_str() << ")" << endl;

	string resp;
	cin >> resp;

	if (dflt == RESPONSE_OK)
	{
		if (resp == cancel)
			return RESPONSE_CANCEL;
		return RESPONSE_OK;
	}
	if (resp == confirm)
		return RESPONSE_OK;
	return RESPONSE_CANCEL;
}



UIInterface::Response
ConsoleUIInterface::yes_no_cancel(
			const std::string &message,
			const std::string &details,
			const std::string &/*button1*/,
			const std::string &/*button2*/,
			const std::string &/*button3*/,
			Response dflt
)
{
	cout<<message.c_str()<<": "<<details.c_str()<<' ';
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


bool
ConsoleUIInterface::task(const std::string &task)
{
	cout<<task.c_str()<<endl;
	return true;
}


bool
ConsoleUIInterface::error(const std::string &task)
{
	cout<<_("error: ")<<task.c_str()<<endl;
	return true;
}


bool
ConsoleUIInterface::warning(const std::string &task)
{
	cout<<_("warning: ")<<task.c_str()<<endl;
	return true;
}


bool
ConsoleUIInterface::amount_complete(int /*current*/, int /*total*/)
{
	return true;
}

