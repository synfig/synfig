/* === S I N F G =========================================================== */
/*!	\file uimanager.h
**	\brief User Interface Manager Class
**
**	$Id: uimanager.h,v 1.1.1.1 2005/01/07 03:34:37 darco Exp $
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

#ifndef __SINFG_APP_UIMANAGER_H
#define __SINFG_APP_UIMANAGER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <sinfg/general.h>
#include <sinfg/string.h>
#include <sigc++/object.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace sinfgapp {

class UIInterface : public etl::shared_object, public sinfg::ProgressCallback, public sigc::trackable
{
public:
	enum Response
	{
		RESPONSE_CANCEL=-1,
		RESPONSE_NO=0,
		RESPONSE_YES=1,
		RESPONSE_OK=2
	};
	virtual ~UIInterface() { }
	virtual Response yes_no(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)=0;
	virtual Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_YES)=0;
	virtual Response ok_cancel(const std::string &title, const std::string &message,Response dflt=RESPONSE_OK)=0;
};	

class DefaultUIInterface : public UIInterface
{
public:
	Response yes_no(const std::string &title, const std::string &message,Response dflt)
		{ return dflt; }
	Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt)
		{ return dflt; }
	Response ok_cancel(const std::string &title, const std::string &message,Response dflt)
		{ return dflt; }
	
	bool task(const std::string &task)
		{ return true; }
	bool error(const std::string &task)
		{ return true; }
	bool warning(const std::string &task)
		{ return true; }
	bool amount_complete(int current, int total)
		{ return true; }
};	

class ConfidentUIInterface : public UIInterface
{
public:
	Response yes_no(const std::string &title, const std::string &message,Response dflt)
		{ return RESPONSE_YES; }
	Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt)
		{ return RESPONSE_YES; }
	Response ok_cancel(const std::string &title, const std::string &message,Response dflt)
		{ return RESPONSE_OK; }
	
	bool task(const std::string &task)
		{ return true; }
	bool error(const std::string &task)
		{ return true; }
	bool warning(const std::string &task)
		{ return true; }
	bool amount_complete(int current, int total)
		{ return true; }
};	

class ConsoleUIInterface : public UIInterface
{
public:
	Response yes_no(const std::string &title, const std::string &message,Response dflt);
	Response yes_no_cancel(const std::string &title, const std::string &message,Response dflt);
	Response ok_cancel(const std::string &title, const std::string &message,Response dflt);
	
	bool task(const std::string &task);
	bool error(const std::string &task);
	bool warning(const std::string &task);
	bool amount_complete(int current, int total);
};	

}; // END of namespace sinfgapp

/* === E N D =============================================================== */

#endif
