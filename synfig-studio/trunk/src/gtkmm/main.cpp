/* === S I N F G =========================================================== */
/*!	\file main.cpp
**	\brief Sinfg Studio Entrypoint
**
**	$Id: main.cpp,v 1.2 2005/01/13 18:37:30 darco Exp $
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

#include "app.h"
#include <iostream>
#include "ipc.h"
#include <stdexcept>

#endif

/* === U S I N G =========================================================== */

using namespace std;
using namespace etl;
using namespace sinfg;
using namespace studio;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

/* === E N T R Y P O I N T ================================================= */

int main(int argc, char **argv)
{
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
	catch(sinfg::SoftwareExpired)
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

	std::cerr<<"Application appears to have terminated successfuly"<<std::endl;
	
	return 0;
}
