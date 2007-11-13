/* === S Y N F I G ========================================================= */
/*!	\file gtkmm/main.cpp
**	\brief Synfig Studio Entrypoint
**
**	$Id$
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**	Copyright (c) 2007 Chris Moore
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

#include "app.h"
#include <iostream>
#include "ipc.h"
#include <stdexcept>

#include "general.h"

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

	setlocale(LC_ALL, "");
	bindtextdomain("synfigstudio", LOCALEDIR);
	textdomain("synfigstudio");

	{
		SmartFILE file(IPC::make_connection());
		if(file)
		{
			fprintf(file.get(),"F\n");

			// Hey, another copy of us is open!
			// don't bother opening us, just go ahead and
			// tell the other copy to load it all up
			if(argc>=1)
			{
				for(;argc>=1;(argc)--)
					if((argv)[argc] && (argv)[argc][0]!='-')
					{
						fprintf(file.get(),"O %s\n",etl::absolute_path((argv)[argc]).c_str());
					}
			}

			fprintf(file.get(),"F\n");

			return 0;
		}
	}

	try
	{
		studio::App app(&argc, &argv);

		app.run();
	}
	catch(synfig::SoftwareExpired)
	{
		cerr<<"FATAL: Software Expired"<<endl;
		return 39;
	}
	catch(int ret)
	{
		std::cerr<<"Application shutdown with errors ("<<ret<<')'<<std::endl;
		return ret;
	}
	catch(string str)
	{
		std::cerr<<"Uncaught Exception:string: "<<str<<std::endl;
		throw;
	}
	catch(std::exception x)
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
