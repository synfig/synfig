/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/main.cpp
**	\brief Synfig Studio Entrypoint
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007, 2008 Chris Moore
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

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/general.h>

#include "app.h"
#include <iostream>
#include "ipc.h"
#include <stdexcept>

#include <gui/localization.h>
#include <glibmm/convert.h>

#ifdef _WIN32
#include "main_win32.h"
#endif

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace synfig;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

int main(int argc, char **argv)
{

#ifdef _WIN32
	if (consoleOptionEnabled(argc, argv))
	{
		redirectIOToConsole();
	}
	else
	{
		// QuickHack: to avoid strange bug with stderr
		freopen("NUL", "w", stdout);
		freopen("NUL", "w", stderr);
		freopen("NUL", "r", stdin);
		ios::sync_with_stdio();
	}
#endif

	String binary_path = synfig::get_binary_path(String(argv[0]));
	
#ifdef ENABLE_NLS
	String locale_dir;
	locale_dir = etl::dirname(etl::dirname(binary_path))+ETL_DIRECTORY_SEPARATOR+"share"+ETL_DIRECTORY_SEPARATOR+"locale";
	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE,  Glib::locale_from_utf8(locale_dir).c_str() );
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif
	{
		SmartFILE file(IPC::make_connection());
		if(file)
		{
			cout << endl;
			cout << "   " << _("synfig studio is already running") << endl << endl;
			cout << "   " << _("the existing process will be used") << endl << endl;

			// Hey, another copy of us is open!
			// don't bother opening us, just go ahead and
			// tell the other copy to load it all up
			if (argc>1)
				fprintf(file.get(),"F\n");

			while(--argc)
				if((argv)[argc] && (argv)[argc][0]!='-')
					fprintf(file.get(),"O %s\n",etl::absolute_path((argv)[argc]).c_str());

			fprintf(file.get(),"F\n");

			return 0;
		}
	}

	cout << endl;
	cout << "   " << _("synfig studio -- starting up application...") << endl << endl;

	try
	{
		studio::App app(etl::dirname(binary_path), &argc, &argv);

		app.run();
	}
	catch(int ret)
	{
		std::cerr<<"Application shutdown with errors ("<<ret<<')'<<std::endl;
		return ret;
	}
	catch(string& str)
	{
		std::cerr<<"Uncaught Exception:string: "<<str.c_str()<<std::endl;
		throw;
	}
	catch(std::exception& x)
	{
		std::cerr<<"Standard Exception: "<<x.what()<<std::endl;
		throw;
	}
	catch(Glib::Exception& x)
	{
		std::cerr<<"GLib Exception: "<<x.what()<<std::endl;
		throw;
	}
	catch(...)
	{
		std::cerr<<"Uncaught Exception"<<std::endl;
		throw;
	}

	std::cerr<<"Application appears to have terminated successfully"<<std::endl;

	return 0;
}
